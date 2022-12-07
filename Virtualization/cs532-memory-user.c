#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Wrong usage!!, try with mb to allocate.\n");
        exit(1);
    }
	
	//Requesting the memory and converting into bytes.
    long long int reqMem = (long long int) atoi(argv[1]);
    long long int reqMemInBytes = reqMem * 1024 * 1024;
	
	//Allocating the memory
	int *ptr = malloc(reqMemInBytes);
	
	//Checking whether the pointer is null or not.
    if (ptr == NULL) {
		//Printing on the console...
        fprintf(stderr, "WARNING:: Allocation of Memory was failed!!\n");
        exit(1);
    }

    long long int numOfInts = reqMemInBytes / sizeof(int);
	//Printing the requested memory on the console...
    printf("INFO:: Requested memory in bytes:: %lld\n", reqMemInBytes);

	//Variables declaration
    long long int initial = 0;
    long long int loop_iterator = 0;
	
	for(initial = 0; initial<numOfInts; ++initial){
		ptr[initial] = initial+1;
	}
	
	//Writting the infinite loop.
    while (1) {
        for(initial = 0; initial<numOfInts; ++initial){
			ptr[initial] = initial+1;
		}
    }
    return 0;
}