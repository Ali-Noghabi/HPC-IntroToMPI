# Exercise 1 – Part 1: x = B·c and ‖x‖

Computes the vector `x = B·c` (B is `m×k`, c has `k` elements) with MPI and
prints its norm plus the timings (initialization, calculation, reduction,
total). B·c can be computed on one rank or split over all ranks.

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
| `seed`        | seed of B (20); c uses `seed+10` (30), A will use `seed-10` (10) |
| `bc_parallel` | `1` = B·c split over all ranks, `0` = B·c only on rank 0        |

Quick test, both modes must print the same norm:

```bash
mpirun -n 4 ./bc_norm 1000 50 100 20 1
mpirun -n 4 ./bc_norm 1000 50 100 20 0
```

Exercise sizes on AION (inside a job or an interactive allocation):

```bash
for i in 1 2 4 8 16 32 64 128; do
    srun -n $i ./bc_norm 128000 500 1000 20 1 > out_$i
done
```

## Output

Rank 0 prints the norm of x and the **maximum over all ranks** of the
initialization, calculation, reduction and total time (the slowest rank
decides the real run time).