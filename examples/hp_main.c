#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_table.h"
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
hp_info* created_info[200];

char* generate_name(){
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
  log_info("Name created succesfuly");
  return f_name;
}

void create_file(){
  char* fname = generate_name();
  created_files[hp_created-1] = fname;
  HP_CreateFile(fname);
}


int main() {

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);

  log_info("Entered hp_main");

  BF_Init(LRU);

<<<<<<< HEAD
  create_file();
  create_file();
  create_file();

  for(int i=0;i<hp_created;i++) created_info[i] = HP_OpenFile(created_files[i]);
  for(int i=0;i<hp_created;i++) HP_CloseFile(created_info[i]);

  // Record record;
  // srand(12569874);
  // int r;
  // printf("Insert Entries\n");
  // for (int id = 0; id < RECORDS_NUM; ++id) {
  //   record = randomRecord();
  //   hp_InsertEntry(info, record);
  // }

  // printf("RUN PrintAllEntries\n");
  // int id = rand() % RECORDS_NUM;
  // hp_GetAllEntries(info, &id);
=======
  char* file1 = get_name_of_next_db();
  log_info("file1 is %s",file1);
  HP_CreateFile(file1);
  free(file1);
  file1 = get_name_of_next_db();
  log_info("file1 is %s",file1);
  HP_CreateFile(file1);
  HP_info* info = HP_OpenFile(file1);
  free(file1);


  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HP_InsertEntry(info, record);
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n",id);
  HP_GetAllEntries(info, id);
>>>>>>> 986472730e591e1dddc9c120cc480ff548057f26

  for(int i=0;i<200;i++) free(created_files[i]);
  fclose(logger);
  BF_Close();
  fclose(logger);
}
