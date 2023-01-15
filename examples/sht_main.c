#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"

#define RECORDS_NUM 103 // you can change it if you want
#define FILE_NAME "data.db"
#define INDEX_NAME "index.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int HashStatisticsHT(char* filename);
int HashStatisticsSHT(char* filename);


int main() {
    srand(12569874);
    srand(time(NULL));
    BF_Init(LRU);
    // Αρχικοποιήσεις
    HT_CreateFile(FILE_NAME,10);
    SHT_CreateSecondaryIndex(INDEX_NAME,10,FILE_NAME);
    HT_info* info = HT_OpenFile(FILE_NAME);
    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);

    Record records[RECORDS_NUM];

    // Θα ψάξουμε στην συνέχεια το όνομα searchName
    Record record=randomRecord();
    char searchName[15];
    strcpy(searchName, record.name);

    // Κάνουμε εισαγωγή τυχαίων εγγραφών τόσο στο αρχείο κατακερματισμού τις οποίες προσθέτουμε και στο δευτερεύον ευρετήριο
    printf("Insert Entries\n");
    for (int id = 0; id < RECORDS_NUM; ++id) {
        record = randomRecord();
        records[id] = record;
        int block_id = HT_InsertEntry(info, record);
        SHT_SecondaryInsertEntry(index_info, record, block_id);
    }
    // Τυπώνουμε όλες τις εγγραφές με όνομα searchName
    printf("RUN PrintAllEntries for name %s\n",searchName);
    int blocksRead = SHT_SecondaryGetAllEntries(info,index_info,searchName);
    if (blocksRead == -1){
      printf("%s not found\n",searchName);
    }
    else{
      printf("%s found after reading %d blocks\n",searchName,blocksRead);
    }

    for (int i=0; i<RECORDS_NUM; i++){
      strcpy(searchName,records[i].name);
      printf("searching for %s\n",searchName);
      int blocksRead = SHT_SecondaryGetAllEntries(info,index_info,searchName);
      if (blocksRead == -1) printf("%s not found\n",searchName);
      else printf("%s found after reading %d blocks\n",searchName,blocksRead);
    }


    // Κλείνουμε το αρχείο κατακερματισμού και το δευτερεύον ευρετήριο
    SHT_CloseSecondaryIndex(index_info);
    HT_CloseFile(info);
    //

    printf("Statistics:\n\n");
    if (HashStatisticsHT(FILE_NAME) == -1) printf("HashStatisticsHT failed for %s\n",FILE_NAME);
    if (HashStatisticsSHT(INDEX_NAME) == -1) printf("HashStatisticsSHT failed for %s\n",INDEX_NAME);
    BF_Close();
}

int HashStatisticsHT(char* filename){
  HT_info* fileInfo = HT_OpenFile(filename); if (fileInfo == NULL) return -1;

  BF_Block* block;
  BF_Block_Init(&block);

  const int buckets = fileInfo->buckets;
  const int fd = fileInfo->fd;

  int check;
  void* data;
  HT_block_info* block_info;

  //1
  printf("1.\n");
  int blockTotal = 1; /*Το block 0*/
  for (int i=2; i<=buckets; i++){
    check = BF_GetBlock(fd,i,block); if (check != BF_OK) return -1;
    data = BF_Block_GetData(block);
    block_info = data;
    blockTotal++;
    while (block_info->next_bucket != -1){
      check = BF_GetBlock(fd,block_info->next_bucket,block); if (check != BF_OK) return -1;
      data = BF_Block_GetData(block);
      block_info = data;
      blockTotal++;
    }
  }
  printf("File %s has %d blocks total\n\n",filename,blockTotal);

  //2
  printf("2.\n");
  int recordTotal = 0;
  int max = 0;
   /*Στη χειρότερη περίπτωση όλες οι εγγραφές είναι στο ίδιο bucket, το οποίο περιέχει όλα τα blocks, και είναι όλα γεμάτα. Και πάλι, αυτός ο αριθμός είναι μεγαλύτερος λόγω του block 0*/
  int min = RECORDS_PER_BLOCK * blockTotal;
  for (int i=2; i<=buckets; i++){
    check = BF_GetBlock(fd,i,block); if (check != BF_OK) return -1;
    data = BF_Block_GetData(block);
    HT_block_info* block_info = data;
    int records_in_bucket = block_info->records_num;
    while (block_info->next_bucket != -1){
      check = BF_GetBlock(fd,block_info->next_bucket,block); if (check != BF_OK) return -1;
      data = BF_Block_GetData(block);
      block_info = data;
      records_in_bucket += block_info->records_num;
    }
    if (records_in_bucket < min) min = records_in_bucket;
    /*Όταν i=1, ισχύουν και τα δύο*/
    if (records_in_bucket > max) max = records_in_bucket;
    recordTotal += records_in_bucket;
  }
  double averageRec = (double)((double)recordTotal / (double)buckets);
  printf("Fewest records in a bucket: %d\nMost records in a bucket: %d\nAverage records per bucket: %lf\n",min,max,averageRec);

  //3
  printf("3.\n");
  double averageBlocks = (double)((double)blockTotal/(double)buckets);
  printf("The average bucket has %lf blocks\n\n",averageBlocks);

  //4
  printf("4.\n");
  int* extra = malloc(buckets * sizeof(int));
  for (int i=0; i<buckets; i++) extra[i] = 0;
  for (int i=2; i<=buckets; i++){
    check = BF_GetBlock(fd,i,block); if (check != BF_OK) return -1;
    void* data = BF_Block_GetData(block);
    HT_block_info* block_info = data;
    while (block_info->next_bucket != -1){
      extra[i-1] = extra[i-1] + 1;
      check = BF_GetBlock(fd,block_info->next_bucket,block); if (check != BF_OK) return -1;
      data = BF_Block_GetData(block);
      block_info = data;
    }
  }
  int overflown = 0;
  for (int i=0; i<buckets; i++){
    if (extra[i] > 0)
      overflown++;
  }
  if (overflown>0){
    printf("These %d buckets have been overflown:\n",overflown);
    for (int i=0; i<buckets; i++){
      if (extra[i] > 0)
        printf("%d: %d extra bucket\n",(i+1),extra[i]);
    }
  }
  else printf("No bucket was overflown\n");
  printf("\n");
  free(extra);

  check = HT_CloseFile(fileInfo); if (check != BF_OK) return -1;
  return 0;


}

int HashStatisticsSHT(char* filename){
  SHT_info* fileInfo = SHT_OpenSecondaryIndex(filename); if (fileInfo == NULL) return -1;

  BF_Block* block;
  BF_Block_Init(&block);

  const int buckets = fileInfo->buckets;
  const int fd = fileInfo->fileDesc;

  int check;
  void* data;
  SHT_block_info* block_info;

  //1
  printf("\n1.\n");
  int blockTotal = 1; /*Το block 0*/
  for (int i=1; i<=buckets; i++){
    check = BF_GetBlock(fd,i,block); if (check != BF_OK) return -1;
    data = BF_Block_GetData(block);
    block_info = data;
    blockTotal++;
    check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    while (block_info->next_bucket != -1){
      printf("next bucket is %d\n",block_info->next_bucket);
      check = BF_GetBlock(fd,block_info->next_bucket,block); if (check != BF_OK) return -1;
      data = BF_Block_GetData(block);
      block_info = data;
      blockTotal++;
      check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    }
  }
  printf("File %s has %d blocks total\n",filename,blockTotal);

  //2
  printf("\n2.\n");
  int entryTotal = 0;
  int max = 0;
   /*Στη χειρότερη περίπτωση όλες οι εγγραφές είναι στο ίδιο bucket, το οποίο περιέχει όλα τα blocks, και είναι όλα γεμάτα. Και πάλι, αυτός ο αριθμός είναι μεγαλύτερος λόγω του block 0*/
  int min = ENTRIES_PER_BLOCK * blockTotal;
  for (int i=1; i<=buckets; i++){
    check = BF_GetBlock(fd,i,block); if (check != BF_OK) return -1;
    data = BF_Block_GetData(block);
    SHT_block_info* block_info = data;
    int entries_in_bucket = block_info->entries;
    check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    while (block_info->next_bucket != -1){
      check = BF_GetBlock(fd,block_info->next_bucket,block); if (check != BF_OK) return -1;
      data = BF_Block_GetData(block);
      block_info = data;
      entries_in_bucket += block_info->entries;
      check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    }
    if (entries_in_bucket < min) min = entries_in_bucket;
    /*Όταν i=1, ισχύουν και τα δύο*/
    if (entries_in_bucket > max) max = entries_in_bucket;
    entryTotal += entries_in_bucket;
  }
  double averageEnt = (double)((double)entryTotal / (double)buckets);
  printf("Fewest entries in a bucket: %d\nMost entries in a bucket: %d\nAverage entries per bucket: %lf\n",min,max,averageEnt);

  //3
  printf("\n3.\n");
  double averageBlocks = (double)((double)blockTotal/(double)buckets);
  printf("The average bucket has %lf blocks\n",averageBlocks);

  //4
  printf("\n4.\n");
  int* extra = malloc(buckets * sizeof(int));
  for (int i=0; i<buckets; i++) extra[i] = 0;
  for (int i=1; i<=buckets; i++){
    check = BF_GetBlock(fd,i,block); if (check != BF_OK) return -1;
    void* data = BF_Block_GetData(block);
    HT_block_info* block_info = data;
    check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    while (block_info->next_bucket != -1){
      extra[i-1] = extra[i-1] + 1;
      check = BF_GetBlock(fd,block_info->next_bucket,block); if (check != BF_OK) return -1;
      data = BF_Block_GetData(block);
      block_info = data;
      check = BF_UnpinBlock(block); if (check != BF_OK) return -1;
    }
  }
  int overflown = 0;
  for (int i=0; i<buckets; i++){
    if (extra[i] > 0)
      overflown++;
  }
  if (overflown > 0){
    printf("These %d buckets have been overflown:\n",overflown);
    for (int i=0; i<buckets; i++){
      if (extra[i] > 0)
        printf("%d: %d extra bucket\n",(i+1),extra[i]);
    }
  }
  else printf("No bucket was overflown\n");
  printf("\n");
  free(extra);

  check = SHT_CloseSecondaryIndex(fileInfo); if (check != BF_OK) return -1;
  return 0;

}
