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
  CALL_OR_DIE(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  HT_info* file_info = data;
  file_info->buckets = buckets;
  file_info->blocks_num  = 1;
  file_info->fd = -1;
  file_info->type = 1;
  log_info("Created %s with info : {fd = %d | blocknum = %d | buckets = %d | type = %d}",fileName,file_info->fd,file_info->blocks_num,file_info->buckets,file_info->type);
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));
  CALL_OR_DIE(BF_CloseFile(fd));

  return 0;
}

HT_info* HT_OpenFile(char *fileName){

  int fd;
  CALL_OR_DIE(BF_OpenFile(fileName, &fd));

  BF_Block* block;
  BF_Block_Init(&block);
  int block_num;
  BF_GetBlockCounter(fd,&block_num);
  BF_GetBlock(fd,block_num-1,block);

  void* data;
  data = BF_Block_GetData(block);
  HT_info* temp = data;

  //If file is not ht return NULL
  if(temp->type != 1){
    CALL_OR_DIE(BF_CloseFile(fd));
    return NULL;
  }
  HT_info* file_info = malloc(sizeof(HT_info));
  file_info->fd = fd;
  file_info->type = temp->type;
  file_info->blocks_num = temp->blocks_num;
  file_info->buckets = temp->buckets;

  CALL_OR_DIE(BF_UnpinBlock(block));

  for(int i = 0; i < BF_MAX_OPEN_FILES; i++){
    if (open_ht_files[i] == NULL){
      open_ht_files[i] = file_info;
      open_ht_files[i]->position_in_open_files = i;
      open_ht_files_counter++;
      log_info("Opened file %s with info : {fd = %d | blocknum = %d | buckets = %d | type = %d | position = %d}",fileName,file_info->fd,file_info->blocks_num,file_info->buckets,file_info->type, file_info->position_in_open_files);
      return file_info;
    }
  }

  return NULL;
}

int HT_CloseFile( HT_info* HT_info ){
  log_info("Closing file with info: {fd = %d | blocknum = %d | buckets = %d | type = %d | position = %d}",HT_info->fd,HT_info->blocks_num,HT_info->buckets,HT_info->type,HT_info->position_in_open_files);
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
  log_info("Inserting record : { %d, %s, %s, %s}", record.id, record.name, record.surname, record.city);
  return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
  log_info("Fetching all entries");
  return 0;
}
