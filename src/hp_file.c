#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

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

/*Επιστρέφει δείκτη στο block_info ενός block*/
void* getBlockInfo(BF_Block* block){
    char* data = BF_Block_GetData(block);

    /*Εύρεση του τέλους του data*/
    void* start = data + BF_BLOCK_SIZE;

    /*Πλέον χωράει ακριβώς μια μεταβλητή τύπου HP_Block_info*/
    start = start - sizeof(HP_Block_info);

    return start;
}

int HP_CreateFile(char *fileName){
  BF_ErrorCode err = BF_CreateFile(fileName); if (err != BF_OK) return -1;

  int fd;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);
  
  err = BF_OpenFile(fileName,&fd); if (err != BF_OK) return -1;

  err = BF_AllocateBlock(fd, block); if (err != BF_OK) return -1;
  data = BF_Block_GetData(block);

  HP_info* info = data;
  info->fileDesc = fd;
  info->blocks = 1;

  /*Εισαγωγή στα τελευταία (sizeof(HP_Block_info)) bytes του block*/
  HP_Block_info* blockInfo = (HP_Block_info*)getBlockInfo(block);
  blockInfo->records = 0;
  blockInfo->nextBlock = NULL;

  BF_Block_SetDirty(block);
  err = BF_UnpinBlock(block); if (err != BF_OK) return -1;
  err = BF_CloseFile(fd); if (err != BF_OK) return -1;
  BF_Block_Destroy(&block);
    
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  if (openFiles == BF_MAX_OPEN_FILES)
    return NULL;
  int fd;
  BF_ErrorCode err = BF_OpenFile(fileName, &fd); if (err != BF_OK) return NULL;
  openFiles++;

  BF_Block* block;
  BF_Block_Init(&block);
  err = BF_GetBlock(fd,0,block); if (err != BF_OK) return NULL;

  void* data = BF_Block_GetData(block);
  HP_info* file_info = data;

  file_info->fileDesc = fd;

  /*Αν το αρχείο δεν είναι σωρού, επιστρέφεται NULL*/
  if(file_info->type != HEAP){
    return NULL;
  }

  /*Δημιουργία νέου στιγμιοτύπου της δομής HP_info, για να επιστραφεί*/
  HP_info* new = malloc(sizeof(*new));
  new->fileDesc = file_info->fileDesc;
  new->blocks = file_info->blocks;
  new->type = file_info->type;

  BF_Block_SetDirty(block);

  BF_Block_Destroy(&block);

  return new;

}


int HP_CloseFile(HP_info* hp_info){
  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);
  BF_ErrorCode err = BF_GetBlock(fd,0,block); if (err != BF_OK) return -1;

  err = BF_UnpinBlock(block); if (err != BF_OK) return -1;

  err = BF_CloseFile(fd); if (err != BF_OK) return -1;
  openFiles--;
  free(hp_info);
  return 0;

}

int HP_InsertEntry(HP_info* hp_info, Record record){

  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);

  /*Η εγγραφή θα γίνει στο τελευταίο block αν έχει χώρο, αλλιώς θα δημιουργηθεί καινούριο*/
  int lastBlock;
  BF_ErrorCode err = BF_GetBlockCounter(fd,&lastBlock); if (err != BF_OK) return -1;
  int blockID = lastBlock;
  lastBlock -= 1;


  /*Οι πληροφορίες για το τελευταίο block*/
  HP_Block_info* currentBlockInfo;

  if (lastBlock == 0){
      /*Δεν έχει δημιουργηθεί block με εγγραφές*/
      err = BF_AllocateBlock(fd,block); if (err != BF_OK) return -1;

      currentBlockInfo = getBlockInfo(block);
      void* data = BF_Block_GetData(block);
      memcpy(data,&record,sizeof(record));
      currentBlockInfo->records = 1;
      currentBlockInfo->nextBlock = NULL;

      /*Το blockID είναι ήδη 1*/

  }
  else{
      err = BF_GetBlock(fd,lastBlock,block); if (err != BF_OK) return -1;
      currentBlockInfo = getBlockInfo(block);
      int recordTotal = currentBlockInfo->records;
      recordTotal += 1;
      int fits = recordTotal <= RECORDS_PER_BLOCK;
      char* data = BF_Block_GetData(block);
      if (fits){
        /*Εισαγωγή αμέσως μετά το τέλος του προηγούμενου record*/
        void* recordPosition = data + currentBlockInfo->records * sizeof(Record);
        memcpy(recordPosition,&record,sizeof(record));
        currentBlockInfo->records += 1;
        blockID = lastBlock;
      }
      else{

        /*Δημιουργία καινούριου block*/
        BF_Block* newBlock;
        BF_Block_Init(&newBlock);
        err = BF_AllocateBlock(fd,newBlock); if (err != BF_OK) return -1;

        /*Ανανέωση του παλιού*/
        currentBlockInfo->nextBlock = newBlock;

        /*Εισαγωγή της εγγραφής στο καινούριο block*/
        void* newData = BF_Block_GetData(newBlock);
        memcpy(newData,&record,sizeof(record));

        HP_Block_info* newInfo = getBlockInfo(newBlock);
        newInfo->records = 1;
        newInfo->nextBlock = NULL;

        BF_Block_SetDirty(newBlock);
        err = BF_UnpinBlock(newBlock); if (err != BF_OK) return -1;
        BF_Block_Destroy(&newBlock);
        /*Το blockID είναι ήδη = lastBlock+1*/       
      }

  }
  BF_Block_SetDirty(block);
  err = BF_UnpinBlock(block); if (err != BF_OK) return -1;
  BF_Block_Destroy(&block);

  return blockID;
}

int HP_GetAllEntries(HP_info* hp_info, int value){

  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);

  /*Ο αριθμός των blocks που έχει το αρχείο. Θα εξεταστούν όλα*/
  int allBlocks;
  BF_ErrorCode err = BF_GetBlockCounter(fd,&allBlocks); if (err != BF_OK) return -1;


  int blocksRead = -1;

  for (int i=0; i<allBlocks; i++){
    err = BF_GetBlock(fd,i,block); if (err != BF_OK) return -1;
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

