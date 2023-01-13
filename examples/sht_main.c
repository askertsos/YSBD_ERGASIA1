#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"
#include "Logs.h"

#define RECORDS_NUM 200 // you can change it if you want
#define MAX_CREATED_FILES 200
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

// Base name to create all ht_databases from
#define DB_HT_ROOT "ht_databases/ht_"
#define DB_SHT_ROOT "sht_databases/sht_"
// Store number of created databases
int sht_created = 0;
char* sht_created_files[MAX_CREATED_FILES];
SHT_info* sht_created_info[MAX_CREATED_FILES];

char* generate_name(){
  // Each database will be named using DB_ROOT as a base
  // and ht_created as their id and will be of .db type.

  ht_created++;
  char* id = malloc(ht_created/10);
  sprintf(id,"%d",ht_created);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  return f_name;
}

void create_file(){
  char* fname = generate_name();
  ht_created_files[ht_created-1] = fname;
  SHT_CreateSecondaryIndex(fname,10);
}


int main() {

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);

  log_info("Entered sht_main");

  BF_Init(LRU);

  create_file();
  create_file();
  create_file();
  return 0;

  for(int i=0;i<ht_created;i++) ht_created_info[i] = HT_OpenFile(ht_created_files[i]);
  HT_CloseFile(ht_created_info[1]);
  create_file();
  ht_created_info[3] = HT_OpenFile(ht_created_files[3]);
  ht_created_info[1] = HT_OpenFile(ht_created_files[1]);

  Record record;
  srand(time(NULL));
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    int times_to_insert = rand() % 10;
    for(int j = 0; j < times_to_insert; j++)  HT_InsertEntry(ht_created_info[0], record);
  }

  for(int i = 0; i < rand() % 100; i++){
    int id = rand() % RECORDS_NUM;
    int buckets_read = HT_GetAllEntries(ht_created_info[0], &id);
    printf("Buckets read : %d\n",buckets_read);
    log_info("Buckets read : %d\n",buckets_read);
  }

  for(int i=0;i<MAX_CREATED_FILES;i++) free(ht_created_files[i]);
  fclose(logger);
  BF_Close();
}
