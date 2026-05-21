# minishell

> A POSIX-compliant Unix shell written in C — built from scratch as a systems programming reference project.

[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.cppreference.com/w/c/11)
[![POSIX](https://img.shields.io/badge/POSIX-200809L-green.svg)](https://pubs.opengroup.org/onlinepubs/9699919799/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](#build)

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Architecture](#architecture)
4. [Directory Structure](#directory-structure)
5. [Requirements](#requirements)
6. [Build](#build)
7. [Usage](#usage)
8. [Internals](#internals)
9. [Testing](#testing)
10. [Tools](#tools)
11. [Roadmap](#roadmap)
12. [References](#references)
13. [Contributing](#contributing)
14. [License](#license)

---

## Overview

`minishell` is a minimal but rigorous POSIX shell implementation targeting Linux/Ubuntu. The goal is not to replace `bash` but to understand, from first principles, how a Unix shell works: process creation, file descriptor manipulation, signal handling, and command parsing.

This project follows the architecture of production shells (dash, mksh) rather than toy interpreters.

---

## Features

### Implemented
- [x] Interactive REPL with `libreadline` (line editing, history navigation)
- [x] External command execution via `fork(2)` + `execvp(3)`
- [x] Built-in commands: `cd`, `exit`
- [x] `waitpid(2)` with proper status inspection

### In Progress
- [ ] I/O redirection: `>`, `>>`, `<`, `2>`, `2>&1`
- [ ] Pipelines: `cmd1 | cmd2 | cmd3`
- [ ] Environment variables: `$VAR`, `export`, `unset`, `env`
- [ ] Signal handling: `SIGINT`, `SIGQUIT`, `SIGCHLD`, `SIGTSTP`
- [ ] Job control: `&`, `fg`, `bg`, `jobs`

### Planned
- [ ] Globbing: `*`, `?`, `[a-z]`
- [ ] Here-documents: `<< EOF`
- [ ] Command substitution: `$(cmd)` and `` `cmd` ``
- [ ] Arithmetic expansion: `$((expr))`
- [ ] Persistent history file (`~/.minishell_history`)
- [ ] `PS1` prompt customization

---

## Architecture

### High-Level Data Flow

```
  stdin
    │
    ▼
┌───────────────────────────────────────────────────────────────────┐
│                          SHELL LOOP                               │
│                                                                   │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│   │  READER  │───▶│  LEXER   │───▶│  PARSER  │───▶│EXECUTOR  │  │
│   │          │    │          │    │          │    │          │  │
│   │ readline │    │ tokenize │    │ AST/cmd  │    │ dispatch │  │
│   │ fgets    │    │ strtok   │    │ pipeline │    │          │  │
│   └──────────┘    └──────────┘    └──────────┘    └────┬─────┘  │
│                                                         │        │
│                                          ┌──────────────┴──────┐ │
│                                          │                     │ │
│                                   ┌──────▼──────┐  ┌──────────▼┐│
│                                   │  BUILTINS   │  │ EXTERNAL  ││
│                                   │             │  │           ││
│                                   │ cd pwd echo │  │fork+execvp││
│                                   │ export unset│  │           ││
│                                   │ history jobs│  │  waitpid  ││
│                                   └─────────────┘  └───────────┘│
└───────────────────────────────────────────────────────────────────┘
```

### Process Model

```
  Shell (PID: parent)
        │
        │  fork(2)
        ├─────────────────────────────────────┐
        │                                     │
        ▼                                     ▼
  parent                                  child
  waitpid(pid, &status, 0)                execvp(argv[0], argv)
  inspect WIFEXITED / WIFSIGNALED         ──► replaces image with cmd
        │                                     │
        │◀────────────────────────────────────┘
        │  child exits → SIGCHLD → parent wakes
        ▼
  next prompt
```

### Pipeline Execution Model

```
  "ls -la | grep src | wc -l"

  Shell
    │
    ├── pipe(fd[0], fd[1])    ← pipe A
    ├── pipe(fd[0], fd[1])    ← pipe B
    │
    ├── fork ──▶ ls -la       stdout ──▶ pipe_A_write
    ├── fork ──▶ grep src     stdin  ◀── pipe_A_read
    │                         stdout ──▶ pipe_B_write
    └── fork ──▶ wc -l        stdin  ◀── pipe_B_read
                              stdout ──▶ terminal
```

### I/O Redirection Model

```
  "cmd < in.txt > out.txt 2>&1"

  open("in.txt",  O_RDONLY)  → fd=3
  open("out.txt", O_WRONLY | O_CREAT | O_TRUNC) → fd=4

  dup2(3, STDIN_FILENO)   ← stdin  now reads from in.txt
  dup2(4, STDOUT_FILENO)  ← stdout now writes to out.txt
  dup2(4, STDERR_FILENO)  ← stderr now writes to out.txt (2>&1)

  close(3); close(4);     ← close originals in child
  execvp("cmd", args)
```

---

## Directory Structure

```
minishell/
├── src/
│   ├── core/
│   │   ├── shell.c           # Main REPL loop
│   │   └── shell.h
│   ├── lexer/
│   │   ├── lexer.c           # Tokenizer: string → token list
│   │   └── lexer.h
│   ├── parser/
│   │   ├── parser.c          # Token list → command AST
│   │   └── parser.h
│   ├── executor/
│   │   ├── executor.c        # AST traversal + dispatch
│   │   └── executor.h
│   ├── builtins/
│   │   ├── builtins.c        # cd, echo, pwd, export, unset, env, exit
│   │   └── builtins.h
│   ├── io/
│   │   ├── redirect.c        # dup2-based I/O redirection
│   │   ├── redirect.h
│   │   ├── pipe.c            # Multi-stage pipeline setup
│   │   └── pipe.h
│   ├── env/
│   │   ├── env.c             # Environment variable table
│   │   └── env.h
│   ├── signals/
│   │   ├── signals.c         # sigaction handlers
│   │   └── signals.h
│   └── history/
│       ├── history.c         # readline history + persistence
│       └── history.h
├── include/
│   └── minishell.h           # Shared types, macros, structs
├── tests/
│   ├── unit/                 # Per-module unit tests (Unity framework)
│   └── integration/          # Shell-script-based integration tests
├── docs/
│   ├── Doxyfile              # Doxygen configuration
│   └── diagrams/             # ASCII / PlantUML architecture diagrams
├── scripts/
│   └── run_tests.sh          # Integration test runner
├── .github/
│   └── workflows/
│       └── ci.yml            # GitHub Actions: build + lint + test
├── Makefile
├── README.md
├── CONTRIBUTING.md
├── CHANGELOG.md
└── LICENSE
```

---

## Requirements

| Dependency    | Purpose                          | Install                        |
|---------------|----------------------------------|--------------------------------|
| `gcc >= 11`   | Compiler (C11)                   | `apt install build-essential`  |
| `libreadline` | Line editing + history           | `apt install libreadline-dev`  |
| `valgrind`    | Memory error detection           | `apt install valgrind`         |
| `cppcheck`    | Static analysis                  | `apt install cppcheck`         |
| `clang-tidy`  | Linting                          | `apt install clang-tidy`       |
| `lcov`        | Code coverage HTML reports       | `apt install lcov`             |
| `doxygen`     | API documentation                | `apt install doxygen graphviz` |

Install all at once:

```bash
sudo apt update && sudo apt install -y \
    build-essential libreadline-dev valgrind \
    cppcheck clang-tidy lcov doxygen graphviz
```

---

## Build

```bash
# Clone
git clone https://github.com/YOUR_USERNAME/minishell.git
cd minishell

# Release build
make

# Debug build (AddressSanitizer + UBSan + -g3)
make debug

# Full rebuild
make re
```

Binary is output to `build/release/bin/minishell`.

---

## Usage

```bash
./build/release/bin/minishell
```

```
minishell$ ls -la | grep src
minishell$ cat /etc/passwd | grep root > root.txt
minishell$ export EDITOR=vim
minishell$ echo "Hello $USER from $SHELL"
minishell$ cd /tmp && pwd
minishell$ sleep 10 &
[1] 4221
minishell$ jobs
[1]+ Running  sleep 10
minishell$ exit
```

---

## Internals

### Token Types

```c
typedef enum {
    TOK_WORD,        /* regular word / argument          */
    TOK_PIPE,        /* |                                */
    TOK_REDIR_IN,    /* <                                */
    TOK_REDIR_OUT,   /* >                                */
    TOK_REDIR_APPEND,/* >>                               */
    TOK_REDIR_ERR,   /* 2>                               */
    TOK_HEREDOC,     /* <<                               */
    TOK_BG,          /* &                                */
    TOK_SEMICOLON,   /* ;                                */
    TOK_EOF,
} TokenType;
```

### Command Node (AST)

```c
typedef struct s_cmd {
    char      **argv;          /* NULL-terminated argument vector  */
    int         argc;
    t_redir    *redirs;        /* linked list of redirections       */
    struct s_cmd *pipe_next;   /* next command in pipeline          */
    int         background;    /* run with &                        */
} t_cmd;
```

### Built-in Dispatch Table

```c
static const t_builtin builtins[] = {
    { "cd",      builtin_cd      },
    { "echo",    builtin_echo    },
    { "env",     builtin_env     },
    { "exit",    builtin_exit    },
    { "export",  builtin_export  },
    { "history", builtin_history },
    { "jobs",    builtin_jobs    },
    { "pwd",     builtin_pwd     },
    { "type",    builtin_type    },
    { "unset",   builtin_unset   },
    { NULL, NULL }
};
```

---

## Testing

### Unit Tests (Unity)

```bash
make test
```

Tests are organized per module under `tests/unit/`. The [Unity](https://github.com/ThrowTheSwitch/Unity) test framework is used (single-file, no external deps).

### Integration Tests

```bash
make test-integration
```

Each test in `tests/integration/` is a shell script that runs `minishell` non-interactively and diffs the output against the expected result.

Example:
```bash
echo "echo hello world" | ./build/bin/minishell
# expected: hello world
```

### Memory Check

```bash
make memcheck
```

---

## Tools

| Tool          | Role                          | Why it matters                              |
|---------------|-------------------------------|---------------------------------------------|
| `gcc -Wall`   | Compiler warnings             | Catches UB, dead code, type mismatches      |
| `AddressSanitizer` | Runtime memory errors    | Detects heap overflows, use-after-free      |
| `UBSan`       | Undefined behaviour at runtime| Integer overflow, null deref, misalignment  |
| `valgrind`    | Memory leak detection         | Finds leaks that ASAN misses                |
| `cppcheck`    | Static analysis               | Finds bugs without running code             |
| `clang-tidy`  | Linting + modernisation hints | Enforces coding guidelines                  |
| `gcov/lcov`   | Code coverage                 | Measures test thoroughness                  |
| `gdb`         | Debugger                      | Breakpoints, backtraces, core dumps         |
| `strace`      | System call tracing           | Verify `open/fork/execve/dup2` sequences    |
| `doxygen`     | API documentation             | HTML docs generated from source comments    |

---

## Roadmap

```
v0.1  ✅  fork/execvp + cd/exit + basic tokenizer
v0.2  🔧  I/O redirection (< > >> 2> 2>&1)
v0.3  🔧  Pipelines (cmd1 | cmd2 | ... | cmdN)
v0.4  🔧  Environment variables + export/unset/env
v0.5  🔧  Signal handling (SIGINT, SIGQUIT, SIGCHLD)
v0.6  🔧  Job control (& fg bg jobs)
v0.7  🔧  Globbing (* ? [...])
v0.8  🔧  Here-documents (<< EOF)
v0.9  🔧  Command substitution $() + arithmetic $(())
v1.0  🔧  readline integration + persistent history
```

---

## References

### Books (essential for system developers)

| Book | Author | Why |
|------|--------|-----|
| *Advanced Programming in the UNIX Environment (3rd ed.)* | Stevens, Rago | The definitive POSIX reference: `fork`, `exec`, signals, file descriptors |
| *The Linux Programming Interface* | Kerrisk | Linux-specific syscall reference, more up to date than Stevens |
| *The Unix Programming Environment* | Kernighan, Pike | Classic: shell philosophy and design |
| *Operating Systems: Three Easy Pieces* | Arpaci-Dusseau | Process model, scheduling, concurrency |

### Standards

| Reference | URL |
|-----------|-----|
| POSIX.1-2017 Shell & Utilities | https://pubs.opengroup.org/onlinepubs/9699919799/ |
| POSIX Signal semantics | https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html |
| GNU Bash manual | https://www.gnu.org/software/bash/manual/bash.html |

### Source Code to Study

| Project | URL | What to learn |
|---------|-----|---------------|
| **bash** (GNU) | https://github.com/bminor/bash | Full POSIX shell, see `execute_cmd.c`, `jobs.c` |
| **dash** | https://git.kernel.org/pub/scm/utils/dash/dash.git | Minimal, fast; study `jobs.c`, `eval.c`, `parser.c` |
| **xv6 shell** | https://github.com/mit-pdos/xv6-public/blob/master/sh.c | ~150 lines: the canonical teaching example |
| **busybox sh** | https://github.com/mirror/busybox/tree/master/shell | Embedded constraints, efficient parsing |
| **mksh** | https://github.com/MirBSD/mksh | Clean codebase, good signal handling |
| **42 minishell** (examples) | https://github.com/search?q=42+minishell&type=repositories | Many open source 42 School implementations |

### Articles and Tutorials

| Resource | URL |
|----------|-----|
| *Writing a Unix Shell* (3-part series) | https://indradhanush.github.io/blog/writing-a-unix-shell-part-1/ |
| *Let's Build a Shell* | https://brennan.io/2015/01/16/write-a-shell-in-c/ |
| *The TTY demystified* | https://www.linusakesson.net/programming/tty/ |
| *Bash Hackers Wiki* | https://wiki.bash-hackers.org/ |
| GNU C Library manual | https://www.gnu.org/software/libc/manual/ |

### System Call Reference

```bash
man 2 fork        # process creation
man 2 execve      # program execution (execvp wraps this)
man 2 waitpid     # process synchronization
man 2 pipe        # inter-process communication
man 2 dup2        # file descriptor redirection
man 2 open        # file descriptor creation
man 2 sigaction   # signal handling
man 2 kill        # signal sending
man 3 readline    # interactive line editing
```

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

---

## License

MIT License — see [LICENSE](LICENSE).
