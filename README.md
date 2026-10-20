# HPC-IntroToMPI

Exercises for the course *Introduction to Message Passing Interface*
(F1_MA_HPC_1-24, University of Luxembourg), run on the AION HPC cluster.

## Structure

```
HPC-IntroToMPI/
├── IntroductionToMPI/   git submodule: course repo (provided libraries)
├── exercise_1/          Graded Exercise 1: d = A·B·c and ‖d‖
└── README.md
```

Each exercise folder has its own `README.md`, `Makefile` and a `setup_libs.sh`
script that copies the provided libraries from the submodule into the
exercise's `include/` folder.

## Getting started

```bash
git clone --recurse-submodules https://github.com/Ali-Noghabi/HPC-IntroToMPI.git
cd HPC-IntroToMPI/exercise_1
./setup_libs.sh
module load toolchain/foss     # on AION
make
```

Already cloned without `--recurse-submodules`?

```bash
git submodule update --init
```

## Exercises

| Exercise | Description | Status |
|----------|-------------|--------|
| [exercise_1](exercise_1/) | MPI matrix–matrix–vector product and norm, scaling on AION | code done, scaling runs on AION to do |