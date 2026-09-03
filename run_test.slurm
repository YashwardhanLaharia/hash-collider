#!/bin/bash
#SBATCH --job-name=test-run
#SBATCH --partition=cits3402
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64
#SBATCH --mem=8G
#SBATCH --time=00:20:00
#SBATCH --output=slurm_test_%j.out
#SBATCH --error=slurm_test_%j.err

module load gcc/14.3.0

cd "$SLURM_SUBMIT_DIR" || exit 1

make clean && make || {
    echo "Build failed" >&2
    exit 1
}

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=== 1_kilo pair ==="
./collider files/1_kilo_a.pdf files/1_kilo_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "=== 2_mega pair ==="
./collider files/2_mega_a.pdf files/2_mega_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "Done."
