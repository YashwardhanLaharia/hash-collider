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
`birthday_attack()` search routine in `src/collider.c` is intentionally left
as a TODO for implementation.

## Usage

```sh
./collider <file-a.pdf> <file-b.pdf> [options]
```

Options:

```text
--student-id ID    eight-digit student number (default: 00000000)
--threads N        OpenMP thread count (default: 1)
-o FILE_A FILE_B   output paths for solved files
```

For example:

```sh
./collider example/example_a.pdf example/example_b.pdf \
    --student-id 12345678 --threads 8 -o solved_a.pdf solved_b.pdf
```
