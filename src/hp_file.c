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
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}

char* get_name_of_next_db(){
  // Each database will be named using DB_ROOT as a base
  // and hp_created as their id and will be of .db type.

  hp_created++;
  log_info("Creating name for id : %d",hp_created);
  char* id = malloc(hp_created/10);
  sprintf(id,"%d",hp_created);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  return f_name;
}

int HP_CreateFile(char *fileName){
  int fd = 5;
  BF_Block* block;
  void* data;

  printf("createfile %s\n",fileName);
  CALL_BF(BF_CreateFile(fileName));
  printf("openfile\n");
  CALL_BF(BF_OpenFile(fileName,&fd));
  BF_Block_Init(&block);

  printf("allocateblock\n");
  CALL_BF(BF_AllocateBlock(fd, block));
  printf("blocGetData\n");
  data = BF_Block_GetData(block);
  log_info("data = ");

  HP_info info;
  info.fileDesc = fd;
  info.blocks = 1;

  memcpy(data,&info,sizeof(info));
  log_info("memcpy");

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

