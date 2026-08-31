# Brute-Forcing `toy_hash` Collisions with OpenMP

**Author:** Yashwardhan Laharia  
**Student ID:** 24295462

## 1. Introduction

The program uses a birthday attack to find nonces that make two PDFs collide
under the supplied 48-bit `toy_hash`. It first stores hashes generated from
file A, then searches hashes from file B for a cross-match. OpenMP distributes
the independent nonce trials while a partitioned table supports concurrent
collision detection.

**Overall result:** [TBD: state whether all six pairs were solved and whether
each completed within the 15-minute limit.]

## 2. Birthday-Attack Algorithm

Assuming independent, approximately uniform outputs, sets containing \(N_A\)
and \(N_B\) hashes have cross-match probability
\(1-e^{-N_A N_B/2^{48}}\). Equal sets reach 50% probability at
\(N_A=N_B=\sqrt{\ln(2)2^{48}}\approx1.40\times10^7\), or about
\(2.79\times10^7\) total hashes. The actual collision location for each input
pair is fixed because the implementation searches deterministic nonce ranges.

Phase A inserts `(hash, nonce_a)` pairs into a collision table. Phase B
generates `(hash, nonce_b)` pairs and queries that table. A match supplies the
two candidate nonces, which the caller independently verifies before writing
either solved file.

**Collision table:** Each open-addressing entry stores a hash, its file-A
nonce, and an occupancy flag. A multiplicative index is masked by
`capacity - 1`, so capacities are powers of two; collisions use linear
probing. The serial table has `2^25` entries for `2^24` A trials, limiting its
planned load to 50% and reducing probe lengths.

**Search ranges:** Phase A hashes nonces zero through `2^24 - 1`. With that
table, the expected B probes before a match are `2^24`, giving approximately
`2^25` total hash calls. Phase B searches in consecutive `8 * 2^24`-nonce
batches while reusing the A table. The first batch has expected cross-match
count eight and success probability `1 - e^-8 = 99.97%`; if needed, searching
continues with the next non-overlapping batch until a match or exhaustion of
the 64-bit nonce space.

## 3. Parallelisation and Synchronisation

The serial attack is the correctness and speedup baseline. The OpenMP version
parallelises independent nonce trials rather than the sequential operations
inside each `toy_hash` call.

**Work assignment:** Both phases use `omp for schedule(static)`, assigning each
nonce once. Hashes are routed to one of `T` partitions using `hash % T`, where
`T` is the actual OpenMP team size. Each partition is sized to the next power
of two at least twice its expected share of A hashes.

**Synchronization:** During phase A, one OpenMP lock per partition protects
entries and the shared entry count. A barrier completes every insertion before
phase B, when the tables are read-only and require no locks. A C11 atomic
compare-and-exchange on `found` selects exactly one thread to publish the
solution; completion of the parallel region synchronizes that result with the
caller.

**Rationale:** Partitioning gives one deterministic phase-B lookup instead of
searching `T` thread-owned tables. Per-partition locks avoid one global critical
section, although phase-A hashes targeting the same partition can contend.
Static scheduling has low overhead because trials perform similar work.

**Termination:** After a match, other threads observe `found` and skip hashing
on their remaining worksharing iterations. Only loop-control overhead remains
until the phase barrier, after which resources are released normally.

## 4. Memory Usage and Trade-offs

The table dominates memory use. On the target 64-bit build, alignment makes
each entry 24 bytes. The serial `2^25`-entry table therefore occupies
805,306,368 bytes (768 MiB), excluding two working PDF copies.

In parallel, each of `T` partitions has capacity
`next_power_of_two(2 * ceil(2^24 / T))`. At 96 threads this is `2^19` entries
per partition, or 1.125 GiB of table storage. Each thread also owns two PDF
buffers, making file size relevant to total memory.

The 50% planned load spends memory to shorten linear probes. Partition locks
add phase-A synchronization but enable one lock-free phase-B lookup. Reusing
the A table for further B batches increases search time without increasing
table memory.

## 5. Performance Results and Analysis

Timing covers the complete attack routine, including table setup and cleanup,
but excludes input loading, final verification, and output writing.
Progress logging is disabled by default, so benchmark runs avoid its extra
timing and synchronization overhead. It can be enabled with `--progress` for
long-running interactive executions.
Measurements used [TBD: Kaya node, compiler/OpenMP details, repetitions, and
summary statistic]. Because every trial hashes the complete PDF, larger inputs
should cost more per trial even though the expected trial count is governed by
the 48-bit collision probability.

### 5.1 Completion Results

| Pair | Threads | Trials [TBD] | Search time (s) | Completed within 15 min? |
|---|---:|---:|---:|:---:|
| `1_kilo` | [TBD] | [TBD] | [TBD] | [TBD] |
| `2_mega` | [TBD] | [TBD] | [TBD] | [TBD] |
| `3_giga` | [TBD] | [TBD] | [TBD] | [TBD] |
| `4_tera` | [TBD] | [TBD] | [TBD] | [TBD] |
| `5_peta` | [TBD] | [TBD] | [TBD] | [TBD] |
| `6_exa` | [TBD] | [TBD] | [TBD] | [TBD] |

### 5.2 Scaling Results

| Pair | 1 thread (serial) | 2 | 4 | 8 | 16 | 32 | 64 | 96 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `1_kilo` | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] |
| `2_mega` | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] |
| `3_giga` | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] |
| `4_tera` | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] |
| `5_peta` | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] |
| `6_exa` | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] | [TBD] |

Times are in seconds. **Speedup and efficiency:** [TBD: calculate speedup
relative to one thread, discuss scaling across thread counts, and identify
where performance stops scaling.]

**Difficulty comparison:** [TBD: compare the six pairs, relate their observed
trial counts and times to the expected birthday-attack behaviour, and explain
any variation caused by random collision location or implementation effects.]

**Performance conclusion:** [TBD: state the best configuration, the slowest
pair, the maximum observed speedup, and whether the 15-minute requirement was
met for every pair.]

## 6. Conclusion

The project demonstrates a birthday attack against a weak 48-bit hash and a
parallel implementation using OpenMP. [TBD: summarise the final algorithm,
synchronization design, memory trade-off, and measured performance in two or
three sentences.]
