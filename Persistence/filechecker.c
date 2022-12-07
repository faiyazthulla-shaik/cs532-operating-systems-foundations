#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/mman.h> 
#include <sys/stat.h>

struct structArg
{
    char *loc;
    long initialName;
    long finalName;
};

void BubbleSort(int arrNum[], int value){
    for (int i=0; i < value-1; i++){
        for (int j=0; j<value-1-i; j++){
            if (arrNum[j] > arrNum[j+1]){
                int fill = arrNum[j];
                arrNum[j] = arrNum[j+1];
                arrNum[j+1] = fill;
            }
        }
    }
}

void *checkFiles(void *ptr)
{
    struct structArg *PTRStruct = (struct structArg *)ptr;
    char *loc = PTRStruct->loc;
    int *pointerStatus =  (int *)malloc(sizeof(int)); 

    for (long i = PTRStruct->initialName; i <= PTRStruct->finalName; i++){
        char arrUnSorFilePath[100];
        char arrSorFilePath[100];
        sprintf(arrSorFilePath, "%s/sorted/sorted_%ld.bin", loc, i);
        sprintf(arrUnSorFilePath, "%s/unsorted_%ld.bin", loc, i);        
        printf("checking %s and %s\n", arrUnSorFilePath, arrSorFilePath);
        int openSort = open(arrSorFilePath, O_RDONLY);
        if (openSort == -1){
            fprintf(stderr, "ERROR: Unable to open the file %s...\n", arrSorFilePath);
            pthread_exit(pointerStatus);
        }
        struct stat stStruct;
        int isFstat = fstat(openSort, &stStruct);
        if (isFstat < 0){
            fprintf(stderr, "ERROR: OMG! Stat was failed for %s...\n", arrSorFilePath);
            pthread_exit(pointerStatus);
        }
        int *mmapForSort = mmap (0, stStruct.st_size, PROT_READ, MAP_PRIVATE, openSort, 0);
        int *arrSor = (int *)malloc(stStruct.st_size);
        int nIntegers = stStruct.st_size / sizeof(int);

        for (int i = 0; i < nIntegers; i++){
#if DEBUG == 1
            printf("%d, ", mmapForSort[i]);
#endif
            arrSor[i] = mmapForSort[i];
        }
#if DEBUG == 1
        printf("\n");
#endif

        munmap(mmapForSort, stStruct.st_size);
        int openUnSor = open (arrUnSorFilePath, O_RDONLY);
        if (openUnSor == -1){
            fprintf(stderr, "ERROR: Unable to open the file %s...\n", arrUnSorFilePath);
            pthread_exit(pointerStatus);
        }
          
        isFstat = fstat(openUnSor, &stStruct);
        if (isFstat < 0){
            fprintf(stderr, "ERROR: OMG! Stat was failed for %s...\n", arrUnSorFilePath);
            pthread_exit(pointerStatus);
        }

         int *mmapForUnSort = mmap(0, stStruct.st_size, PROT_READ, MAP_PRIVATE, openUnSor, 0);

        int *arryToStorTemVal = (int *)malloc(stStruct.st_size);

        for (int i = 0; i < nIntegers; i++){
#if DEBUG == 1
            printf("%d== ", mmapForUnSort[i]);
#endif
            arryToStorTemVal[i] = mmapForUnSort[i];
        }
#if DEBUG == 1
        printf("\n");
#endif
        munmap(mmapForUnSort, stStruct.st_size);
        BubbleSort(arryToStorTemVal, nIntegers);

#if DEBUG == 1
        for (int i = 0; i < nIntegers; i++){
            printf("%d== ", arryToStorTemVal[i]);
        }
        printf("\n");
#endif
        for (int i = 0; i < nIntegers; i++) {
            if (arryToStorTemVal[i] != arrSor[i]){
                *pointerStatus = 1;
                pthread_exit(pointerStatus);
            }
        }
    }

    *pointerStatus = 2;
    pthread_exit(pointerStatus);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "ERROR: Please enter all the three arguments...\n");
        return -1;
    }
    char *direcPath = argv[1];
    printf("The entered directory was dir: %s \n", direcPath);
    if (access(direcPath, F_OK) == -1){
        fprintf(stderr, "ERROR: Unable to find the directory...\n");
        return -1;
    }
    if (access(direcPath, R_OK) == -1){
        fprintf(stderr, "ERROR: The directory doesn't have read permission.\n");
        return -1;
    }
    DIR *openFolder = opendir(direcPath);
    if (openFolder == NULL){
        fprintf(stderr, "ERROR: Unable to open the directory..\n");
        return -1;
    }
    struct dirent *rdDir;
    int isAvailable = 0;
    while ((rdDir = readdir(openFolder))){
        if (!strcmp(rdDir->d_name, "sorted")) {
            isAvailable = 1;
            break;
        }
    }
    if (isAvailable == 0){
        fprintf(stderr, "ERROR: Unable to find the sorted directory..\n");
        return -1;
    }
    long countTh = strtol(argv[2], NULL, 10);
    if (countTh < 1){
        fprintf(stderr, "ERROR: Please enter a valid thread count..\n");
        return -1;
    }
    pthread_t w1[countTh];
    int countFl = 0;
    rewinddir(openFolder);
    while ((rdDir = readdir(openFolder))){
        if (!strncmp(rdDir->d_name, "unsorted_", 9)){
            countFl++;
        }
    }
    long countOnTh = (countFl / countTh);
    printf("Count of total files: %d, number of threads: %ld, files per thread is %ld \n", countFl, countTh, countOnTh);
    for (long i = 0; i < countTh; i++){
#if 1
        struct structArg *poiForArg = (struct structArg *)malloc(sizeof(struct structArg));
#else
        struct structArg arguments;
        struct structArg *poiForArg = &arguments;
#endif
        poiForArg->loc = direcPath;
        poiForArg->initialName = i * countOnTh;
        poiForArg->finalName = poiForArg->initialName + countOnTh - 1;
        if (i == countTh - 1){
            poiForArg->finalName += countFl % countTh;
        }
        pthread_create(&w1[i], NULL, &checkFiles, poiForArg);
    }
    for (long i = 0; i < countTh; i++){
        void *pointerStatus;
        pthread_join(w1[i], &pointerStatus);
        int isFstat = *((int *)pointerStatus);
        if (isFstat == 0){
            return -1;
        } else if (isFstat == 1) {
            fprintf(stderr, "ERROR: Matching check was failed...\n");
            return -1;
        } 
    }
    fprintf(stdout, "Error matching check was success....\n");
    return 0;
}
