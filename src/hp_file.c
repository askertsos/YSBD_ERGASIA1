#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "Logs.h"

// Base name to create all hp_databases from
#define DB_ROOT "hp_databases/hp_"
// Store number of created databases
int hp_created = 0;


#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}

char* get_name_of_next_db(){
  // Each database will be named using DB_ROOT as a base
  // and hp_created as their id and will be of .db type.

  hp_created++;
  log_info("Creating name for id : %d",hp_created);
  // char* id = malloc(hp_created/10);
  char id[100];
  sprintf(id,"%d",hp_created);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  // free(id);
  return f_name;
}

int HP_CreateFile(char *fileName){
  log_info("Creating file : %s",fileName);
  CALL_OR_DIE(BF_CreateFile(fileName));

  int fd;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

  CALL_OR_DIE(BF_OpenFile(fileName, &fd));
  CALL_OR_DIE(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  HP_info* file_info = data;
  file_info->blocks_num  = 1;
  file_info->fd = -1;
  file_info->type = 1;
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));
  CALL_OR_DIE(BF_CloseFile(fd));

  return 0;
}

HP_info* HP_OpenFile(char *fileName){

  if(open_hp_files_counter > 100){
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
  hp_info* file_info = data;
  file_info->fd = fd;

  //If file is not hp return NULL
  if(file_info->type != 0){
    free(file_info);
    CALL_OR_DIE(BF_CloseFile(fd));
    return NULL;
  }
  CALL_OR_DIE(BF_UnpinBlock(block));
  log_info("File info %s : {fd = %d | blocknum = %d}",fileName,file_info->fd,file_info->blocks_num);

  open_hp_files[open_hp_files_counter++] = file_info;

  return file_info;
}


int HP_CloseFile( HP_info* HP_info ){
  log_info("Closing file with fd %d",hp_info->fd);
  for(int i=0;i<open_hp_files_counter;i++){
    if( open_hp_files[i]->fd == hp_info->fd )
      BF_CloseFile(hp_info->fd);
      open_hp_files_counter--;
  }
  return 0;
}

int HP_InsertEntry(HP_info* hp_info, Record record){
  return 0;
}

int HP_GetAllEntries(HP_info* hp_info, int value){
  return 0;
}

