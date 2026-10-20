#!/bin/bash -l
#SBATCH -J bc_norm
#SBATCH -N 1
#SBATCH --ntasks-per-node 128   # MPI processes per node
#SBATCH --cpus-per-task 1       # Cores per MPI process
#SBATCH --time=0-01:00:00
#SBATCH -p batch
#SBATCH --reservation=mpi_course
#SBATCH --exclusive

# Scaling benchmark: runs ./bc_norm for several n, modes and numbers of ranks,
# keeps every raw output in outputs/raw/ and one line per run in
# outputs/results.csv.
#
#   sbatch submit.sh                      # normal use on AION
#   N_LIST="128000" REPS=1 bash submit.sh # inside an interactive allocation
#
# Settings can be changed from the environment:
#   N_LIST   rows of A          (default: 128000 640000 1280000)
#   P_LIST   numbers of ranks   (default: 1 2 4 8 16 32 64 128)
#   BC_LIST  bc_parallel flags  (default: 1 0)
#   REPS     repetitions        (default: 3)
#   LAUNCHER how to start MPI   (default: "srun -n", use "mpirun -n" locally)

type module &>/dev/null && module load toolchain/foss

EXE=${EXE:-./bc_norm}
M=${M:-500}
K=${K:-1000}
SEED=${SEED:-20}
N_LIST=${N_LIST:-"128000 640000 1280000"}
P_LIST=${P_LIST:-"1 2 4 8 16 32 64 128"}
BC_LIST=${BC_LIST:-"1 0"}
REPS=${REPS:-3}
LAUNCHER=${LAUNCHER:-"srun -n"}

OUTDIR=${OUTDIR:-outputs}
RAW=$OUTDIR/raw
CSV=$OUTDIR/results.csv
mkdir -p "$RAW"

# Header (only if the file is new, delete results.csv to start from scratch)
if [ ! -f "$CSV" ]; then
    echo "n,m,k,seed,bc_parallel,ranks,rep,norm,init_s,calc_s,reduce_s,total_s" > "$CSV"
fi

for n in $N_LIST; do
  for bc in $BC_LIST; do
    for p in $P_LIST; do
      for rep in $(seq 1 "$REPS"); do
        log="$RAW/n${n}_bc${bc}_p${p}_r${rep}.out"
        echo "[$(date +%H:%M:%S)] n=$n bc_parallel=$bc ranks=$p rep=$rep"

        $LAUNCHER "$p" "$EXE" "$n" "$M" "$K" "$SEED" "$bc" > "$log" 2>&1

        # Take the numbers out of the output of rank 0
        norm=$(awk '/^Norm/            {print $NF}'      "$log")
        init=$(awk '/Max initialization/ {print $(NF-1)}' "$log")
        calc=$(awk '/Max calculation/    {print $(NF-1)}' "$log")
        red=$(awk  '/Max reduction/      {print $(NF-1)}' "$log")
        tot=$(awk  '/Max total/          {print $(NF-1)}' "$log")

        if [ -z "$tot" ]; then
            echo "  run failed, see $log"
            continue
        fi
        echo "$n,$M,$K,$SEED,$bc,$p,$rep,$norm,$init,$calc,$red,$tot" >> "$CSV"
      done
    done
  done
done

echo "Done. Results in $CSV, raw outputs in $RAW/"