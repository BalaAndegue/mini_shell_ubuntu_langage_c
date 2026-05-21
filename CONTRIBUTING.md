# Contributing to minishell

## Code Style

- C11 standard (`-std=c11`) with POSIX 200809L
- `clang-format` enforced (see `.clang-format`)
- No global mutable state except the shell context struct
- Every syscall return value must be checked
- No `malloc` without a matching `free` path

## Workflow

1. Branch from `main`: `git checkout -b feat/pipe-support`
2. Run `make debug` — must compile without warnings
3. Run `make lint` — must pass cppcheck and clang-tidy
4. Run `make memcheck` — zero Valgrind errors
5. Add or update tests in `tests/`
6. Open a PR with a clear description of what and why

## Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: add pipeline execution (cmd1 | cmd2)
fix: handle SIGINT in interactive mode without exiting shell
refactor: extract tokenizer into src/lexer/lexer.c
test: add integration tests for I/O redirection
docs: update architecture diagram for pipeline model
```

## Testing Requirements

- New feature → new unit test in `tests/unit/`
- New feature → new integration test in `tests/integration/`
- Bug fix → regression test that would have caught the bug

## Useful Commands for Development

```bash
# Trace all syscalls made by the shell
strace -f ./build/bin/minishell

# Check file descriptor leaks
ls -la /proc/$(pidof minishell)/fd

# Run under gdb
gdb -q ./build/bin/minishell

# Show open file descriptors in a child process
(gdb) catch syscall open dup2 close
```
