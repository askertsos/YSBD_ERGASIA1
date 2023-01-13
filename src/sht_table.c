#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_table.h"
#include "ht_table.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


int HashFunction(char* name, int buckets){
  int ascii_sum = 0;
  for(int i = 0;i < strlen(name); i++) ascii_sum += name[i];
  int hash_id = ascii_sum % buckets;
  log_info("Calculated {hash id : %d , name : %s}",hash_id,name);
  return hash_id;
}


int SHT_CreateSecondaryIndex(char *sfileName,  int buckets, char* fileName){

}

SHT_info* SHT_OpenSecondaryIndex(char *indexName){

}


int SHT_CloseSecondaryIndex( SHT_info* SHT_info ){

}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){

}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){

}
