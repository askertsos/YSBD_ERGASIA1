#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"
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

// Base name to create all hp_databases from
#define DB_ROOT "hp_databases/hp_"
// Store number of created databases
int hp_created = 0;
char* created_files[200];
HP_info* created_info[200];

char* filenameGenerator(){
  // Each database will be named using DB_ROOT as a base
  // and hp_created as their id and will be of .db type.

  hp_created++;
  log_info("Creating name for id : %d",hp_created);
  char* id = malloc(100);
  sprintf(id,"%d",hp_created);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  log_info("Name created succesfuly");
  free(id);
  return f_name;
}


int main() {

  int blocksInMemory = 0;
  int filesInMemory = 0;

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);

  log_info("Entered hp_main");

  BF_Init(LRU);

  char* filename = filenameGenerator();
  log_info("filename is %s",filename);
  HP_CreateFile(filename);
  printf("created %s\n",filename);
  free(filename);
  filename = filenameGenerator();
  log_info("filename is %s",filename);
  HP_CreateFile(filename);
  printf("created %s\n",filename);
  HP_info* info = HP_OpenFile(filename);
  free(filename);


  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HP_InsertEntry(info, record);
  }

  printf("RUN PrintAllEntries\n");
  for (int i=0; i<100; i++){
    int id = rand() % RECORDS_NUM;
    printf("\nSearching for: %d\n",id);
    HP_GetAllEntries(info, id);
  }

  // for(int i=0;i<200;i++) free(created_files[i]);

  BF_CloseFile(info->fileDesc);
  free(info);
  BF_Close();
  fclose(logger);
}
