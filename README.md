# Mini Shell (msh)

A minimalistic Linux shell implemented in C using Linux system calls and POSIX APIs.

The project was developed to understand process creation, process synchronization, signal handling, exit status, command parsing, background processes, job control and inter-process communication using pipes.

---

## Objective

The objective of this project is to understand and implement the fundamental concepts involved in a Linux shell, including:

- Process creation using `fork()`
- Program execution using `execvp()`
- Process synchronization using `waitpid()`
- Signal handling using `sigaction()`
- Exit status handling
- Command-line parsing
- Background process execution
- Job control
- Inter-process communication using pipes
- Environment variables and shell special variables

---

## Features

### 1. Shell Prompt

Default prompt:

```text
msh>