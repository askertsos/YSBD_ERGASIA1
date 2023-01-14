#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_table.h"
#include "ht_table.h"
#include "record.h"
#include "Logs.h"

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


int SHT_CreateSecondaryIndex(char* sfileName,  int buckets, char* fileName){

  /*Δημιουργία κενού αρχείου ευρετηρίου ht*/
  CALL_OR_DIE(HT_CreateFile(fileName,buckets));

  /*Δημιουργία σε BF επίπεδο το δευτερεύον αρχείο*/
  CALL_OR_DIE(BF_CreateFile(sfileName));

  int fdSec;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

  CALL_OR_DIE(BF_OpenFile(sfileName, &fdSec));

  // Allocate first block that contains metadata of file
  CALL_OR_DIE(BF_AllocateBlock(fdSec, block));
  data = BF_Block_GetData(block);
  SHT_info* file_info = data;
  file_info->buckets = buckets;
  file_info->fdPrim = -1;
  file_info->fdSec = -1;
  file_info->next_empty_bucket = buckets + 2;
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));

  // Allocate initial blocks for buckets
  for(int i=1; i <= buckets; i++){
    CALL_OR_DIE(BF_AllocateBlock(fdSec, block));
    data = BF_Block_GetData(block);
    HT_block_info* block_info = data;
    block_info->records_num = 0;
    block_info->bucket_id = i;
    block_info->next_bucket = -1;
    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));
  }

  // Free info block before closing file

  log_info("Created %s with info : {fdSec = %d | buckets = %d}",sfileName,file_info->fdSec,file_info->buckets);
  BF_Block_Destroy(&block);
  CALL_OR_DIE(BF_CloseFile(fdSec));

  return 0;


}

SHT_info* SHT_OpenSecondaryIndex(char *indexName){

}


int SHT_CloseSecondaryIndex( SHT_info* SHT_info ){

}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){

}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){

}
