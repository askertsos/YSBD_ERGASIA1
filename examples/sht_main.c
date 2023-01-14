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
#define HT_DB_ROOT "ht_databases/ht_"
#define SHT_DB_ROOT "sht_databases/ht_"


int main() {

  //Initialize logger
  log_set_quiet(1);
  FILE * logger = fopen("./Logs/Logs.txt","w");
  log_add_fp(logger,1);

  log_info("Entered sht_main");

  BF_Init(LRU);

  SHT_CreateSecondaryIndex(create_sht_file(),10,create_ht_file());
  SHT_CreateSecondaryIndex(create_sht_file(),10,create_ht_file());
  SHT_CreateSecondaryIndex(create_sht_file(),10,create_ht_file());

  BF_Close();
}
