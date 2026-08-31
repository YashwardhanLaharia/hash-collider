# Brute-Forcing `toy_hash` Collisions with OpenMP

**Author:** Yashwardhan Laharia  
**Student ID:** 24295462

## 1. Introduction

The task is to develop a parallel OpenMP program to brute-force collisions for
`toy_hash`, an intentionally weak 48-bit hash function based on FNV-1a and
MurmurHash3. For each pair of files, the program must find two nonces that,
when inserted into the file headers, cause the files to produce the same hash.
The six provided file pairs have varying difficulty. The implemented program
uses a birthday attack, first generating hashes for one file and storing them,
then searching for a matching hash for the other file.

**Overall result:** [TBD: state whether all six pairs were solved and whether
each completed within the 15-minute limit.]

## 2. Birthday-Attack Algorithm

The hash output has \(2^{48}\) possible values. Searching for a collision by
fixing one file and looking for a particular hash would require, on average,
an impractical number of trials. A birthday attack instead generates hashes
for both files. After approximately \(O(\sqrt{2^{48}}) = O(2^{24})\) trials,
there is a substantial probability that a hash from one set matches a hash
from the other. The number of trials required for approximately 50% collision
probability is \(\sqrt{2 \ln(2) \cdot 2^{48}} \approx 2.0 \times 10^7\),
although the exact result varies between runs.

The attack uses two phases. In the first phase, the program inserts a sequence
of pairs `(hash, nonce_a)` generated from file A into a collision-detection
structure. In the second phase, it generates `(hash, nonce_b)` pairs for file B
and queries the structure. A matching hash supplies candidate nonces for the
two files. The program then applies both nonces and recomputes `toy_hash` on
the complete modified files before writing the solved outputs.

**Collision-detection structure:** The implementation uses the supplied
open-addressing `collision_table`. Each entry stores a 64-bit hash, the
associated 64-bit nonce, and an occupancy flag. The table index multiplies the
hash by the 64-bit golden-ratio constant `11400714819323198485` and masks with
`capacity - 1`; therefore, capacities are powers of two. Collisions are
resolved by linear probing. In the serial implementation, the table capacity
is `2^25` entries for `2^24` file-A trials, giving a maximum planned load of
50%. This leaves space for probing while keeping the structure simple and
fast.

**Search details:** Both phases traverse nonces sequentially from zero through
`2^24 - 1`. For every file-A nonce, the nonce is inserted into the header of a
private working copy, hashed, and stored with its hash. File-B trials are then
hashed and looked up in the table. A successful lookup returns the file-A
nonce. If allocation, table initialization, or insertion fails, the attack
returns failure after releasing its resources; no unverified result is
returned. The caller performs a final hash verification before writing solved
files.

## 3. Parallelisation and Synchronisation

The serial attack provides the correctness baseline. The parallel attack uses
OpenMP to distribute independent nonce trials across threads. Each call to
`toy_hash` is treated as sequential; the implementation focuses on
parallelising independent nonce trials and the collision-detection logic
rather than modifying the hash function itself.

**Nonce assignment:** The parallel implementation uses `omp for schedule(static)`
for both phases. Each nonce in the range `0` through `2^24 - 1` is assigned to
exactly one thread, so duplicate trials are not generated. Hashes are routed to
one of `T` shared partitions using `hash % T`. Each partition has capacity equal
to the next power of two at least twice the expected share of the file-A trials.

**Shared collision-detection strategy:** The partitions are shared during
phase A, so each partition has its own OpenMP lock. A thread locks the selected
partition while performing an open-addressing insertion and unlocks it before
continuing. This protects table entries and the shared entry count from races.
A barrier separates insertion from lookup. During phase B, all partitions are
read-only and the hash determines exactly one partition, so each trial performs
one lookup rather than scanning all `T` tables. The only shared mutable search
state is the `found` flag, which uses a C11 atomic compare-and-exchange. The
winning thread publishes the nonce pair, while the parallel region's
completion provides the synchronization before the caller uses the result.

**Design rationale:** Hash partitioning changes phase-B lookup from `O(T)` table
searches per trial to one deterministic lookup. Per-partition locks avoid a
single global critical section, although phase-A insertions can still contend
when several hashes map to the same partition. Since the partitions are
immutable during phase B, that phase has no lock contention. The static schedule
is appropriate because each trial performs approximately the same amount of
work and avoids the overhead of dynamic scheduling.

**Termination:** When a thread finds a matching hash, it atomically changes
`found` from zero to one. Only the thread that succeeds in this operation
writes the solution. Other threads observe the flag at subsequent file-B
iterations and skip remaining trials. The parallel-region exit ensures that
all working buffers and partition resources are released before the function
returns.

The program verifies that a discovered pair of nonces produces a genuine
collision: `toy_hash` is recomputed on the final, modified files and the two
hashes must match before any solved file is written.

## 4. Memory Usage and Trade-offs

The principal memory cost is the collision-detection structure. Each stored
entry contains a hash value, its associated nonce, and occupancy information.
The table's approximate memory usage is therefore:

\[
M \approx C \times (\text{hash bytes} + \text{nonce bytes} +
\text{occupancy bytes}),
\]

where \(C\) is the allocated capacity. The two input files and temporary
verification buffers add comparatively small fixed costs.

**Final memory footprint:** The serial table allocates `2^25` entries. On the
target 64-bit build, `sizeof(collision_entry)` is 24 bytes because the 8-byte
hash, 8-byte nonce, and occupancy field are aligned within the structure. The
serial table therefore uses approximately 805,306,368 bytes (768 MiB), not
including the two working file copies. In the parallel implementation, every
partition allocates one table sized for its share of the file-A trials. For a
thread count `T`, each partition has capacity
`next_power_of_two(2 * ceil(2^24 / T))`, so the total table capacity is `T`
times that value, multiplied by the entry size. The parallel working file
copies add two file buffers per thread; these are minor for the small test PDFs
but can be significant for the larger provided inputs.

**Trade-offs:** The serial table's 50% planned load factor reduces linear-probe
lengths at the cost of allocating twice as many slots as the planned number of
file-A trials. Partition locks add phase-A synchronization, but avoid the
`O(T)` phase-B lookup that occurred with separate thread-owned tables. The
fixed `2^24` trial window keeps the birthday-attack work bounded; if no match
is found in that window, the function reports failure rather than silently
returning an invalid collision.

## 5. Performance Results and Analysis

Timing covers only the search and collision-detection portion of the program,
excluding file I/O. The parallel implementation must be run on Kaya while
varying the number of threads, and each provided pair must be timed. The job
must complete within 15 minutes per pair on a single Kaya node using up to 96
cores. Measurements were taken on [TBD: Kaya node specification, compiler
version, and OpenMP/compiler details]. Each configuration was run [TBD: number
of repetitions and summary statistic used].

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

| Pair | 1 thread | 2 | 4 | 8 | 16 | 32 | 64 | 96 |
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
