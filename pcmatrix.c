/*
 * Sam Brendel, 11/23/2018, 12/6/2018
 * TCSS422 Operating Systems, Professor Wes Lloyd
 * Disclaimer: Tested on Ubuntu 18.04.1 LTS, kerne l4.15.0-42-generic
 */

/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 * 
 *  Producer consumer bounded buffer program to produce random matrices in parallel
 *  and consume them while searching for valid pairs for matrix multiplication.
 *  Matrix multiplication requires the first matrix column count equal the 
 *  second matrix row count.  
 *
 *  A matrix is consumed from the bounded buffer.  Then matrices are consumed
 *  from the bounded buffer, one at a time, until an eligible matrix for multiplication
 *  is found.
 *
 *  Totals are tracked using the ProdConsStats Struct for:
 *  - the total number of matrices multiplied (multtotal from consumer threads)
 *  - the total number of matrices produced (matrixtotal from producer threads)
 *  - the total number of matrices consumed (matrixtotal from consumer threads)
 *  - the sum of all elements of all matrices produced and consumed (sumtotal from producer and consumer threads)
 *
 *  Correct programs will produce and consume the same number of matrices, and
 *  report the same sum for all matrix elements produced and consumed. 
 *
 *  For matrix multiplication only ~25% may be e
 *  and consume matrices.  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

int main (int argc, char * argv[]) {
  time_t t;
  int numw = NUMWORK;

  // Seed the random number generator with the system time
  srand((unsigned) time(&t));

  //
  // Demonstration code to show the use of matrix routines
  //
  // DELETE THIS CODE ON ASSIGNMENT 2 SUBMISSION
  // ----------------------------------------------------------
  // printf("MATRIX MULTIPLICATION DEMO:\n\n");
  // Matrix *m1, *m2, *m3;
  // for (int i=0;i<12;i++)
  // {
  //   m1 = GenMatrixRandom();
  //   m2 = GenMatrixRandom();
  //   m3 = MatrixMultiply(m1, m2);
  //   if (m3 != NULL)
  //   {
  //     DisplayMatrix(m1,stdout);
  //     printf("    X\n");
  //     DisplayMatrix(m2,stdout);
  //     printf("    =\n");
  //     DisplayMatrix(m3,stdout);
  //     printf("\n");
  //     FreeMatrix(m3);
  //     FreeMatrix(m2);
  //     FreeMatrix(m1);
  //     m1=NULL;
  //     m2=NULL;
  //     m3=NULL;
  //   }
  // }
  // return 0;
  // ----------------------------------------------------------

  //printf("Producing %d %dx%d matrices.\n",LOOPS, ROW, COL);
  printf("Producing %d matrices of random dimensions.\n", LOOPS);
  printf("Using a shared buffer of size=%d\n", MAX);
  printf("With %d producer and consumer thread(s).\n",numw);
  printf("\n");

  // Initialize
  int prs = 0;
  int cos = 0;
  int prodtot = 0;
  int constot = 0;
  int consmul = 0;
  counterProd = (counter_t *) malloc(sizeof(counter_t));
  counterCons = (counter_t *) malloc(sizeof(counter_t));
  counters_t *counters = (counters_t *) malloc(sizeof(counters_t));
  counters->prod = counterProd;
  counters->cons = counterCons;
  init_cnt(counterProd);
  init_cnt(counterCons);
  initProdCons();


  //new code.
  pthread_t pr;
  pthread_t co;
  pthread_create(&pr, NULL, prod_worker, LOOPS);
  pthread_create(&co, NULL, cons_worker, LOOPS);

  pthread_join(pr, NULL);
  pthread_join(co, NULL);

  /* //old code.
  pthread_t *pr[NUMWORK];
  pthread_t *co[NUMWORK];
  
   Start concurrent threads!
  for(int i = 0; i < NUMWORK; i++) {
    pthread_create(pr[i], NULL, prod_worker, LOOPS);
    pthread_create(co[i], NULL, cons_worker, LOOPS);
  }
  
  //End of work, prepare to exit this process.
  for(int j = 0; j < NUMWORK; j++) {
    pthread_join(*pr[j], NULL);
    pthread_join(*co[j], NULL);
  }*/

  // consume ProdConsStats from producer and consumer threads
  // add up total matrix stats in prs, cos, prodtot, constot, consmul
  prs = stats->prodSum;
  cos = stats->consSum;
  prodtot = stats->prodTotal;
  constot = stats->consTotal;
  consmul = stats->multTotal;
  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n", prs, cos);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n", prodtot, constot, consmul);

  //Clean up.
  cleanup();
  free(counters);
  printf("End of program.\n\n");
  return 0;
}

void cleanup() {
  cleanProdCons(); // BUG causing crash on small loop count. i.e. 10.
  free(counterProd);
  free(counterCons);
}
