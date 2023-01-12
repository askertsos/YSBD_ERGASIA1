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


  /*Για RECORDS_NUM επαναλήψεις, επιλέγεται ένα τυχαίο record, και επιλέγεται τυχαία ο αριθμός των φορών που θα εισαχθεί στο αρχείο.
  Γίνεται για να δειχθεί πως δουλεύει σωστά η GetAllEntries, ότι δηλαδή εκτυπώνει όλα τα records με το ζητούμενο id*/
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    int times = rand () % 3 + 1;
    for (int i=0; i<times; i++)
      HP_InsertEntry(info, record);
  }

  printf("RUN PrintAllEntries\n");
  /*Εκτυπώνει όλα τα records που έχουν εισαχθεί στο αρχείο*/
  for (int id = 0; id < RECORDS_NUM; ++id){
    printf("\nSearching for: %d\n",id);
    int blocks = HP_GetAllEntries(info, id);

    if (blocks > 0) 
      printf("Read %d blocks\n",blocks);
    else 
      printf("id #%d not found in file\n",id);
  }

  // for(int i=0;i<200;i++) free(created_files[i]);

  BF_CloseFile(info->fileDesc);
  free(info);
  BF_Close();
  fclose(logger);
}
