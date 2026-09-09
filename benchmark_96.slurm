#!/bin/bash
#SBATCH --job-name=benchmark_96
#SBATCH --partition=cits3402
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=96
#SBATCH --exclusive
#SBATCH --mem=8G
#SBATCH --time=02:00:00
#SBATCH --output=benchmark_96_%j.out
#SBATCH --error=benchmark_96_%j.err

module load gcc/14.3.0

cd "$SLURM_SUBMIT_DIR" || exit 1

make clean && make || {
    echo "Build failed" >&2
    exit 1
}

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

for repetition in 1 2 3; do
    for pair in 1_kilo 2_mega 3_giga 4_tera 5_peta 6_exa; do
        echo "=== repetition $repetition: $pair pair ==="

        output="benchmark_96_${SLURM_JOB_ID}_${pair}_r${repetition}.out"
        error="benchmark_96_${SLURM_JOB_ID}_${pair}_r${repetition}.err"

        ./collider "files/${pair}_a.pdf" "files/${pair}_b.pdf" \
            --threads 96 > "$output" 2> "$error"

        cat "$output"
    done
done

echo "96-thread benchmark complete."
