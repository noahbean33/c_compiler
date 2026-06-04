# Test Framework Integration Plan

## Overview

The `tests/` directory contains the [Writing a C Compiler](https://github.com/nlsandler/writing-a-c-compiler-tests) test suite. This document describes how it is integrated with our compiler with minimal code changes.

## Interface Gap

### Test framework expects:
```
./compiler [--lex|--parse|--validate|--tacky|--codegen] [-c] [-lm] <source_file.c>
```
- Exit code **0** on success, **non-zero** on failure
- Full compile produces executable at `<source_file>` (same path, `.c` stripped)
- `-c` flag produces object file at `<source_file>.o`
- Stage flags (`--lex`, `--parse`, etc.) exit after that stage, produce no output files

### Our compiler currently accepts:
```
./main <input_file> <output_file> [exec|object]
```
- Always exits 0 (even on failure)
- Outputs assembly, then assembles/links via `nasm` + `gcc -m32`
- Output path is explicitly specified (not derived from input)

## Integration Strategy: Wrapper Script

A Bash wrapper script (`mycc`) translates the test framework's calling convention into our compiler's interface. This avoids modifying the test framework and requires only one small fix to `main.c`.

### Wrapper responsibilities:
1. Parse flags: `--lex`, `--parse`, `--validate`, `--tacky`, `--codegen`, `-c`, `-lm`
2. Extract the source file (last positional argument)
3. Derive output paths from the source file path (strip `.c`)
4. Call `./main <source> <output> [exec|object]` with the correct arguments
5. For stage-only flags (`--lex`, `--parse`, etc.), exit 0 and produce no output files
6. Forward the compiler's exit code to the test framework

### Required code change in `main.c`:
- Return non-zero exit code when `compile_file()` returns `COMPILER_FAILED_WITH_ERRORS`
- Currently line 66 always returns 0; must return 1 on failure

## File Layout

```
c_compiler/
├── mycc              # Wrapper script (new)
├── main.c            # One-line fix: return 1 on error
├── Makefile          # New "test" target
└── tests/
    ├── test_compiler # Test runner entry point
    └── ...
```

## Usage

```bash
# Build the compiler
make

# Run chapter 1 tests
make test CHAPTER=1

# Or run directly
cd tests
python3 test_compiler ../mycc --chapter 1

# More options
python3 test_compiler ../mycc --chapter 1 --skip-invalid
python3 test_compiler ../mycc --chapter 1 -f   # stop on first failure
python3 test_compiler ../mycc --chapter 1 -v   # verbose
```

## Limitations

- Stage flags (`--lex`, `--parse`, etc.) are stubbed — the wrapper exits 0 without actually running only that stage. To properly test intermediate stages, the compiler itself would need `--stage` support.
- The compiler targets x86 32-bit (NASM + ELF32 + `gcc -m32`). The test framework expects the produced executable to run on the host machine, so a 32-bit toolchain must be available.
- `-lm` (math library linking) is acknowledged by the wrapper but depends on the compiler's linker invocation supporting it.
