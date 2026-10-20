# Exercise 1: d = A·B·c and ‖d‖

Computes `d = A·(B·c)` with MPI (A is `n×m`, B is `m×k`, c has `k` elements)
and prints the norm of d plus the timings (initialization, calculation,
reduction, total).

1. `x = B·c` is computed either on rank 0 only or split over all ranks
   (`bc_parallel` flag). Every rank then gets the full x with `MPI_Allreduce`
   (each rank writes its piece into a zero vector, so the sum is the full x).
2. `d = A·x` is always parallel: the rows of A are split over all ranks, each
   rank computes its piece of d and its local dot product.
3. The local dot products are summed with `MPI_Reduce` on rank 0, which prints
   the square root (the norm of d).

Computing `A·(B·c)` instead of `(A·B)·c` gives the same result with about 1000×
fewer operations.

```
exercise_1/
├── include/        course libraries (copied here by setup_libs.sh)
├── src/main.c      source code
├── Makefile
├── setup_libs.sh   copies the libraries into include/
└── README.md
```

## Dependencies

- MPI and a C compiler (on AION: `module load toolchain/foss`)
- Course libraries `arrays.h`, `linearalgebra.h`, `mt19937ar.h` from
  <https://github.com/mbarborini/IntroductionToMPI/tree/main/Examples/Libraries>.
  They are included in this repo as the git submodule `IntroductionToMPI`
  (in the parent directory).

## Build

```bash
# 0. clone with the submodule (skip if already done)
git clone --recurse-submodules https://github.com/Ali-Noghabi/HPC-IntroToMPI.git
cd HPC-IntroToMPI/exercise_1

# 1. copy the libraries into include/
./setup_libs.sh

# 2. compile (on AION load the toolchain first)
module load toolchain/foss
make
```

`make clean` removes `obj/` and the executable. If the repo was cloned without
`--recurse-submodules`, `setup_libs.sh` downloads the submodule by itself.

## Run

```
mpirun -n <P> ./bc_norm <n> <m> <k> <seed> <bc_parallel>
srun   -n <P> ./bc_norm <n> <m> <k> <seed> <bc_parallel>     # on AION
```

| argument      | meaning                                                         |
|---------------|-----------------------------------------------------------------|
| `n`           | rows of A (128000)                                              |
| `m`           | columns of A = rows of B (500)                                  |
| `k`           | columns of B = elements of c (1000)                             |
| `seed`        | seed of B (20); c uses `seed+10` (30), A uses `seed-10` (10)      |
| `bc_parallel` | `1` = B·c split over all ranks, `0` = B·c only on rank 0        |

Quick test, all runs must print the same norm (427.0457708965 for these sizes):

```bash
mpirun -n 4 ./bc_norm 1000 50 100 20 1
mpirun -n 4 ./bc_norm 1000 50 100 20 0
mpirun -n 1 ./bc_norm 1000 50 100 20 1
```

Exercise sizes on AION (inside a job or an interactive allocation):

```bash
for i in 1 2 4 8 16 32 64 128; do
    srun -n $i ./bc_norm 128000 500 1000 20 1 > out_$i
done
```

## Output

Rank 0 prints the norm of d and the **maximum over all ranks** of the
initialization, calculation, reduction and total time (the slowest rank
decides the real run time).

- *initialization*: random numbers (rank 0) and distribution of B, c and A
- *calculation*: the two `dgemv` calls and the local dot product
- *reduction*: `MPI_Allreduce` of x and `MPI_Reduce` of the norm

Note: rank 0 holds the full A (about 512 MB for the exercise sizes).