#!/bin/bash
#SBATCH --job-name=2peta_exa
#SBATCH --partition=cits3402
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64
#SBATCH --mem=8G
#SBATCH --time=01:00:00
#SBATCH --output=2peta_exa_%j.out
#SBATCH --error=2peta_exa_%j.err

module load gcc/14.3.0

cd "$SLURM_SUBMIT_DIR" || exit 1

make clean && make || {
    echo "Build failed" >&2
    exit 1
}

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=== kilo pair ==="
./collider files/1_kilo_a.pdf files/1_kilo_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "=== mega pair ==="
./collider files/2_mega_a.pdf files/2_mega_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "=== giga pair ==="
./collider files/3_giga_a.pdf files/3_giga_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "=== tera pair ==="
./collider files/4_tera_a.pdf files/4_tera_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "=== peta pair ==="
./collider files/5_peta_a.pdf files/5_peta_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress

echo "=== exa pair ==="
./collider files/6_exa_a.pdf files/6_exa_b.pdf --threads "$SLURM_CPUS_PER_TASK" --progress


echo "Done."
