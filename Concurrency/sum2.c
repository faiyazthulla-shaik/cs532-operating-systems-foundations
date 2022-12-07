/*
 * CS 532: sum.c: repeatedly sum numbers using multiple threads
 */

/*
 * sum: compute the value of numthreads * (sum of all integers from 0 to Round) 
 * for every value of Round from 1 to numrounds
 * the program also uses a closed-form solution to check the result after each round. if it finds
 * an error, then it exits.
 *
 * build like this ---> gcc -g -Wall -Werror sum.c -o sum -lpthread
 * 
 * the code as-is has no synchronization, and so it exits with an error nearly every single time.
 * your assignment is to create two "fixed" versions of this code.
 * 1. a version that calculates the correct answer but continues to inefficiently update the 
 *    the "Total" global variable within the inner loop of each thread. this version should be 
 *    called "sum_fix1.c"
 * 2. a more-efficient version in which each thread computes its own sum locally before updating 
 *    the global "Total" after computing its local sum. this version should be called "sum_fix2.c"
 *
 * Instead of two different programs you may create a single "sum_fix.c" that satisfies 
 * both (1) and (2), but then you need to 
 * provide a way for the user to toggle between (1) and (2) from the command line.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXTHREADS 10

volatile unsigned long long Round = 0ULL;
volatile unsigned long long Total = 0ULL;

pthread_mutex_t total_mutex = PTHREAD_MUTEX_INITIALIZER;

void *Summation(void *tidptr)
{
  unsigned long long ids = * ((unsigned long long*) tidptr);
  for (unsigned long long dex = 0ULL; dex <= ids; dex++) {
    pthread_mutex_lock(&total_mutex);
    Total += dex;
    pthread_mutex_unlock(&total_mutex);
  }

  return NULL;
}

void doOneRound(unsigned long long thisRound, int numthreads)
{
  int tids[MAXTHREADS] = {0,1,2,3,4,5,6,7,8,9};
  pthread_t t[numthreads];
  int rc;
 
  /* start all of the threads */
  for (int i = 0; i < numthreads; i++) {
    rc = pthread_create(&t[i], NULL, Summation, (void *) &thisRound);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  /* wait for threads to complete */
  for (int i = 0; i < numthreads; i++) {
    rc = pthread_join(t[i], NULL);
    if (rc != 0) {
      fprintf(stderr, "ERROR joining with thread %d (error==%d)\n", tids[i], rc);
      exit(-1);
    }
  } 
}

void checkResult(unsigned long long thisRound, int numthreads)
{
  /* 
   * note: credit for this closed-form solution goes to Johann Carl Friedrich Gauss.
   * it will not work for very large values of thisRound
   * because the multiplication will cause it to overflow. but it is good enough
   * for our purposes.
   */
  unsigned long long calc = (thisRound * (thisRound + 1ULL)) / 2ULL;
  calc *= (unsigned long long) numthreads;

  if (Total != calc) {
    printf("PARENT: ERROR! Round %llu total should have been %llu but was %llu\n", thisRound, calc, Total);
    exit(-1);
  } 
}


int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s <nthreads> <max>\n", argv[0]);
    exit(-1);
  }

  int numthreads = atoi(argv[1]);
  if ((numthreads < 1) || (numthreads > MAXTHREADS)) {
    fprintf(stderr, "ERROR: numthreads must be >= 1 and <= %d\n", MAXTHREADS);
    exit(-1);
  }

  unsigned long long numrounds = strtoull(argv[2], NULL, 0);
  if (numrounds <= 0ULL) {
    fprintf(stderr, "ERROR: number of rounds must be a positive unsigned long long (not '%s')\n", argv[2]);
    exit(-1);
  }

  printf("PARENT: input %d threads %llu rounds\n", numthreads, numrounds);
 
  for (Round = 1ULL; Round < numrounds; Round++) {
    Total = 0ULL;

    doOneRound(Round, numthreads);
    checkResult(Round, numthreads);
  }

  printf("PARENT: SUCCESS! exiting after final Round %llu (Total: %llu)\n", numrounds-1, Total);

  return(0);
}