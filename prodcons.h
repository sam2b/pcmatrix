/*
 *  prodcons header
 *  Function prototypes, data, and constants for producer/consumer module
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

// PRODUCER-CONSUMER put() get() function prototypes

// Data structure to track matrix production / consumption stats
// sumtotal - total of all elements produced or consumed
// multtotal - total number of matrices multipled 
// matrixtotal - total number of matrces produced or consumed
typedef struct prodcons {
  int sumTotal;
  int multTotal;
  int prodTotal;
  int consTotal;
  int matrixTotal;
  int prodSum;
  int consSum;
} ProdConsStats;

ProdConsStats *stats;

// PRODUCER-CONSUMER thread method function prototypes
void *prod_worker(void *arg);
void *cons_worker(void *arg);

// Routines to add and remove matrices from the bounded buffer
void put(Matrix *value);
Matrix* get();

void initProdCons();
void cleanProdCons();
void displayCalculation(Matrix *dotProduct);
void displayBoundedBuffer();
