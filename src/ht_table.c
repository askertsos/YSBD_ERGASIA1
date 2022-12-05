#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"
#include "Logs.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


char* get_full_path(char* file_name){
  char* full_path = malloc(strlen(DB_ROOT) + strlen(file_name) + 1);
  strcpy(full_path,DB_ROOT);
  strcat(full_path,file_name);
  return full_path;
}

int HT_CreateFile(char *fileName,  int buckets){
  BF_CreateFile(get_full_path(fileName));
  log_info("Created file %s",get_full_path(fileName));
  return 0;
}

HT_info* HT_OpenFile(char *fileName){
  return NULL;
}

int HT_CloseFile( HT_info* HT_info ){
  return 0;
}

int HT_InsertEntry(HT_info* ht_info, Record record){
  return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
  return 0;
}
