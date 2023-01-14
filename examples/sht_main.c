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
#define HT_ROOT  "ht_databases/ht_"
#define SHT_ROOT "sht_databases/sht_"

// Store number of created databases
int sht_created = 0;
int ht_created = 0;
char* sht_names_created[MAX_CREATED_FILES];
char*  ht_names_created[MAX_CREATED_FILES];

SHT_info* sht_created_info[MAX_CREATED_FILES];

char* generate_ht_file(){
  // Each ht database will be named using HT_ROOT as a base
  // and ht_created as their id and will be of .db type.

  ht_created++;
  char* id = malloc(100);
  sprintf(id,"%d",ht_created);
  char* f_name = malloc(strlen(HT_ROOT) + 100);
  strcpy(f_name,HT_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  ht_names_created[ht_created - 1] = f_name;
  return f_name;
}

char* generate_sht_file(){
  // Each sht database will be named using SHT_ROOT as a base
  // and sht_created as their id and will be of .db type.

  sht_created++;
  char* id = malloc(100);
  sprintf(id,"%d",sht_created);
  char* f_name = malloc(strlen(SHT_ROOT) + 100);
  strcpy(f_name,SHT_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  sht_names_created[sht_created - 1] = f_name;
  return f_name;
}

int main() {

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);
  log_info("Entered ht_main");

  BF_Init(LRU);

  SHT_CreateSecondaryIndex(generate_sht_file(),10,generate_ht_file());
  SHT_CreateSecondaryIndex(generate_sht_file(),10,generate_ht_file());
  SHT_CreateSecondaryIndex(generate_sht_file(),10,generate_ht_file());

  for(int i = 0; i < sht_created; i++) free(sht_names_created[i]);
  for(int i = 0; i < ht_created; i++)  free(ht_names_created[i]);

  BF_Close();
}
