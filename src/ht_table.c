#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

// Base name to create all ht_databases from
#define DB_ROOT "ht_databases/ht_"
// Store number of created databases
int open_ht_files_counter = 0;


#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


int HT_CreateFile(char *fileName,  int buckets){
  BF_ErrorCode err = BF_CreateFile(fileName); if (err != BF_OK){BF_PrintError(err); return -1;}

  int fd;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

  err = BF_OpenFile(fileName, &fd); if (err != BF_OK){BF_PrintError(err); return -1;}

  // Allocate first block that contains metadata of file
  err = BF_AllocateBlock(fd, block); if (err != BF_OK){BF_PrintError(err); return -1;}
  data = BF_Block_GetData(block);
  HT_info* file_info = data;
  file_info->buckets = buckets;
  file_info->fd = -1;
  file_info->type = 1;
  file_info->next_empty_bucket = buckets + 2;
  BF_Block_SetDirty(block);
  err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return -1;}

  // Allocate initial blocks for buckets
  for(int i=1; i <= buckets; i++){

    err = BF_AllocateBlock(fd, block); if (err != BF_OK){BF_PrintError(err); return -1;}
    data = BF_Block_GetData(block);
    HT_block_info* block_info = data;
    block_info->records_num = 0;
    block_info->bucket_id = i;
    block_info->next_bucket = -1;
    BF_Block_SetDirty(block);
    err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return -1;}

  }

  // Free info block before closing file

  BF_Block_Destroy(&block);
  err = BF_CloseFile(fd); if (err != BF_OK){BF_PrintError(err); return -1;}

  return 0;
}

HT_info* HT_OpenFile(char *fileName){

  int fd;
  BF_ErrorCode err;
  err = BF_OpenFile(fileName, &fd); if (err != BF_OK){BF_PrintError(err); return NULL;}

  BF_Block* block;
  BF_Block_Init(&block);
  BF_GetBlock(fd,0,block);

  void* data;
  data = BF_Block_GetData(block);
  HT_info* temp = data;

  //If file is not ht return NULL
  if(temp->type != 1){
    err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return NULL;}
    BF_Block_Destroy(&block);
    err = BF_CloseFile(fd); if (err != BF_OK){BF_PrintError(err); return NULL;}
    return NULL;
  }
  HT_info* file_info = malloc(sizeof(HT_info) );
  file_info->fd = fd;
  file_info->type = temp->type;
  file_info->buckets = temp->buckets;
  file_info->next_empty_bucket = temp->next_empty_bucket;


  if (open_ht_files_counter < BF_MAX_OPEN_FILES){
      open_ht_files_counter++;
      BF_Block_Destroy(&block);
      return file_info;
  }

  BF_Block_Destroy(&block);
  free(file_info);
  return NULL;
}

int HT_CloseFile( HT_info* HT_info ){
  BF_Block* block;
  BF_Block_Init(&block);
  BF_ErrorCode err = BF_GetBlock(HT_info->fd,0,block); if (err != BF_OK) {BF_PrintError(err); return -1;}
  err = BF_UnpinBlock(block); if (err != BF_OK) {BF_PrintError(err); return -1;}
  BF_Block_Destroy(&block);
  err = BF_CloseFile(HT_info->fd); if (err != BF_OK) {BF_PrintError(err); return -1;}
  if( err == BF_OK ){
    open_ht_files_counter--;
    return 0;
  }
  BF_PrintError(err);
  return -1;
}

int HT_InsertEntry(HT_info* ht_info, Record record){

  int bucket_num = ht_info->buckets;
  int hash_id = record.id % (bucket_num) + 1;
  int fd = ht_info->fd;
  int bucket_inserted = -1;
  BF_ErrorCode err;

  BF_Block* block;
  BF_Block_Init(&block);

  //Find the first empty block used by the hash_id we found to insert the new record
  BF_GetBlock(fd,hash_id,block);
  void* data = BF_Block_GetData(block);
  HT_block_info* block_info = data;
  while(block_info->next_bucket != -1){
    int next_bucket = block_info->next_bucket;
    err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return -1;}
    BF_GetBlock(fd,next_bucket - 1,block);
    data = BF_Block_GetData(block);
    block_info = data;
  }

  void* starting_bucket_position = data + block_info->records_num * sizeof(Record) + sizeof(HT_block_info);
  memcpy(starting_bucket_position,&record,sizeof(Record)) ;
  block_info->records_num += 1;
  bucket_inserted = block_info->bucket_id;
  BF_Block_SetDirty(block);
  err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return -1;}
  BF_Block_Destroy(&block);

  //After insertion check fs bucket has reached the maximum amount of records. Is it has, allocate a new bucket to be used at next insertion of same hash id
  if(block_info->records_num == RECORDS_PER_BLOCK){
    int new_bucket = ht_info->next_empty_bucket - 1; //Get the first available empty bucket and allocate it
    block_info->next_bucket = new_bucket; //Update previous bucket to point to new bucket
    ht_info->next_empty_bucket += 1; //Update next_empty_bucket variable for next allocation
    BF_Block* new_block;
    BF_Block_Init(&new_block);
    err = BF_AllocateBlock(fd, new_block); if (err != BF_OK){BF_PrintError(err); return -1;}
    void* new_data = BF_Block_GetData(new_block);
    HT_block_info* new_block_info = new_data;
    new_block_info->records_num = 0;
    new_block_info->bucket_id = new_bucket;
    new_block_info->next_bucket = -1;
    BF_Block_SetDirty(new_block);
    err = BF_UnpinBlock(new_block); if (err != BF_OK){BF_PrintError(err); return -1;}
    BF_Block_Destroy(&new_block);
  }

  return bucket_inserted;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){

  int* pid = value;
  int id = *pid;
  int bucket_num = ht_info->buckets;
  int hash_id = id % (bucket_num) + 1;
  int fd = ht_info->fd;
  BF_ErrorCode err;

  Record* record;
  int buckets_read = 1;
  BF_Block* block;
  BF_Block_Init(&block);

  //Get all buckets used by the hash id calculated
  BF_GetBlock(fd,hash_id,block);
  void* data = BF_Block_GetData(block);
  HT_block_info* block_info = data;

  int current_bucket = hash_id;
  int found = 0;
  while(current_bucket != -1){
    int record_found = 0;
    void* bucket_position = data + sizeof(HT_block_info);
    for(int i = 1; i <= block_info->records_num; i++){
      record = bucket_position;
      if(record->id == id){
        found = 1;
        printRecord(*record);
      }
      else if(found == 1) return buckets_read;
      bucket_position += sizeof(Record);
    }

    current_bucket = block_info->next_bucket;
    if(current_bucket == -1) break;
    buckets_read++;
    err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return -1;}
    BF_GetBlock(fd,current_bucket - 1,block);
    data = BF_Block_GetData(block);
    block_info = data;
  }
  err = BF_UnpinBlock(block); if (err != BF_OK){BF_PrintError(err); return -1;}
  BF_Block_Destroy(&block);

  if(found == 0) return -1;
  return buckets_read;
}
