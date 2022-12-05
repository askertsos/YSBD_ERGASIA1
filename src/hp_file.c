#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "Logs.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

char* get_full_path(char* file_name){
  char* full_path = malloc(strlen(DB_ROOT) + strlen(file_name) + 1);
  strcpy(full_path,DB_ROOT);
  strcat(full_path,file_name);
  return full_path;
}

int HP_CreateFile(char *fileName){
  BF_CreateFile(get_full_path(fileName));
  log_info("Created file %s",get_full_path(fileName));
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
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

