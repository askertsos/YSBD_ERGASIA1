#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "Logs.h"

#define RECORDS_NUM 200 // you can change it if you want
#define FILE_NAME "ht_1.db"

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

  log_info("Entered ht_main");

  BF_Init(LRU);

  HT_CreateFile(get_name_of_next_db(),10);
  HT_CreateFile(get_name_of_next_db(),10);
  HT_info* info = HT_OpenFile(FILE_NAME);

  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HT_InsertEntry(info, record);
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  HT_GetAllEntries(info, &id);

  HT_CloseFile(info);
  BF_Close();
}
