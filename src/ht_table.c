#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"
#include "Logs.h"

// Base name to create all ht_databases from
#define DB_ROOT "ht_databases/ht_"
// Store number of created databases
int open_ht_files_counter = 0;
int max_records_per_block = BF_BLOCK_SIZE / sizeof(Record);


#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


int HT_CreateFile(char *fileName,  int buckets){
  CALL_OR_DIE(BF_CreateFile(fileName));

  int fd;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

  CALL_OR_DIE(BF_OpenFile(fileName, &fd));

  // Allocate first block that contains metadata of file
  CALL_OR_DIE(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  HT_info* file_info = data;
  file_info->buckets = buckets;
  file_info->fd = -1;
  file_info->type = 1;
  file_info->next_empty_bucket = buckets + 2;
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));

  // Allocate initial blocks for buckets
  for(int i=1; i <= buckets; i++){

    CALL_OR_DIE(BF_AllocateBlock(fd, block));
    data = BF_Block_GetData(block);
    HT_block_info* block_info = data;
    block_info->records_num = 0;
    block_info->bucket_id = i;
    block_info->next_bucket = -1;
    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));

  }

  // Free info block before closing file

  log_info("Created %s with info : {fd = %d | buckets = %d | type = %d}",fileName,file_info->fd,file_info->buckets,file_info->type);
  BF_Block_Destroy(&block);
  CALL_OR_DIE(BF_CloseFile(fd));

  return 0;
}

HT_info* HT_OpenFile(char *fileName){

  int fd;
  CALL_OR_DIE(BF_OpenFile(fileName, &fd));

  BF_Block* block;
  BF_Block_Init(&block);
  BF_GetBlock(fd,0,block);

  void* data;
  data = BF_Block_GetData(block);
  HT_info* temp = data;

  //If file is not ht return NULL
  if(temp->type != 1){
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_OR_DIE(BF_CloseFile(fd));
    return NULL;
  }
  HT_info* file_info = malloc(sizeof(HT_info));
  file_info->fd = fd;
  file_info->type = temp->type;
  file_info->buckets = temp->buckets;
  file_info->next_empty_bucket = temp->next_empty_bucket;

  CALL_OR_DIE(BF_UnpinBlock(block));

  if (open_ht_files_counter < BF_MAX_OPEN_FILES){
      open_ht_files_counter++;
      BF_Block_Destroy(&block);
      log_info("Opened file %s with info : {fd = %d | buckets = %d | type = %d | position = %d}",fileName,file_info->fd,file_info->buckets,file_info->type, file_info->position_in_open_files);
      return file_info;
  }

  BF_Block_Destroy(&block);
  return NULL;
}

int HT_CloseFile( HT_info* HT_info ){
  log_info("Closing file with info: {fd = %d | buckets = %d | type = %d | position = %d}",HT_info->fd,HT_info->buckets,HT_info->type,HT_info->position_in_open_files);
  if( BF_CloseFile(HT_info->fd) == 0 ){
    open_ht_files_counter--;
    return 0;
  }
  return -1;
}

int HT_InsertEntry(HT_info* ht_info, Record record){

  int bucket_num = ht_info->buckets;
  int hash_id = record.id % (bucket_num) + 1;
  int fd = ht_info->fd;
  int bucket_inserted = -1;

  BF_Block* block;
  BF_Block_Init(&block);

  //Find the first empty block used by the hash_id we found to insert the new record
  BF_GetBlock(fd,hash_id,block);
  void* data = BF_Block_GetData(block);
  HT_block_info* block_info = data;
  while(block_info->next_bucket != -1){
    int next_bucket = block_info->next_bucket;
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_GetBlock(fd,next_bucket - 1,block);
    data = BF_Block_GetData(block);
    block_info = data;
  }

  void* starting_bucket_position = data + block_info->records_num * sizeof(Record) + sizeof(HT_block_info);
  memcpy(starting_bucket_position,&record,sizeof(Record));
  block_info->records_num += 1;
  bucket_inserted = block_info->bucket_id;
  log_info("Inserted record : { %d, %s, %s, %s} at bucket %d", record.id, record.name, record.surname, record.city,block_info->bucket_id);
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  //After insertion check fs bucket has reached the maximum amount of records. Is it has, allocate a new bucket to be used at next insertion of same hash id
  if(block_info->records_num == max_records_per_block){
    int new_bucket = ht_info->next_empty_bucket; //Get the first available empty bucket and allocate it
    block_info->next_bucket = new_bucket; //Update previous bucket to point to new bucket
    ht_info->next_empty_bucket += 1; //Update next_empty_bucket variable for next allocation
    BF_Block* new_block;
    BF_Block_Init(&new_block);
    CALL_OR_DIE(BF_AllocateBlock(fd, new_block));
    void* new_data = BF_Block_GetData(new_block);
    HT_block_info* new_block_info = new_data;
    new_block_info->records_num = 0;
    new_block_info->bucket_id = new_bucket;
    new_block_info->next_bucket = -1;
    BF_Block_SetDirty(new_block);
    CALL_OR_DIE(BF_UnpinBlock(new_block));
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

  printf("Fetching all entries with id = %d for file %d\n\n",id,ht_info->fd);
  log_info("Fetching all entries with id = %d for file %d",id,ht_info->fd);

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
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_GetBlock(fd,current_bucket - 1,block);
    data = BF_Block_GetData(block);
    block_info = data;
  }

  BF_Block_Destroy(&block);

  return -1;
}
