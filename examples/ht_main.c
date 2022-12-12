#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "Logs.h"

#define RECORDS_NUM 200 // you can change it if you want
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

// Base name to create all ht_databases from
#define DB_ROOT "ht_databases/ht_"
// Store number of created databases
int ht_created = 0;
char* created_files[200];
HT_info* created_info[200];

char* generate_name(){
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

void create_file(){
  char* fname = generate_name();
  created_files[ht_created-1] = fname;
  HT_CreateFile(fname,10);
}


int main() {

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);

  log_info("Entered ht_main");

  BF_Init(LRU);

  create_file();
  create_file();
  create_file();

  for(int i=0;i<ht_created;i++) created_info[i] = HT_OpenFile(created_files[i]);
  for(int i=0;i<ht_created;i++) HT_CloseFile(created_info[i]);

  // Record record;
  // srand(12569874);
  // int r;
  // printf("Insert Entries\n");
  // for (int id = 0; id < RECORDS_NUM; ++id) {
  //   record = randomRecord();
  //   HT_InsertEntry(info, record);
  // }

  // printf("RUN PrintAllEntries\n");
  // int id = rand() % RECORDS_NUM;
  // HT_GetAllEntries(info, &id);

  for(int i=0;i<200;i++) free(created_files[i]);
  BF_Close();
}
