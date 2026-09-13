#!/bin/bash
#SBATCH --job-name=scale_1_kilo
#SBATCH --partition=cits3402
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
#SBATCH --time=02:00:00
#SBATCH --output=scale_kilo_%j.out
#SBATCH --error=scale_kilo_%j.err

module load gcc/14.3.0

cd "$SLURM_SUBMIT_DIR" || exit 1

make clean && make || {
    echo "Build failed" >&2
    exit 1
}

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

for threads in 1; do
    for repetition in 1 2 3; do
        echo "=== 1_kilo: ${threads} threads, repetition ${repetition} ==="

        output="scale_kilo_${SLURM_JOB_ID}_t${threads}_r${repetition}.out"
        error="scale_kilo_${SLURM_JOB_ID}_t${threads}_r${repetition}.err"

        ./collider files/1_kilo_a.pdf files/1_kilo_b.pdf \
            --threads "$threads" > "$output" 2> "$error"

        cat "$output"
    done
done

echo "Scaling test complete."
