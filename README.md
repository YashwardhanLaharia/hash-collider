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
written to the current directory as `solved_<input-name>`, e.g.
`example/example_a.pdf` produces `solved_example_a.pdf`.

For example:

```sh
./collider example/example_a.pdf example/example_b.pdf --threads 8
```

writes `solved_example_a.pdf` and `solved_example_b.pdf`.
