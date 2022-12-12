#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"
#include "Logs.h"


HT_info* open_ht_files[BF_MAX_OPEN_FILES];
int      open_ht_files_counter = 0;

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int HT_CreateFile(char *fileName,  int buckets){
  log_info("Creating file : %s",fileName);
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
  file_info->type = 2;
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));
  CALL_OR_DIE(BF_CloseFile(fd));

  return 0;
}

HT_info* HT_OpenFile(char *fileName){

  if(open_ht_files_counter > 100){
    log_info("Failed to open file %s : Maximum amount of open BF files has been reached.",fileName);
    return NULL;
  }

  int fd;
  CALL_OR_DIE(BF_OpenFile(fileName, &fd));
  log_info("Opened file %s",fileName);

  BF_Block* block;
  BF_Block_Init(&block);
  int block_num;
  BF_GetBlockCounter(fd,&block_num);
  BF_GetBlock(fd,block_num-1,block);

  void* data;
  data = BF_Block_GetData(block);
  HT_info* file_info = data;
  file_info->fd = fd;

  //If file is not ht return NULL
  if(file_info->type != 2){
    free(file_info);
    CALL_OR_DIE(BF_CloseFile(fd));
    return NULL;
  }
  CALL_OR_DIE(BF_UnpinBlock(block));
  log_info("File info %s : {fd = %d | blocknum = %d | buckets = %d}",fileName,file_info->fd,file_info->blocks_num,file_info->buckets);

  open_ht_files[open_ht_files_counter++] = file_info;

  return file_info;
}

int HT_CloseFile( HT_info* HT_info ){
  log_info("Closing file with fd %d",HT_info->fd);
  for(int i=0;i<open_ht_files_counter;i++){
    if( open_ht_files[i]->fd == HT_info->fd )
      BF_CloseFile(HT_info->fd);
      open_ht_files_counter--;
  }
  return 0;
}

int HT_InsertEntry(HT_info* ht_info, Record record){
  log_info("Inserting record : { %d, %s, %s, %s}", record.id, record.name, record.surname, record.city);
  return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
  log_info("Fetching all entries");
  return 0;
}
