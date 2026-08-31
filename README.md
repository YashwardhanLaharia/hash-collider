# hash-collider

Skeleton implementation for the CITS3402/CITS5507 hash-collision assignment.
The project searches for two different PDF nonces that produce the same value
from the intentionally weak 48-bit `toy_hash` function.

## Build

```sh
make
```

The current skeleton builds the command-line, file-I/O, nonce insertion,
collision verification, output, and timing infrastructure. The
`birthday_attack_serial()` and `birthday_attack_parallel()` search routines
are intentionally left as TODOs for implementation.

## Tasks

- [ ] Implement `birthday_attack_serial()` in `src/attack_serial.c`: insert
      `(hash, nonce)` trials for file A into the collision table, then trial
      nonces for file B until a match is found.
- [ ] Implement `birthday_attack_parallel()` in `src/attack_parallel.c` with
      OpenMP; pick and document a strategy for the shared collision-detection
      structure (criticals/atomics, thread-local tables merged periodically,
      or a partitioned table).
- [ ] Test both search paths on `example/example_{a,b}.pdf` locally.
- [ ] Add a Slurm script for single-node Kaya jobs (up to 96 threads,
      15-minute limit per pair).
- [ ] Solve all 6 pairs in `files/` on Kaya and keep the `solved_*.pdf`
      outputs for submission.
- [ ] Benchmark each pair across thread counts (e.g. 1/2/4/8/16/32/64/96)
      and record raw timings for the report.
- [ ] Verify every solved pair independently, e.g.
      `python3 src/check_toy_hash.py solved/solved_1_kilo_a.pdf` matches
      `solved/solved_1_kilo_b.pdf`.
- [ ] Write the 1,000 +/- 100 word report (PDF): birthday-attack algorithm and
      collision-detection data structure, OpenMP parallelisation and
      race-condition management, memory footprint and trade-offs, and
      speedup/scalability analysis.
- [ ] Package the submission zip/tar: all sources, Makefile, README, report
      PDF, and the solved pairs (exclude `docs/` and build artifacts).

## Source layout

- `src/main.c` contains argument parsing and program orchestration.
- `src/pdf_io.{c,h}` contains PDF loading, saving, validation, and header updates.
- `src/table.{c,h}` contains the reusable open-addressing collision table.
- `src/attack_serial.{c,h}` is the serial birthday-attack module.
- `src/attack_parallel.{c,h}` is the OpenMP birthday-attack module.
- `src/toy_hash.{c,h}` is the supplied reference hash implementation.

## Usage

```sh
./collider <file-a.pdf> <file-b.pdf> [options]
```

Options:

```text
--threads N        OpenMP thread count (default: 1)
```

The student number (`24295462`) is hardcoded as the `STUDENT_ID` macro in
`src/main.c` and is written into every solved file's header. Solved files are
written to the `solved/` directory as `solved_<input-name>`, e.g.
`example/example_a.pdf` produces `solved/solved_example_a.pdf`.

For example:

```sh
./collider example/example_a.pdf example/example_b.pdf --threads 8
```

writes `solved/solved_example_a.pdf` and `solved/solved_example_b.pdf`.
