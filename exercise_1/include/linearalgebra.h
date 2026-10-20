// Perform matrix-vector multiplication
void dgemv(int *n, int *m, double **A, double *b, double *c)
{
   for (int i = 0; i < *n; i++)
   {
      c[i] = 0.0;
      for (int j = 0; j < *m; j++)
      {
         c[i] += A[i][j] * b[j];
      }
   }
}

// Compute the dot product of two vectors
double ddot(int *n, double *a, double *b)
{
   double result;
   result = 0.0;
   for (int i = 0; i < *n; i++)
   {
      result += a[i] * b[i];
   }
   return result;
}