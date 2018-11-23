/*
 * Sam Brendel, 11/23/2018
 * TCSS422 Operating Systems, Professor Wes Lloyd
 * Disclaimer: Tested on Ubuntu 18.04.1 LTS, kernel 4.15.0-34-generic
 */

/*
 *  prodcons module
 *  Producer Consumer module
 * 
 *  Implements routines for the producer consumer module based on 
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"
#include <assert.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// A shared sub buffer for the consumer since it can only get a single matrix per loop iteration.
Matrix* multiplySet[2];

// Define Locks and Condition variables here
int ready = 0; // The state variable.

// Producer consumer data structures.
// An array of pointers to data on the heap.
Matrix *bigmatrix[MAX];

// Bounded buffer put() get()
void put(Matrix * value) {
  int index = get_cnt(counterProd) % MAX;
  bigmatrix[index] = value;
  increment_cnt(counterProd);
  stats->prodTotal++;
  //displayBoundedBuffer(); // DEBUGGING.
}

void displayBoundedBuffer() {
  //printf("Bounded buffer: {");
  int i = 0;
  printf("------BOUNDED BUFFER-------------------------------------------------------------------------------------------------------------------------------------------\n");
  for (; i < MAX - 1; i++)
  {
    //printf("i=%p, ", bigmatrix[i]);
    if(bigmatrix[i] != NULL) {
      DisplayMatrix(bigmatrix[i], stdout);
    }
  }
  //printf("i=%p}\n", bigmatrix[i]);
  printf("-------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

Matrix* get() {
  int index = get_cnt(counterCons) % MAX;
  Matrix* retrieved;
  retrieved = bigmatrix[index];
  if (retrieved != NULL) {
    stats->consSum += SumMatrix(retrieved);
    increment_cnt(counterCons);
    stats->consTotal++;
  }
  bigmatrix[index] = NULL; // "free up" the slot in the array.
  return retrieved;
}

// Matrix PRODUCER worker thread
void* prod_worker(void* arg) {
    int* prodLoops = (int*)&arg;
    #if OUTPUT
        printf("In prod_worker...\n");
        printf("   prodLoops=%d\n", *prodLoops);
    #endif
        for (int i = 0; i < *prodLoops; i++) {
          pthread_mutex_lock(&mutex);
          // Only fill the element if there is a vacancy.
          Matrix *theMatrix = GenMatrixRandom();
          stats->prodSum += SumMatrix(theMatrix);
          put(theMatrix);
          ready = 1;
          pthread_cond_signal(&cond);
          while (ready == 1) {
            pthread_cond_wait(&cond, &mutex);
          }
          pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Matrix CONSUMER worker thread
void* cons_worker(void* arg) {
  int* consLoops = (int*)&arg;
#if OUTPUT
  printf("In cons_worker...\n");
  printf("   consLoops=%d\n", *consLoops);
#endif
  int i;
  for (i = 0; i < *consLoops; i++) {
    pthread_mutex_lock(&mutex);
    while (ready == 0) {
      pthread_cond_wait(&cond, &mutex);
    }

    if (multiplySet[0] == NULL) {
      multiplySet[0] = get();
    }
    else if (multiplySet[1] == NULL) {
      multiplySet[1] = get();
    }
    ready = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    if (multiplySet[0] != NULL && multiplySet[1] != NULL) {
      Matrix *dotProduct = MatrixMultiply(multiplySet[0], multiplySet[1]); // Possible that dotProduct will be null because of incompatible matrices.

      // If the two matrices are compatible.
      if (dotProduct != NULL) {
        displayCalculation(dotProduct);
        FreeMatrix(dotProduct);
      }

      //Clean up.
      FreeMatrix(multiplySet[0]);
      multiplySet[0] = NULL;
      FreeMatrix(multiplySet[1]);
      multiplySet[1] = NULL;
    }
  } // end for loop.
  return NULL;
}

void displayCalculation(Matrix* dotProduct) {
  DisplayMatrix(multiplySet[0], stdout);
  printf("    X\n");
  DisplayMatrix(multiplySet[1], stdout);
  printf("    =\n");
  DisplayMatrix(dotProduct, stdout);
  printf("\n");
}

void initProdCons() {
  stats = (ProdConsStats*)malloc(sizeof(ProdConsStats));
  stats->sumTotal = 0;
  stats->multTotal = 0;
  stats->matrixTotal = 0;
  stats->prodTotal = 0;
  stats->consTotal = 0;
  stats->prodSum = 0;
  stats->consSum = 0;
  for (int i = 0; i < MAX; i++) {
    bigmatrix[i] = NULL;
  }
}

void cleanProdCons() {
  free(stats);
  for (int i = 0; i < MAX; i++) {
    if (bigmatrix[i] != NULL) {
      FreeMatrix(bigmatrix[i]);
      bigmatrix[i] = NULL;
    }
  }
}