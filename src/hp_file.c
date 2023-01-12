#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "Logs.h"

// Base name to create all hp_databases from
#define DB_ROOT "hp_databases/hp_"
// Store number of created databases


int openFiles = 0;

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    exit(code);        \
  }                         \
}

void* getBlockInfo(BF_Block* block){
    char* data = BF_Block_GetData(block);

    /*Εύρεση του τέλους του data*/
    void* start = data + BF_BLOCK_SIZE;

    /*Πλέον χωράει ακριβώς μια μεταβλητή τύπου HP_Block_info*/
    start = start - sizeof(HP_Block_info);

    return start;
}

int HP_CreateFile(char *fileName){
  log_info("Creating file : %s",fileName);
  CALL_BF(BF_CreateFile(fileName));

  int fd;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

  log_info("initiated block with address %p",block);
  
  CALL_BF(BF_OpenFile(fileName,&fd));

  CALL_BF(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);

  HP_info* info = data;
  info->fileDesc = fd;
  info->blocks = 1;

  /*Εισαγωγή στα τελευταία (sizeof(HP_Block_info)) bytes του block*/
  HP_Block_info* blockInfo = (HP_Block_info*)getBlockInfo(block);
  blockInfo->records = 0;
  blockInfo->nextBlock = NULL;

  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_CloseFile(fd));
  BF_Block_Destroy(&block);
    
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  if (openFiles == BF_MAX_OPEN_FILES)
    return NULL;
  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd));\
  openFiles++;
  log_info("Opened file %s",fileName);

  BF_Block* block;
  BF_Block_Init(&block);
  BF_OpenFile(fileName,&fd);
  BF_GetBlock(fd,0,block);

  void* data = BF_Block_GetData(block);
  HP_info* file_info = data;
  log_info("File info %s : {fd = %d| newFD = %d | blocknum = %d | type = %d}",fileName,file_info->fileDesc,fd,file_info->blocks,file_info->type);

  file_info->fileDesc = fd;
  log_info("copied fd");

  //If file is not heap-type return NULL
  if(file_info->type != HEAP){
    CALL_BF(BF_CloseFile(fd));
    return NULL;
  }

  /*Δημιουργία νέου στιγμιοτύπου της δομής HP_info, για να επιστραφεί*/
  HP_info* new = malloc(sizeof(*new));
  new->fileDesc = file_info->fileDesc;
  new->blocks = file_info->blocks;
  new->type = file_info->type;
  log_info("right before unpinBlock");
  // CALL_BF(BF_UnpinBlock(block));
  BF_Block_SetDirty(block);
  log_info("File info %s : {fd = %d | blocknum = %d | type = %d}",fileName,file_info->fileDesc,file_info->blocks,file_info->type);

  BF_Block_Destroy(&block);
  // open_ht_files[open_ht_files_counter++] = file_info;

  return new;

}


int HP_CloseFile(HP_info* hp_info){
  printf("in hp_closefile\n");
  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);
  BF_GetBlock(fd,0,block);

  CALL_BF(BF_UnpinBlock(block));

  /*Το return value της HP_CloseFile είναι ακριβώς το ίδιο με αυτό της BF_CloseFile*/
  int result = BF_CloseFile(fd);
  if (result)
    openFiles--;
  return result;

}

int HP_InsertEntry(HP_info* hp_info, Record record){
  log_info("in insertEntry");

  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);

  /*Η εγγραφή θα γίνει στο τελευταίο block αν έχει χώρο, αλλιώς θα δημιουργηθεί καινούριο*/
  int lastBlock;
  CALL_BF(BF_GetBlockCounter(fd,&lastBlock));
  int blockID = lastBlock;
  lastBlock -= 1;


  /*Οι πληροφορίες για το τελευταίο block*/
  HP_Block_info* currentBlockInfo;

  if (lastBlock == 0){
      /*Δεν έχει δημιουργηθεί block με εγγραφές*/
      log_info("last block = 0");
      printf("last block = 0\n");
      CALL_BF(BF_AllocateBlock(fd,block));

      currentBlockInfo = getBlockInfo(block);
      void* data = BF_Block_GetData(block);
      memcpy(data,&record,sizeof(record));
      currentBlockInfo->records = 1;
      currentBlockInfo->nextBlock = NULL;

      log_info("created new currentBlockInfo");
      log_info("Data:city inside is %s",((Record*)data)->city);
      /*Το blockID είναι ήδη 1*/

  }
  else{
      log_info("lastBlock =  %d",lastBlock);

      CALL_BF(BF_GetBlock(fd,lastBlock,block));
      currentBlockInfo = getBlockInfo(block);
      int recordTotal = currentBlockInfo->records;
      log_info("record total is %d",recordTotal);
      recordTotal += 1;
      int fits = recordTotal <= RECORDS_PER_BLOCK;
      char* data = BF_Block_GetData(block);
      if (fits){
        log_info("fits");
        
        /*Εισαγωγή αμέσως μετά το τέλος του προηγούμενου record*/
        void* recordPosition = data + currentBlockInfo->records * sizeof(Record);
        memcpy(recordPosition,&record,sizeof(record));
        currentBlockInfo->records += 1;
        blockID = lastBlock;
        
        log_info("Copied %s %d",record.record,record.id);
        log_info("Data:id inside is %d",((Record*)recordPosition)->id);
      }
      else{
        log_info("doesnt fit");

        /*Δημιουργία καινούριου block*/
        BF_Block* newBlock;
        BF_Block_Init(&newBlock);
        CALL_BF(BF_AllocateBlock(fd,newBlock));

        /*Ανανέωση του παλιού*/
        currentBlockInfo->nextBlock = newBlock;
        log_info("updated oldBlockInfo");

        /*Εισαγωγή της εγγραφής στο καινούριο block*/
        void* newData = BF_Block_GetData(newBlock);
        memcpy(newData,&record,sizeof(record));

        HP_Block_info* newInfo = getBlockInfo(newBlock);
        newInfo->records = 1;
        newInfo->nextBlock = NULL;

        log_info("Copied %s %d",record.record,record.id);
        log_info("Data:name inside is %s",((Record*)newData)->name);
        BF_Block_SetDirty(newBlock);
        CALL_BF(BF_UnpinBlock(newBlock));
        BF_Block_Destroy(&newBlock);
        /*Το blockID είναι ήδη = lastBlock+1*/       
      }

  }
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return blockID;
}

int HP_GetAllEntries(HP_info* hp_info, int value){

  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);

  /*Ο αριθμός των blocks που έχει το αρχείο. Θα εξεταστούν όλα*/
  int allBlocks;
  BF_GetBlockCounter(fd,&allBlocks);


  int blocksRead = -1;

  for (int i=0; i<allBlocks; i++){
    BF_GetBlock(fd,i,block);
    void* data = BF_Block_GetData(block);
    HP_Block_info* blockInfo = (HP_Block_info*)getBlockInfo(block);
    int records = blockInfo->records;

    for (int j=0; j<records; j++){
      Record* record = data + j * sizeof(Record);
      if (record->id == value){
        printRecord(*record);
        blocksRead = i+1;

      }
    }
  }

  BF_Block_Destroy(&block);
  return blocksRead;
}

