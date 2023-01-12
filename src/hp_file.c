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
int hp_created = 0;


#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}

char* get_name_of_next_db(){
  // Each database will be named using DB_ROOT as a base
  // and hp_created as their id and will be of .db type.

  hp_created++;
  log_info("Creating name for id : %d",hp_created);
  char id[100];
  sprintf(id,"%d",hp_created);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  return f_name;
}

void* getBlockInfo(BF_Block* block){
    char* data = BF_Block_GetData(block);

    /*Εύρεση του τέλους του data*/
    void* start = data + BF_BUFFER_SIZE;

    /*Πλέον χωράει ακριβώς μια μεταβλητή τύπου HP_Block_info*/
    start = start - sizeof(HP_Block_info);

    return start;
}

int HP_CreateFile(char *fileName){
  log_info("Creating file : %s",fileName);
  CALL_OR_DIE(BF_CreateFile(fileName));

  int fd;
  void* data;
  BF_Block* block;
  BF_Block_Init(&block);

<<<<<<< HEAD
  CALL_OR_DIE(BF_OpenFile(fileName, &fd));
  CALL_OR_DIE(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  HP_info* file_info = data;
  file_info->blocks_num  = 1;
  file_info->fd = -1;
  file_info->type = 1;
  BF_Block_SetDirty(block);
  CALL_OR_DIE(BF_UnpinBlock(block));
  CALL_OR_DIE(BF_CloseFile(fd));
=======
  log_info("initiated block with address %p",block);
  



  CALL_BF(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);

  HP_info* info = data;
  info->fileDesc = fd;
  info->blocks = 1;
  // /*Εισαγωγή στα τελευταία (sizeof(HP_Block_info)) bytes του block*/
  // void* start = data + BF_BUFFER_SIZE;
  // start = data - sizeof(HP_Block_info);

  // HP_Block_info* hp_block_info = start;
  // hp_block_info->records = 0;
  // hp_block_info->nextBlock = NULL;
  // log_info("copied hp_block_info");
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_CloseFile(fd));
  BF_Block_Destroy(&block);
>>>>>>> 986472730e591e1dddc9c120cc480ff548057f26
    
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd));
  log_info("Opened file %s",fileName);

  BF_Block* block;
<<<<<<< HEAD
  BF_Block_Init(&block);
  int block_num;
  BF_GetBlockCounter(fd,&block_num);
  BF_GetBlock(fd,block_num-1,block);

  void* data;
  data = BF_Block_GetData(block);
  hp_info* file_info = data;
  file_info->fd = fd;

  //If file is not hp return NULL
  if(file_info->type != 0){
    free(file_info);
    CALL_OR_DIE(BF_CloseFile(fd));
    return NULL;
  }
  CALL_OR_DIE(BF_UnpinBlock(block));
  log_info("File info %s : {fd = %d | blocknum = %d}",fileName,file_info->fd,file_info->blocks_num);

  open_hp_files[open_hp_files_counter++] = file_info;

  return file_info;
}


int HP_CloseFile( HP_info* HP_info ){
  log_info("Closing file with fd %d",hp_info->fd);
  for(int i=0;i<open_hp_files_counter;i++){
    if( open_hp_files[i]->fd == hp_info->fd )
      BF_CloseFile(hp_info->fd);
      open_hp_files_counter--;
  }
=======
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
  HP_info* new = malloc(sizeof(*new));
  new->fileDesc = file_info->fileDesc;
  new->blocks = file_info->blocks;
  new->type = file_info->type;
  log_info("right before unpinBlock");
  CALL_BF(BF_UnpinBlock(block));
  log_info("File info %s : {fd = %d | blocknum = %d | type = %d}",fileName,file_info->fileDesc,file_info->blocks,file_info->type);

  BF_Block_Destroy(&block);
  // open_ht_files[open_ht_files_counter++] = file_info;

  return new;

}


int HP_CloseFile(HP_info* hp_info ){
  int fd = hp_info->fileDesc;
  CALL_BF(BF_CloseFile(fd));
  free(hp_info);

>>>>>>> 986472730e591e1dddc9c120cc480ff548057f26
  return 0;
}

int HP_InsertEntry(HP_info* hp_info, Record record){
  log_info("in insertEntry");

  int fd = hp_info->fileDesc;
  BF_Block* block;
  BF_Block_Init(&block);

  int lastBlock;
  CALL_BF(BF_GetBlockCounter(fd,&lastBlock));
  lastBlock -= 1;

  HP_Block_info* blockInfo;

  if (lastBlock == 0){
      log_info("last block = 0");
      CALL_BF(BF_AllocateBlock(fd,block));

      blockInfo = getBlockInfo(block);
      blockInfo->records = 0;
      blockInfo->nextBlock = NULL;

      log_info("created new blockInfo");
      log_info("Copied %s %d",record.record,record.id);
  }
  else{
      CALL_BF(BF_GetBlock(fd,1,block));
      blockInfo = getBlockInfo(block);
      log_info("lastBlock =  %d",lastBlock);
      int recordTotal = blockInfo->records;
      log_info("record total is %d",recordTotal);
      recordTotal += 1;
      int fits = (recordTotal * sizeof(Record) + sizeof(HP_Block_info)) <= BF_BUFFER_SIZE;
      char* data = BF_Block_GetData(block);
      if (fits){
        log_info("fits");
        
        void* start = data + blockInfo->records * sizeof(Record);
        memcpy(start,&record,sizeof(record));
        blockInfo->records += 1;
        
        log_info("Copied %s %d",record.record,record.id);
        log_info("Data inside is %s",(char*)start);
      }
      else{
        log_info("doesnt fit");

        /*Δημιουργία καινούριου block*/
        BF_Block* newBlock;
        BF_Block_Init(&newBlock);
        CALL_BF(BF_AllocateBlock(fd,newBlock));

        /*Ανανέωση του παλιού*/
        blockInfo->nextBlock = newBlock;
        log_info("updated oldBlockInfo");

        /*Εισαγωγή της εγγραφής στο καινούριο block*/
        void* newData = BF_Block_GetData(newBlock);
        memcpy(newData,&record,sizeof(record));

        HP_Block_info* newInfo = getBlockInfo(newBlock);
        newInfo->records = 1;
        newInfo->nextBlock = NULL;

        log_info("Copied %s %d",record.record,record.id);
        log_info("Data inside is %s",((Record*)newData)->name);
        BF_Block_SetDirty(newBlock);
        CALL_BF(BF_UnpinBlock(newBlock));
        BF_Block_Destroy(&newBlock);       
      }

  }
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);



  // CALL_BF(BF_OpenFile(fd));

  return 0;
}

int HP_GetAllEntries(HP_info* hp_info, int value){
  return 0;
}

