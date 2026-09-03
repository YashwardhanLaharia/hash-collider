# hash-collider

Brute-force collisions for the intentionally weak 48-bit `toy_hash` function
for the CITS3402/CITS5507 assignment. The program searches for two different PDF
nonces (one per file) that produce the same hash via a birthday attack.

## Build

```sh
make
```

Requires a C11 compiler with OpenMP support (e.g. `gcc` with `-fopenmp`).

## Usage

```sh
./collider <file-a.pdf> <file-b.pdf> [options]
```

Options:

```text
--threads N        OpenMP thread count (default: 1)
--progress         show per-phase progress on stderr
```

With `--threads 1` the serial attack runs; with `N > 1` the OpenMP parallel
attack runs. With `--progress`, progress prints as
`<Serial|Parallel>: Phase A/B <pct> ETA: <time>` and each phase prints a
`Phase A/B completed in <time>` line. The final summary prints to stdout:

```text
<file-a.pdf> : nonce 0x... hash ...
<file-b.pdf> : nonce 0x... hash ...
Total time: <time>
```

The student number (`24295462`) is hardcoded as the `STUDENT_ID` macro in
`src/main.c` and is written into every solved file's header. Solved files are
written to the `solved/` directory as `solved_<input-name>`, e.g.
`example/example_a.pdf` produces `solved/solved_example_a.pdf`.

For example:

```sh
./collider example/example_a.pdf example/example_b.pdf --threads 8 --progress
```

writes `solved/solved_example_a.pdf` and `solved/solved_example_b.pdf`.

## Slurm (Kaya)

`run_test.slurm` builds and solves pairs on the `cits3402` partition. Edit the
`#SBATCH --cpus-per-task` and the pair list as needed, then submit:

```sh
sbatch run_test.slurm
squeue -u $USER
```

Output goes to `slurm_test_<jobid>.out` (stdout) and `slurm_test_<jobid>.err`
(stderr, progress). Each pair must solve within 15 minutes at up to 96 cores on
a single node.

## Source layout

- `src/main.c` contains argument parsing, program orchestration, and collision
  verification.
- `src/pdf_io.{c,h}` contains PDF loading, saving, validation, and header updates.
- `src/table.{c,h}` contains the reusable open-addressing collision table.
- `src/attack_serial.{c,h}` is the serial birthday-attack module.
- `src/attack_parallel.{c,h}` is the OpenMP birthday-attack module.
- `src/toy_hash.{c,h}` is the supplied reference hash implementation.
- `src/check_toy_hash.py` recomputes `toy_hash` independently for verification.
- `performance_analysis.md` records measured scaling behaviour for the report.
