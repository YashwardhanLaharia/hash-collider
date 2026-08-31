# Brute-Forcing `toy_hash` Collisions with OpenMP

**Author:** Yashwardhan Laharia  
**Student ID:** 24295462

## 1. Introduction

This project searches for two different nonce values, one for each of two PDF
files, that produce the same output from the intentionally weak 48-bit
`toy_hash` function. The six provided file pairs have varying difficulty. The
implemented program uses a birthday attack, first generating hashes for one
file and storing them, then searching for a matching hash for the other file.

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

**Collision-detection structure:** [TBD: describe the final data structure,
including its representation, hash-index calculation, collision handling,
capacity/load-factor choice, and why it was selected.]

**Search details:** [TBD: state the nonce traversal order, the number of
trials used in each phase, and how table exhaustion or a failed search is
handled.]

## 3. Parallelisation and Synchronisation

The serial attack provides the correctness baseline. The parallel attack uses
OpenMP to distribute independent nonce trials across threads. Each trial
modifies a private in-memory copy or otherwise avoids changing shared file
bytes, because each call to `toy_hash` is treated as sequential and the
parallelism comes from independent trials.

**Nonce assignment:** [TBD: explain how nonce values are divided between
threads, whether scheduling is static or dynamic, and how duplicate trials are
prevented.]

**Shared collision-detection strategy:** [TBD: describe the strategy actually
implemented for table insertion and lookup. Explain exactly which operations
are shared, which synchronization primitives or ownership rules protect them,
and how the strategy avoids data races.]

**Design rationale:** [TBD: explain why this strategy was chosen over the
available alternatives, such as critical sections, atomics, thread-local
tables with merging, or a partitioned table. Discuss synchronization overhead
and possible contention.]

**Termination:** [TBD: explain how one thread publishes a successful
collision, how other threads observe the result, and how unnecessary work is
stopped safely.]

After a candidate is found, the program performs a final verification before
writing either output file. This verification recomputes both complete file
hashes and accepts the result only when the two 48-bit values are equal.

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

**Final memory footprint:** [TBD: give the table capacity, entry size,
number of tables if applicable, total memory usage, and the basis for those
values.]

**Trade-offs:** [TBD: explain the chosen capacity/load factor and the balance
between memory consumption, cache behaviour, probe length, synchronization,
and search speed. If thread-local or partitioned structures are used, explain
the additional memory cost and why it is worthwhile.]

## 5. Performance Results and Analysis

The search timer measures only the search and collision-detection phase. File
loading, output, and final verification are excluded. Measurements were taken
on [TBD: Kaya node specification, compiler version, and OpenMP/compiler
details]. Each configuration was run [TBD: number of repetitions and summary
statistic used].

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
