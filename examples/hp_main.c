#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"
#include "Logs.h"

#define RECORDS_NUM 5 // you can change it if you want
#define FILE_NAME "hp_1.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);

  log_info("Entered hp_main");
  
  BF_Init(LRU);

  char* filename = get_name_of_next_db();
  log_info("filename is %s",filename);
  HP_CreateFile(filename);
  free(filename);
  filename = get_name_of_next_db();
  log_info("filename is %s",filename);
  HP_CreateFile(filename);
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
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n",id);
  HP_GetAllEntries(info, id);

  HP_CloseFile(info);
  BF_Close();
  fclose(logger);
}
