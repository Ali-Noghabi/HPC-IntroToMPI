#include "arrays.h"
#include "linearalgebra.h"
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  int num_of_ranks;
  MPI_Comm_size(MPI_COMM_WORLD, &num_of_ranks);
  int mpi_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  // Measure initial system time
  double t0 = MPI_Wtime();

  // Check number of arguments
  if (argc != 6) {
    if (mpi_rank == 0) {
      printf("Usage: %s <n> <m> <k> <seed> <bc_parallel>\n"
             "  matrix A (n*m), matrix B (m*k), vector c (k)\n"
             "  seed        : random seed of B\n"
             "  bc_parallel : 1 = compute x = B*c split over all ranks\n"
             "                0 = compute x = B*c only on rank 0\n",
             argv[0]);
    }
    MPI_Finalize();
    return 1;
  }

  // Convert string arguments to integers using atoi()
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);
  int k = atoi(argv[3]);
  int seed = atoi(argv[4]);
  int bc_parallel = atoi(argv[5]);

  // The sizes are broadcasted to all ranks
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Compute the number of rows that must be distributed on each rank.
  int num_of_matrix_elements_per_rank = (int)m / num_of_ranks;
  int num_of_residual_elements = (int)m % num_of_ranks;

  // Number of rows of B that this rank has to multiply with c.
  // In serial mode rank 0 does everything and the other ranks do nothing
  int rows_local;
  if (bc_parallel == 1) {
    rows_local = num_of_matrix_elements_per_rank +
                 (mpi_rank < num_of_residual_elements ? 1 : 0);
  } else {
    rows_local = (mpi_rank == 0) ? m : 0;
  }

  double *vector_c = NULL;
  double **matrix_b = NULL;
  double *vector_x = NULL;

  // Initialize "local" and "initial" matrices. B and c are initialized by
  // rank 0, so rank 0 always allocates the full B
  if (mpi_rank == 0) {
    vector_c = allocate_1d_double(&k);
    matrix_b = allocate_2d_packed_double(&m, &k);
    init_random_matrix(seed, &m, &k, matrix_b);
    init_random_vector(seed + 10, &k, vector_c);
  } else if (bc_parallel == 1) {
    vector_c = allocate_1d_double(&k);
    if (rows_local > 0) {
      matrix_b = allocate_2d_packed_double(&rows_local, &k);
    }
  }

  if (bc_parallel == 1) {
    // Bcast the whole vector c to all ranks
    MPI_Bcast(&vector_c[0], k, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Distribute matrix B to all ranks.
    if (mpi_rank == 0) {
      for (int i = 1; i < num_of_ranks; i++) {
        int rows_i = rows_of_rank(i, m, num_of_ranks);
        if (rows_i > 0) {
          MPI_Send(&matrix_b[first_row_of_rank(i, m, num_of_ranks)][0],
                   rows_i * k, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
        }
      }
    } else if (rows_local > 0) {
      MPI_Recv(&matrix_b[0][0], rows_local * k, MPI_DOUBLE, 0, mpi_rank,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }

  // Allocate "local" output vector (a piece of x)
  if (rows_local > 0) {
    vector_x = allocate_1d_double(&rows_local);
  }

  // Measure time before mathematical operations
  double t1 = MPI_Wtime();

  // Compute "local" dgemv operation x = B*c and the "local" dot product of x.
  // Compute "local dot product"
  double norm = 0.0;
  if (rows_local > 0) {
    dgemv(&rows_local, &k, matrix_b, vector_c, vector_x);
    norm = ddot(&rows_local, vector_x, vector_x);
  }

  // Measure time after mathematical operations
  double t2 = MPI_Wtime();

  // Reduce the values of "norm", they contain the local dot products
  if (mpi_rank == 0) {
    MPI_Reduce(MPI_IN_PLACE, &norm, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  } else {
    MPI_Reduce(&norm, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  }

  // Rank 0 computes the norm (sqrt of the dot product)
  if (mpi_rank == 0) {
    norm = sqrt(norm);
    printf("Norm of the vector x = B*c is : %4.10f\n", norm);
  }

  // Final time
  double t3 = MPI_Wtime();
  
  // Compute time differences and print them.
  // The slowest rank decides the real time, so we take the max over the ranks
  double times[4];
  double times_max[4];
  times[0] = t1 - t0; // Time taken for initialization
  times[1] = t2 - t1; // Time taken for calculation
  times[2] = t3 - t2; // Time taken for reduction
  times[3] = t3 - t0; // Total time
  MPI_Reduce(times, times_max, 4, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  if (mpi_rank == 0) {
    printf("B*c mode (1 = parallel, 0 = rank 0)  : %d\n", bc_parallel);
    printf("Number of ranks                      : %d\n", num_of_ranks);
    printf("Max initialization time over ranks   : %f seconds\n", times_max[0]);
    printf("Max calculation time over ranks      : %f seconds\n", times_max[1]);
    printf("Max reduction time over ranks        : %f seconds\n", times_max[2]);
    printf("Max total time over ranks            : %f seconds\n", times_max[3]);
  }

  MPI_Finalize();
  return 0;
}