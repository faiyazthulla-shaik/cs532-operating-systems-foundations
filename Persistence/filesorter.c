#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void swap(int *temp1, int *temp2){
    int varTemp = *temp1;
    *temp1 = *temp2;
    *temp2 = varTemp;
}

void funToSortArray(int inputArr[], int count) {
    int g, j;
    for (g = 0; g < count-1; g++){
        for (j = 0; j < count-g-1; j++){
            if (inputArr[j] > inputArr[j+1]){
                swap(&inputArr[j], &inputArr[j+1]);
            }
        }
    }
}

int funToValidateUnsortedFiles(char* createTargetFolder) {
    int c = 0;
    DIR *dirOpenDir;
    struct dirent *readDirecT;
    if ((dirOpenDir = opendir (createTargetFolder)) != NULL){
        while ((readDirecT = readdir (dirOpenDir)) != NULL) {
            if((strstr(readDirecT->d_name, "unsorted_") != NULL) && (strstr(readDirecT->d_name, ".bin")!=NULL)) {
                c++;
            }
        }
    }
    if(c == 0){
        fprintf(stderr,"There is an error here!!\n Input directory doesn't have unsorted_<ID>.bin files\n");
        return -1;
    }
    else{
        return 0;
    }

}

int funToValidateSorting(char* createTargetFolder){
    int c = 0;
    DIR *dirOpenDir;
    struct dirent *readDirecT;
    if ((dirOpenDir = opendir (createTargetFolder)) != NULL){
        while ((readDirecT = readdir (dirOpenDir)) != NULL) {
            if(strstr(readDirecT->d_name, "sorted_") != NULL) {
                c++;
            }
        }
    }
    if(c == 0){
        return 0;
    }
    else{
        fprintf(stderr,"There is an error here!!\n Target directory already have sorted_<ID>.bin files\n");
        return -1;
    }
}

int main(int argc, char **argv) {
    char *pathOfFiles = argv[1];
    if (pathOfFiles == NULL) {
        fprintf(stderr, "The location you entered is not available.\n Exiting...\n");
        return -1;
    }

    int isDirectoryFound = access(pathOfFiles, F_OK);
    if (isDirectoryFound == -1) {
        fprintf(stderr, "The mentioned directory was not found.\n Exiting\n");
        return -1;
    }

    int isProgramHasReadAccess = access(pathOfFiles, R_OK);
    if (isProgramHasReadAccess == -1) {
        fprintf(stderr, "The directory has no permission.\n Exiting\n");
        return -1;
    }

    int isUnsortedFilesExists = funToValidateUnsortedFiles(pathOfFiles);
    if(isUnsortedFilesExists== -1) {
        fprintf(stderr, "The directory has no unsorted files.\n Exiting\n");
        return -1;
    }
    
    char *createTargetFolder = (char*)malloc(strlen(pathOfFiles) + 10);
    sprintf(createTargetFolder, "%s/sorted/", pathOfFiles);
    int isTargetFolderExists = access(createTargetFolder, F_OK);
    if (isTargetFolderExists == -1) {
        fprintf(stderr, "The target directory %s was not created previously, creating it\n", createTargetFolder);
        mkdir(createTargetFolder, 0777);
    }else{
        int areSortedFilesExistsInTarget = funToValidateSorting(createTargetFolder);
        if (areSortedFilesExistsInTarget == -1) {
            fprintf(stderr, "Sorted files are already exists in the target folder\n. Exiting.....\n");
            return -1;
        }
    }

    int isTargetFolderHasAccess = access(createTargetFolder, W_OK);
    if (isTargetFolderHasAccess == -1) {
        fprintf(stderr, "Target folder denied the permission\n. Exiting\n");
        return -1;
    }

    printf("Sorting the files from target location: %s\n", pathOfFiles);
    DIR *ptrOpenDir = opendir(pathOfFiles);
    struct dirent* inFile;
    int countOfFiles = 0;

    while ((inFile = readdir(ptrOpenDir))) {
        if (!strcmp (inFile->d_name, "."))
            continue;
        if (!strcmp (inFile->d_name, ".."))
            continue;
        if (!strcmp (inFile->d_name, "sorted"))
            continue;
        
        countOfFiles += 1;
        char *pathOfSortFile = (char*)malloc(strlen(pathOfFiles) + 20);
        sprintf(pathOfSortFile, "%s/%s", pathOfFiles, inFile->d_name);

        int isSortFileHasAccess = access(pathOfSortFile, R_OK);
        if (isSortFileHasAccess == -1) {
            fprintf(stderr, "The sort file does not have read permission %s\n. Exiting...\n", pathOfSortFile);
            return -1;
        }

        struct stat fileStat;
        if (stat(pathOfSortFile, &fileStat) == -1) {
            fprintf(stderr, "Could not get file stat for %s, exiting...", pathOfSortFile);
            return -1;
        }

        int fileInReadOnlyMode = open(pathOfSortFile, O_RDONLY);
        int numOfFilesInReadOnlyMode = 0;
        int* contentFromReadOnlyFile = malloc(fileStat.st_size);
        while (read(fileInReadOnlyMode, contentFromReadOnlyFile, sizeof(int)) != 0) {
            numOfFilesInReadOnlyMode += 1;
        }

        lseek(fileInReadOnlyMode, 0, 0);
        int arrayForFileNumbers[numOfFilesInReadOnlyMode];
        int varToStoreCount = 0;
        while (varToStoreCount < numOfFilesInReadOnlyMode) {
            read(fileInReadOnlyMode, contentFromReadOnlyFile, sizeof(int));
            arrayForFileNumbers[varToStoreCount] = *contentFromReadOnlyFile;
            varToStoreCount += 1;
        }

        close(fileInReadOnlyMode);

        funToSortArray(arrayForFileNumbers, numOfFilesInReadOnlyMode);

        char arrayToStoreSortedFileNames[strlen(inFile->d_name)-2];
        strncpy(arrayToStoreSortedFileNames, &inFile->d_name[2], strlen(inFile->d_name));

        char *ptrTargetFolder = (char*)malloc(strlen(createTargetFolder) + 20);
        sprintf(ptrTargetFolder, "%s%s", createTargetFolder, arrayToStoreSortedFileNames);
        int openFileToWrite = open(ptrTargetFolder, O_RDWR|O_CREAT, 0644);

        for (int j=0; j<numOfFilesInReadOnlyMode; j++) {
            write(openFileToWrite, &arrayForFileNumbers[j], sizeof(int));
        }
        close(openFileToWrite);
        printf("***************Sorting complete************\n");
    }

    if (countOfFiles == 0) {
        fprintf(stderr, "Permission was denied\n. Exiting\n");
        return -1;
    }
    return 0;
}