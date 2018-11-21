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
Matrix* multiplySet[2];
//struct matrix multiplySet[2];

// Define Locks and Condition variables here
int ready = 0; // The state variable.

// Producer consumer data structures
Matrix *bigmatrix[MAX];

// Bounded buffer put() get()
int put(Matrix * value)
{
  //printf("put()\n");  // DEBUGGING
  int result = 1;
  int index = get_cnt(counterProd) % MAX;
  //printf("   put index=%d\n", index); // DEBUGGING
  // if (value == NULL)
  // {
  //   printf("put failed\n");
  //   result = 0; // false, did not put.
  // }
  // else {
    bigmatrix[index] = value;
    //printf("   put bigmatrix[%d] = %x\n", index, bigmatrix[index]);  // DEBUGGING
    //assert(bigmatrix[index] != NULL);
    increment_cnt(counterProd);
    stats->prodTotal++;
    result = 1; // true, successful put.
  //}
  return result;
}

Matrix* get() 
{
  int index = get_cnt(counterCons) % MAX;
  Matrix* retrieved;
  retrieved = bigmatrix[index];
  //printf("   get bigmatrix[%d] = %p\n", index, bigmatrix[index]);       // DEBUGGING
  //printf("   bigmatrix[%d]->rows=%d\n", index, bigmatrix[index]->rows); // DEBUGGING
  //printf("   bigmatrix[%d]->cols=%d\n", index, bigmatrix[index]->cols); // DEBUGGING
  //assert(retrieved != NULL);
  if (retrieved != NULL)
  {
    stats->consSum += SumMatrix(retrieved);
    //FreeMatrix(bigmatrix[index]);
    //FreeMatrix(retrieved); // Removed 5:03pm 11/20
    increment_cnt(counterCons);
    stats->consTotal++;
  }
  //printf("Returning from get()\n"); // DEBUGGING
  //DisplayMatrix(retrieved, stdout); // DEBUGGING
  return retrieved;
}

// Matrix PRODUCER worker thread
void* prod_worker(void* arg)
{
    int* prodLoops = (int*)&arg;
    #if OUTPUT
        printf("In prod_worker...\n");
        printf("   prodLoops=%d\n", *prodLoops);
    #endif
        for (int i = 0; i < *prodLoops; i++)
        {
          pthread_mutex_lock(&mutex);

          // Only fill the element if there is a vacancy.
          Matrix *theMatrix = GenMatrixRandom();
          stats->prodSum += SumMatrix(theMatrix);
          if (put(theMatrix))
          {
            ready = 1;
            pthread_cond_signal(&cond);
          } else {
            printf("--------------PUT FAILED---------------\n");
          }

          //printf("   prod_worker i=%d\n", i); // DEBUGGING

          while (ready == 1)
          {
            //printf("   prod while  ");
            pthread_cond_wait(&cond, &mutex);
          }
          pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Matrix CONSUMER worker thread
void* cons_worker(void* arg) 
{
  int* consLoops = (int*)&arg;
#if OUTPUT
  printf("In cons_worker...\n");
  printf("   consLoops=%d\n", *consLoops);
#endif
  int i;
  for (i = 0; i < *consLoops; i++)
  {
    //printf("   con_worker i=%d\n", i);  // DEBUGGING
    pthread_mutex_lock(&mutex);
    while (ready == 0) {
      //printf("   cons while   ");
      pthread_cond_wait(&cond, &mutex);
    }

    //printf("   multiplySet[0]=%p\n", (Matrix *)&multiplySet[0]); // DEBUGGING
    //printf("   multiplySet[1]=%p\n", (Matrix *)&multiplySet[1]); // DEBUGGING
    if (multiplySet[0] == NULL)
    {
      multiplySet[0] = get();
      //printf("   *multiplySet[0] = get()\n"); // DEBUGGING
    }
    else if (multiplySet[1] == NULL)
    {
      multiplySet[1] = get();
      //printf("   *multiplySet[1] = get()\n"); // DEBUGGING
    }
    //printf("Done.\n");
    ready = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    if (multiplySet[0] != NULL && multiplySet[1] != NULL)
    {
      Matrix *m1 = multiplySet[0]; //BUG how do I get/set the array element as a Matrix* ?
      Matrix *m2 = multiplySet[1];
      Matrix* dotProduct = MatrixMultiply(m1, m2);
      if (dotProduct == NULL) {
        //printf("Did not multiply.\n");
      }
      multiplySet[0] = NULL;
      multiplySet[1] = NULL;
    }
  } // end for loop.
  return NULL;
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
}

void cleanProdCons() {
  free(stats);
  int i;
  for (i = 0; i < MAX; i++)
  {
    if (bigmatrix[i] != NULL) {
      FreeMatrix(bigmatrix[i]);
    }
  }
}