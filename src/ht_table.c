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
int ht_created = 0;


#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


char* get_name_of_next_db(){
  // Each database will be named using DB_ROOT as a base
  // and ht_created as their id and will be of .db type.

  ht_created++;
  log_info("Creating name for id : %d",ht_created);
  char* id = malloc(ht_created/10);
  sprintf(id,"%d",ht_created);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  log_info("Name created succesfuly");
  return f_name;
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
  file_info->blocks  = 0;
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));
  CALL_OR_DIE(BF_CloseFile(fd));

  return 0;
}

HT_info* HT_OpenFile(char *fileName){
  log_info("Opening file %s",fileName);
  return NULL;
}

int HT_CloseFile( HT_info* HT_info ){
  log_info("Closing file");
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
