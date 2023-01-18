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

int open_sht_files_counter = 0;

int HashFunction(char* name, int buckets){
  int ascii_sum = 0;
  for(int i = 0;i < strlen(name); i++) ascii_sum += name[i];
  int hash_id = ascii_sum % buckets + 1;
  log_info("Calculated {hash id : %d , name : %s}",hash_id,name);
  return hash_id;
}

void* nextEntryPosition(void* start, int entries){
  return start + sizeof(SHT_block_info) + entries * sizeof(SHT_entry);
}

int SHT_CreateSecondaryIndex(char* sfileName,  int buckets, char* fileName){

  int check = BF_CreateFile(sfileName); if (check != BF_OK) return -1;

  int fileDesc;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

  check = BF_OpenFile(sfileName, &fileDesc); if (check != BF_OK) return -1;

  /*Δημιουργία του 1ου μπλοκ, το οποίο θα περιέχει μετα-δεδομένα για το αρχείο*/
  check = BF_AllocateBlock(fileDesc, block); if (check != BF_OK) return -1;

  data = BF_Block_GetData(block);
  SHT_info* file_info = data;
  file_info->buckets = buckets;
  file_info->fileDesc = -1;
  file_info->next_empty_bucket = buckets + 1;
  BF_Block_SetDirty(block);
  check = BF_UnpinBlock(block); if (check != BF_OK) return -1;

  /*Δέσμευση αρκετού χώρου για όλα τα buckets*/
  for(int i=1; i <= buckets; i++){
    check = BF_AllocateBlock(fileDesc, block); if (check != BF_OK) return -1;

    data = BF_Block_GetData(block);
    /*Το block_info θα είναι στην αρχή κάθε block/bucket*/
    SHT_block_info* block_info = data;
    block_info->entries = 0;
    block_info->bucket_id = i;
    block_info->next_bucket = -1;
    BF_Block_SetDirty(block);
    check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
  }
  

  log_info("Created %s with info : {fdSec = %d | buckets = %d}",sfileName,fileDesc,file_info->buckets);
  BF_Block_Destroy(&block);
  check = BF_CloseFile(fileDesc); if (check != BF_OK) return -1;

  return 0;


}

SHT_info* SHT_OpenSecondaryIndex(char *indexName){

  if (open_sht_files_counter == (BF_MAX_OPEN_FILES/2)){
    log_info("Max open files reached");
    return NULL;    
  }

  int fd;
  int check = BF_OpenFile(indexName, &fd); if (check != BF_OK) return NULL;

  BF_Block* block;
  BF_Block_Init(&block);
  check = BF_GetBlock(fd,0,block); if (check != BF_OK) return NULL;

  void* data;
  data = BF_Block_GetData(block);
  SHT_info* temp = data;

  temp->fileDesc = fd;
  BF_Block_SetDirty(block);

  SHT_info* info = malloc(sizeof(SHT_info));

  /*Αντιγραφή του info από τη μνήμη*/
  info->fileDesc = temp->fileDesc;
  info->buckets = temp->buckets;
  info->next_empty_bucket = temp->next_empty_bucket;

  BF_Block_Destroy(&block);
  log_info("Opened file %s",indexName);  
  return info;
}


int SHT_CloseSecondaryIndex( SHT_info* SHT_info ){
  log_info("Closing secondary file with info: {fdSec = %d | buckets = %d }",SHT_info->fileDesc,SHT_info->buckets);
  BF_Block* block;
  BF_Block_Init(&block);
  int check = BF_GetBlock(SHT_info->fileDesc,0,block); if (check != BF_OK) return -1;
  check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
  BF_Block_Destroy(&block);
  BF_ErrorCode err = BF_CloseFile(SHT_info->fileDesc);
  if(err == BF_OK){
    open_sht_files_counter--;
    free(SHT_info);
    return 0;
  }
  BF_PrintError(err);
  return -1;
}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){

  log_info("in SHT_SecondaryInsertEntry for %d",record.id);

  /*Αρχικοποίηση της δομής SHT_entry, η οποία θα εισαχθεί στο αρχείο*/
  SHT_entry newEntry;
  strcpy(newEntry.name,record.name);
  newEntry.bucket = block_id;

  /*Αρχικοποίηση της δομής block*/
  BF_Block* block;
  BF_Block_Init(&block);

  int hash_id = HashFunction(newEntry.name,sht_info->buckets);
  /*Το block στο οποίο θα εισαχθεί η εγγραφή*/
  int block_to_insert = hash_id;

  int check = BF_GetBlock(sht_info->fileDesc,block_to_insert,block); if (check != BF_OK) return -1;
  void* data = BF_Block_GetData(block);
  SHT_block_info* block_info = data;

  if(block_info->entries == ENTRIES_PER_BLOCK){
    int new_bucket = sht_info->next_empty_bucket; //Get the first available empty bucket and allocate it
    printf("maxxed out: next bucket is %d\n",new_bucket);
    block_info->next_bucket = new_bucket; //Update previous bucket to point to new bucket
    printf("new bucket is %d\n",new_bucket);
    sht_info->next_empty_bucket += 1; //Update next_empty_bucket variable for next allocation

    BF_Block* new_block;
    BF_Block_Init(&new_block);
    check = BF_AllocateBlock(sht_info->fileDesc, new_block); if (check != BF_OK) return -1;
    printf("block allocated\n");
    void* new_data = BF_Block_GetData(new_block);
    SHT_block_info* new_block_info = new_data;
    new_block_info->entries = 0;
    new_block_info->bucket_id = new_bucket;
    new_block_info->next_bucket = -1;

    BF_Block_SetDirty(block);
    check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    block = new_block;
    block_info = new_block_info;
    BF_Block_Destroy(&new_block);
    printf("block destroyed\n");
  }  

  SHT_block_info* temp = block_info;
  /*Όσο είναι γεμάτο το block, πάμε στο επόμενο που αντιστοιχεί στο ίδιο hash_id*/
  while (block_info->next_bucket != -1){
    block_to_insert = block_info->next_bucket;
    BF_GetBlock(sht_info->fileDesc,block_to_insert,block);
    void* data = BF_Block_GetData(block);
    block_info = data;
    check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
  }

  void* pos = nextEntryPosition(data,block_info->entries);
  memcpy(pos,&newEntry,sizeof(SHT_entry));
  block_info->entries += 1;

  log_info("inserted entry: { %s, %d } which hashed as %d at bucket %d",newEntry.name,newEntry.bucket,hash_id,block_to_insert);
  BF_Block_SetDirty(block);
  check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
  BF_Block_Destroy(&block);

}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){
  log_info("in SHT_SecondaryGetAllEntries: Searching for %s",name);
  int fdPrim = ht_info->fd;
  int fdSec = sht_info->fileDesc;
  int blocksRead = 0;
  bool found = false;

  int hash_id = HashFunction(name,sht_info->buckets);

  BF_Block* blockSec;
  BF_Block* blockPrim;
  BF_Block_Init(&blockSec);
  BF_Block_Init(&blockPrim);


  SHT_block_info* infoSec;
  int block_to_insert = hash_id;
  do {
    blocksRead++;
    int check = BF_GetBlock(fdSec,block_to_insert,blockSec); if (check != BF_OK) return -1;
    void* dataSec = BF_Block_GetData(blockSec);
    infoSec = dataSec;
    SHT_entry* entry = dataSec + sizeof(SHT_block_info);
    int entryPos = 0;
    for (int i=0; i<infoSec->entries; i++){

      /*Μπορεί πολλαπλά ονόματα να επιστρέφουν το ίδιο hash_id, οπότε ελέγχουμε*/
      if (strcmp(entry->name,name) == 0){
        int block_id = entry->bucket;
        check = BF_GetBlock(fdPrim,block_id,blockPrim); if (check != BF_OK) return -1;
        blocksRead++;
        void* dataPrim = BF_Block_GetData(blockPrim);
        HT_block_info* infoPrim = dataPrim;
        Record* record = dataPrim + sizeof(HT_block_info);
        /*Θα ελέγξουμε τις εγγραφές του block, στο οποίο ξέρουμε πως βρίσκεται η ζητούμενη, μία-μία ως προς το όνομα*/
        for (int j=0; j<infoPrim->records_num; j++){

          if (strcmp(record->name,name) == 0){
            found = true;
            printRecord(*record);
          }
          /*Το record = record + sizeof(Record) για κάποιον λόγο δεν λειτουργούσε*/
          record = dataPrim + sizeof(HT_block_info) + (j+1) * sizeof(Record);
        }
        check = BF_UnpinBlock(blockPrim); if (check != BF_OK) return -1;
      }
      entryPos++;
      /*Παρόμοια κι εδώ*/
      entry = dataSec + sizeof(SHT_block_info) + entryPos * sizeof(SHT_entry);
    }
    block_to_insert = infoSec->next_bucket;
    /*Δεν αλλάζει το block, άρα δεν χρειάζεται SetDirty*/
    check = BF_UnpinBlock(blockSec); if (check != BF_OK) return -1;
  }
  while (infoSec->next_bucket != -1);
  BF_Block_Destroy(&blockPrim);
  BF_Block_Destroy(&blockSec);
  if (found) return blocksRead;
  return -1;
}
