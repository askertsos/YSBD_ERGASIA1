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

  log_info("Created file %s",fileName);

  int fd;
  BF_Block* block;
  void* data;


  CALL_BF(BF_CreateFile(fileName));
  BF_Block_Init(&block);
  CALL_BF(BF_OpenFile(fileName,&fd));



  printf("allocateblock\n");
  CALL_BF(BF_AllocateBlock(fd, block));
  printf("blockGetData\n");
  data = BF_Block_GetData(block);
  log_info("data = ");

  HP_info* info = data;
  info->fileDesc = -1;
  info->blocks = 1;

  log_info("copied hp_info");

  BF_Block_SetDirty(block);
  log_info("setdirty");
  CALL_BF(BF_UnpinBlock(block));
  log_info("unpin");

  CALL_BF(BF_CloseFile(fd));
  log_info("bf_closefile");
  CALL_BF(BF_Close());
  log_info("bf_close");

  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  log_info("in hp_openfile");
  int fd;
  BF_Block* block;
  CALL_BF(BF_OpenFile(fileName,&fd));
  BF_GetBlock(fd,0,block);

  HP_info* info = (HP_info*) BF_Block_GetData(block);
  return info;

  return NULL ;
}


int HP_CloseFile( HP_info* hp_info ){
  return 0;
}

int HP_InsertEntry(HP_info* hp_info, Record record){
  return 0;
}

int HP_GetAllEntries(HP_info* hp_info, int value){
  return 0;
}

