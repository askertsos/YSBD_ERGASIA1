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
  return f_name;
}

int HT_CreateFile(char *fileName,  int buckets){
  BF_CreateFile(fileName);
  log_info("Created file %s",fileName);
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
