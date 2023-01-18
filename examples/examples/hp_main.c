#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 70 // you can change it if you want
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

/*Βάση για το όνομα των αρχείο .db*/
#define DB_ROOT "hp_databases/hp_"

/*Ο αριθμός των δημιουργημένων αρχείων. Χρησιμοποιείται για την ονομασία τους*/
int filesCreated = 0;


char* filenameGenerator(){
  /*Κάθε αρχείο σωρού θα ονομάζεται "hp_databases/hp_<filesCreated>"*/
  filesCreated++;
  char* id = malloc(100);
  sprintf(id,"%d",filesCreated);
  char* f_name = malloc(strlen(DB_ROOT) + strlen(id) + 4);
  strcpy(f_name,DB_ROOT);
  strcat(f_name,id);
  strcat(f_name,".db");
  free(id);
  return f_name;
}


int main() {
  BF_Init(LRU);
  char* filename = filenameGenerator();

  /*Θα χρησιμοποιείται για τον έλεγχο σωστής λειτουργίας του προγράμματος, ανάλογα με το τι επιστρέφουν οι συναρτήσεις*/
  int check;

  check = HP_CreateFile(filename); if (check == -1) printf("HP_CreateFile failed for %s\n",filename);
  HP_info* info = HP_OpenFile(filename); if (info == NULL) printf("HP_OpenFile failed for %s\n",filename);
  free(filename);
  Record record;
  srand(time(NULL));

  printf("Insert Entries\n");
  /*Για RECORDS_NUM επαναλήψεις, επιλέγεται ένα τυχαίο record, και επιλέγεται τυχαία ο αριθμός των φορών που θα εισαχθεί στο αρχείο (1-3 φορές).
  Γίνεται για να δειχθεί πως δουλεύει σωστά η GetAllEntries, ότι δηλαδή εκτυπώνει όλα τα records με το ζητούμενο id*/
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    int times = rand () % 3 + 1;
    for (int i=0; i<times; i++)
      check = HP_InsertEntry(info, record); if (check == -1) printf("HP_InsertEntry failed for id %d\n",record.id);
  }

  printf("RUN PrintAllEntries\n");
  /*Εκτυπώνει όλα τα records που έχουν εισαχθεί στο αρχείο*/
  for (int id = 0; id < RECORDS_NUM; ++id){
    printf("\nSearching for: %d\n",id);
    int blocks = HP_GetAllEntries(info, id);

    if (blocks > 0) 
      printf("Read %d blocks\n",blocks);
    else 
      printf("id #%d not found in file\n",id);

  }

  check = HP_CloseFile(info); if (check == -1) printf("HP_CloseFile failed for %d\n",info->fileDesc);
  BF_Close();
}
