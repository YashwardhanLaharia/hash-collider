# Performance Analysis Notes (for the report)

Working notes documenting measured behaviour of the parallel attack, to be
incorporated into `report.md` §5.

## Key finding: the attack is memory-bandwidth-bound, not lock-bound

Both Phase A and Phase B spend essentially all their time in `toy_hash` over
the *entire* file. `toy_hash` is a byte-by-byte sequential FNV-1a/Murmur mix
that re-reads every byte of the PDF for every single nonce trial. Only 16 bytes
(the nonce at bytes 16–31) change between trials, but the hash function has no
incremental API and is treated (per the spec) as sequential/atomic, so each
trial pays the full-file hashing cost.

Measured throughput on the development laptop (16 cores; `-O3 -fopenmp`):

| File size | Threads | Throughput        |
|----------:|--------:|------------------:|
| 64 KB     | 1       | 0.02 Mhash/s      |
| 64 KB     | 16      | 0.14 Mhash/s      |
| 1 KB      | 16      | 8.20 Mhash/s      |

Phase A inserts `2^24 = 16.7M` hashes of the 64 KB `1_kilo` file. At
0.14 Mhash/s that is ~119 s — matching the observed `Phase A completed in
~131–150s`. Phase A cost is therefore ~100% hashing, and the lock/insert and
table-probe overhead is negligible by comparison.

Byte-driven figures make the ceiling obvious:

- 16 threads hash a 64 KB file at ~9.3 GB/s vs ~1.0 GB/s serial (~8.8×,
  already approaching the laptop's memory-bandwidth limit).
- 16 threads hash a 1 KB file at ~8.4 GB/s (same bandwidth, more hashes/s).

Conclusion: performance scales with thread count until memory bandwidth
saturates; both phases are bandwidth-bound, and per-trial cost grows linearly
with file size (kilo → exa).

## Why cache-line-padding the locks did not help

A change was made to place each `omp_lock_t` in its own 64-byte cache line
(`padded_lock` in `src/attack_parallel.c`). It produced **no measurable**
speedup (Phase A ~149 s vs ~131 s baseline; within run-to-run noise). The
reason: lock operations cost ~nanoseconds while a full-file `toy_hash` costs
~microseconds (64 KB), so there was no meaningful false sharing to remove.
The lock-padding change is retained (harmless), but should **not** be cited as
a performance fix in the report.

The earlier hypothesis in `kaya_scaling_notes.md` (that contiguous `omp_lock_t`
objects cause phase-A false sharing on Kaya) is **not supported** by the
measured behaviour. Phase A was slow because hashing dominates, not because of
lock contention.

## Implication for the report's "what could be optimised" discussion

The honest, measurement-backed position is:

1. The parallelisation is already close to optimal for this workload — it
   parallelises independent nonce trials, uses a partitioned table, and does
   lock-free phase-B lookups.
2. The binding constraint is `toy_hash` memory bandwidth on the *full* file.
   Larger files (kilo 64 KB → exa 900 KB) cost proportionally more per trial.
3. No scheduling (`static` vs `guided`) or locking change moves the needle
   materially, because the hot path is the byte-serial hash, not
   synchronisation.

## Numbers to carry into the report tables

From actual runs:

| Pair   | Threads | Phase A (s) | Phase B (s) | Total (s) |
|--------|--------:|------------:|------------:|----------:|
| 1_kilo | 16      | ~131–150    | ~65         | ~197      |

(Add Kaya 64/96-thread rows and the remaining pairs once benchmarked.)

## Verification of raw throughput

Reproduce with a tiny harness that halts after a fixed time budget (no need to
run to completion):

- Compile a loop calling `toy_hash` repeatedly in a `num_threads(t)` parallel
  region for `budget` seconds, counting hashes and reporting Mhash/s and MB/s.
- Run for 1 KB and 64 KB buffers at 1 and 16 threads.

This isolates `toy_hash` throughput from table/lock overhead and directly
explains the phase-A wall time.
