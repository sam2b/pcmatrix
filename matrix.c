/*
 *  Matrix routines
 *  Supports generation of random R x C matrices
 *  And operations on them
 * 
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

#include "matrix.h"
#include "prodcons.h"
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// MATRIX ROUTINES
Matrix* AllocMatrix(int r, int c)
{
  Matrix * mat;
  mat = (Matrix *) malloc(sizeof(Matrix)); 
  int ** a;
  int i;
  //a = NULL;
  //while (a == NULL)
  //{
    a = (int**) malloc(sizeof(int *) * r);
    if (a == NULL) {
      printf("   a=malloc() failed\n");
    }
  //}
  assert(a != NULL); //assert(a != 0);
  for (i = 0; i < r; i++)
  {
    a[i] = NULL;
    //while (a[i] == NULL)
    //{
      a[i] = (int *) malloc(c * sizeof(int));
      if (a[i] == NULL) {
        printf("   a[i]=malloc() failed\n");
      }
      //}
      assert(a[i] != NULL); //assert(a[i] != 0);
  }
  mat->m=a;
  mat->rows=r;
  mat->cols=c;
  //printf("   AllocMatrix: mat->rows=%d  mat->cols=%d\n", mat->rows, mat->cols); // DEBUGGING
  return mat;
}

void FreeMatrix(Matrix* mat)
{
  int r = mat->rows;
  //int c = mat->cols;
  int **a = mat->m;
  int i;
  for (i=0; i<r; i++)
  {
    free(a[i]);
  }
  free(a);
  free(mat);
}

// Fills the matrix with values.
void GenMatrix(Matrix* mat)
{
  int height = mat->rows;
  int width = mat->cols;
  int ** a = mat->m;
  int i, j;
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      int* mm = a[i];
      //matrix[i * width + j] = rand() % 10;
      mm[j] = 1 + rand() % 10;
      //mm[j] = 1;
#if OUTPUT
      printf("matrix[%d][%d]=%d \n",i,j,mm[j]);
#endif
    }
  }
  stats->matrixTotal++;
}

// Generates a matrix with up to 4 rows and up to 4 columns, and allocates and fills with values.
Matrix* GenMatrixRandom()
{
  int row = 1 + rand() % 4;               //1 + timeInMicroseconds() % 4;
  int col = 1 + rand() % 4;               //1 + timeInMicroseconds() % 4;
  Matrix * mat = AllocMatrix(row, col);
  GenMatrix(mat);
  //printf("   GenMatrixRandom: mat->rows=%d  mat->cols=%d\n", mat->rows, mat->cols); // DEBUGGING
  return mat;
}

// Mitigates the perplexing issue of repeat random numbers with rand().  This solves the issue.
int timeInMicroseconds() {
  // #include <sys/time.h>
  struct timeval time;
  gettimeofday(&time, NULL);
  int val = (int)time.tv_usec; // Time In Microseconds.
  return val;
}

Matrix* GenMatrixBySize(int row, int col) {
  printf("Generate random matrix (RxC) = (%dx%d)\n",row,col);
  Matrix * mat = AllocMatrix(row, col);
  GenMatrix(mat);
  return mat; 
}

Matrix* MatrixMultiply(Matrix* m1, Matrix* m2)
{
  if ((m1==NULL) || (m2==NULL)) {
    printf("m1=%p  m2=%p!\n",m1,m2);
    return NULL; // Not possible to multiply. Sam put this line here.
  }
  int sum=0;
  if (m1->cols != m2->rows)
  {
    //printf("   m1->cols=%d != m2->rows=%d\n", m1->cols, m2->rows); // DEBUGGING
    return NULL; // Not possible to multiply.
  }
  printf("MULTIPLY (%d x %d) BY (%d x %d):\n",m1->rows,m1->cols,m2->rows,m2->cols);
  Matrix* newmat = AllocMatrix(m1->rows, m2->cols);
  int** nm = newmat->m;
  int** ma1 = m1->m;
  int** ma2 = m2->m;
  for (int c=0;c<newmat->rows;c++)
  {
    for (int d=0;d<newmat->cols;d++)
    {
      for (int k=0;k<m2->rows;k++)
      {
        sum = sum + ma1[c][k]*ma2[k][d]; 
      }
      nm[c][d] = sum;
      sum=0;
    }
  }
  stats->multTotal++;
  //printf("   Multiply success. multTotal=%d\n", stats->multTotal);
  return newmat;
}

void DisplayMatrix(Matrix* mat, FILE* stream)
{
  if ((mat == NULL) || (mat->m == NULL))
  { 
    printf("DisplayMatrix: EMPTY matrix\n");
    return;
  }
  int** matrix = mat->m;
  int height = mat->rows;
  int width = mat->cols; 
  int y=0;
  int i, j;
  for (i=0; i<height; i++)
  {
    int* mm = matrix[i];
    fprintf(stream, "|");
    for (j=0; j<width; j++)
    {
      y=mm[j];
      if (j==0)
        fprintf(stream, "%3d",y);
      else
        fprintf(stream, " %3d",y);
    }
    fprintf(stream, "|\n");
  }
}


int AvgElement(Matrix* mat) // int ** matrix, const int height, const int width)
{
  int** a = mat->m;
  int height = mat->rows;
  int width = mat->cols; 
  int x=0;
  int y=0;
  int ele=0;
  int i, j;
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
    {
      int *mm = a[i];
      y=mm[j];  
      x=x+y;
      ele++;
#if OUTPUT
      printf("[%d][%d]--%d x=%d ele=%d\n",i,j,mm[j],x,ele);
#endif
    }
  printf("x=%d ele=%d\n",x, ele);
  return x / ele;
}

int SumMatrix(Matrix* mat) {
   int** a = mat->m;
   int height = mat->rows;
   int width = mat->cols;
   int i =0;
   int j =0;
   int y =0;
   int total = 0;
   for (i = 0; i < height; i++) 
   {
      for (j = 0; j < width; j++)
      {
        int* mm = a[i];
        y=mm[j];
        total = total+y;
      }
   }
   stats->sumTotal += total;
   return total;
}



 
