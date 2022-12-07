#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

int numberOfFiles;
int numberOfIntegers;
char *locationOfDirectory;
int numberOfFiles;
int numberOfThreads;

void *funcationToCreatefiles(void *idOfThread){
    unsigned int timeValue = time(0);
    char nameofTheFile[5000]; 
    int thread_Id = *((int*) idOfThread);
    int f_Id;

    for(f_Id=thread_Id; f_Id<numberOfFiles; f_Id +=numberOfThreads){
        sprintf(nameofTheFile, "%s/unsorted_%d.bin", locationOfDirectory, f_Id);
        int isFileOpened = open(nameofTheFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if(isFileOpened < 0){
            char *errorMessage= strerror(errno);
            fprintf(stderr, "File opening was failed(%s)\n", errorMessage);
            exit(-1);
        }
        for (int j=0; j<numberOfIntegers; j++){
            int randomValue = rand_r(&timeValue);
            int isWrittenSuccessful=  write(isFileOpened, &randomValue, sizeof(int));
            if(isWrittenSuccessful < 0){
                fprintf(stderr, "\nWriting was failed...\n");
                exit(-1);
            }
        }
        close(isFileOpened);
    }
    printf("Create funcation ran successfully\n"); 
    return 0;   
}

int main(int argc, char *argv[]){
    if(argc != 5){
        fprintf(stderr, "Please enter all the four arguments!!");
        exit(-1);
    }
    locationOfDirectory = argv[1];
    numberOfFiles = atoi(argv[2]);
    numberOfIntegers = atoi(argv[3]);
    numberOfThreads = atoi(argv[4]);    
    pthread_t threads[numberOfThreads];
    int t_args[numberOfThreads];
    int isThreadCreated;

    for(int i=0; i<numberOfThreads; i++){
        t_args[i] = i;
        isThreadCreated = pthread_create(&threads[i], NULL, funcationToCreatefiles, (void *) &t_args[i]);
        if(isThreadCreated){
            fprintf(stderr, "Error while creating the thread!!\n Return code from pthread_create() is %d\n", isThreadCreated);
            exit(-1);
        }else{
            printf("Thread created successfully!!\n");
        }
    }
    for (int i=0; i<numberOfThreads; i++){
        isThreadCreated = pthread_join(threads[i],NULL);
    }
    return 0;
}