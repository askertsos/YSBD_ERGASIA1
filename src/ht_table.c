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
HT_info* open_ht_files[100];
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
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));

  // Allocate initial blocks for buckets
  for(int i=1; i <= buckets; i++){
    CALL_OR_DIE(BF_AllocateBlock(fd, block));
    data = BF_Block_GetData(block);
    memcpy(data,"-",BF_BLOCK_SIZE);
    HT_block_info* block_info = data;
    block_info->records_num = 0;
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

  CALL_OR_DIE(BF_UnpinBlock(block));

  for(int i = 0; i < BF_MAX_OPEN_FILES; i++){
    if (open_ht_files[i] == NULL){
      open_ht_files[i] = file_info;
      open_ht_files[i]->position_in_open_files = i;
      open_ht_files_counter++;
      BF_Block_Destroy(&block);
      log_info("Opened file %s with info : {fd = %d | buckets = %d | type = %d | position = %d}",fileName,file_info->fd,file_info->buckets,file_info->type, file_info->position_in_open_files);
      return file_info;
    }
  }

  BF_Block_Destroy(&block);
  return NULL;
}

int HT_CloseFile( HT_info* HT_info ){
  log_info("Closing file with info: {fd = %d | buckets = %d | type = %d | position = %d}",HT_info->fd,HT_info->buckets,HT_info->type,HT_info->position_in_open_files);
  for(int i=0;i<BF_MAX_OPEN_FILES;i++){
    if( open_ht_files[i] != NULL && open_ht_files[i]->fd == HT_info->fd ){
      BF_CloseFile(HT_info->fd);
      free(open_ht_files[i]);
      open_ht_files[i] = NULL;
      open_ht_files_counter--;
      return 0;
    }
  }
  return -1;
}

int HT_InsertEntry(HT_info* ht_info, Record record){

  int bucket_num = ht_info->buckets;
  int hash_id = record.id % (bucket_num + 1);
  int fd = ht_info->fd;

  BF_Block* block;
  BF_Block_Init(&block);

  BF_GetBlock(fd,hash_id,block);
  void* data = BF_Block_GetData(block);

  HT_block_info block_info;
  memcpy(&block_info,data,sizeof(HT_block_info));

  if(block_info.records_num < max_records_per_block){

    void* starting_bucket_position = data + block_info.records_num * sizeof(Record) + sizeof(HT_block_info);
    memcpy(starting_bucket_position,&record,sizeof(Record));
    block_info.records_num += 1;
    memcpy(data,&block_info,sizeof(HT_block_info));
    log_info("Inserted record : { %d, %s, %s, %s} at bucket %d", record.id, record.name, record.surname, record.city,hash_id,block_info.records_num);

  }
  else{

    

  }
  BF_Block_SetDirty(block);
  // CALL_OR_DIE(BF_UnpinBlock(block));
  // BF_Block_Destroy(&block);

  return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
  printf("Fetching all entries for file %d\n\n",ht_info->fd);
  log_info("Fetching all entries for file %d",ht_info->fd);

  BF_Block* block;
  BF_Block_Init(&block);
  HT_block_info block_info;
  int bucket_num = ht_info->buckets;
  int fd = ht_info->fd;

  for(int bucket = 1; bucket <= bucket_num; bucket++){

    BF_GetBlock(fd,bucket,block);
    void* data = BF_Block_GetData(block);
    memcpy(&block_info,data,sizeof(HT_block_info));

    printf("Printing all entries of bucket %d\n",bucket);
    log_info("Printing all entries of bucket %d",bucket);
    printf("Bucket info : {number of records = %d}\n",block_info.records_num);
    log_info("Bucket info : {number of records = %d}",block_info.records_num);

    Record* record;
    for(int entry = 0; entry < block_info.records_num; entry++){
      record = data + entry * sizeof(Record) + sizeof(HT_block_info);
      printRecord(*record);
      log_info("{ %d, %s, %s, %s}", record->id, record->name, record->surname, record->city);
    }

    CALL_OR_DIE(BF_UnpinBlock(block));

  }

  BF_Block_Destroy(&block);

  return 0;
}
