# Command Argument Passing System

## Objective

This project is a beginner-friendly Operating Systems lab demonstration of how a shell accepts a command and its arguments, creates a child process, replaces the child process image with another program, and waits for that child to finish.

## Problem Statement

Build a small interactive command shell that reads a command line, separates the command from its arguments, and executes it in a child process. The project must make the complete argument list visible to the newly executed program and show how the parent synchronizes with the child.

## Technologies Used

- C (C11 source with POSIX/Linux system calls)
- GCC
- GNU Make
- Linux, Unix, or another POSIX environment

`fork()`, `execvp()`, and `waitpid()` require a Linux/Unix/POSIX environment. On Windows, compile and run this project inside WSL or another Linux virtual machine; the code is intentionally not rewritten with Windows APIs.

## System Calls and Functions Used

- `fgets()` reads a complete input line safely into a fixed-size buffer.
- `strtok()` separates the command line into whitespace-delimited arguments.
- `fork()` creates a child process.
- `execvp()` replaces the child process with the requested program and passes its `argv` array.
- `waitpid()` makes the parent wait for its specific child.
- `WIFEXITED()` and `WEXITSTATUS()` inspect normal child termination.
- `WIFSIGNALED()` and `WTERMSIG()` inspect termination by a signal.

## Project Structure

```text
arg_passing_system/
├── arg_executor.c
├── arg_printer.c
├── Makefile
└── README.md
```

## How the Programs Work

### `arg_executor.c`

`arg_executor` is the interactive mini-shell. It displays the `argsh>` prompt, reads up to 1023 characters with `fgets()`, and uses `strtok()` to create a NULL-terminated array containing at most 63 arguments plus the required NULL pointer.

For each command, it calls `fork()`. The child calls `execvp(arguments[0], arguments)`, which searches for the command in `PATH` and passes the entire argument array. If execution fails, the child prints an error and exits with `EXIT_FAILURE`. The parent prints the child PID, calls `waitpid()`, reports the result, and then displays the prompt again.

The built-in commands `exit` and `quit` terminate the shell without creating a child. Empty lines are ignored.

### `arg_printer.c`

`arg_printer` is a helper program designed to make argument passing easy to observe. It receives `argc` and `argv` in `main()`, prints the number of arguments, and loops over every element, including `argv[0]`, which is the program name.

For example, `./arg_printer hello world 42` displays the command name followed by the three supplied arguments.

## Process Concepts

### Parent and Child Processes

The shell is initially the parent process. `fork()` creates a nearly identical child process. Both processes continue from the point after `fork()`, but the return value identifies each role:

- A return value less than zero means creation failed.
- A return value of zero identifies the child.
- A positive return value is the child PID seen by the parent.

### `fork()` and `execvp()`

`fork()` creates the process. In the child, `execvp()` then replaces the child's program image with the requested command. The child keeps the argument values supplied through the `argv` array, while its running program becomes the requested executable.

If `execvp()` succeeds, it does not return. If it returns, execution failed, so the child reports the error and terminates.

### `waitpid()` and Synchronization

The parent calls `waitpid(child_pid, &wait_status, 0)` after forking. This synchronizes the parent with that particular child and prevents the shell from accepting the next command until the current command has completed. The parent then checks whether the child exited normally or was terminated by a signal.

### `argc` and `argv`

`argc` is the number of command-line arguments. `argv` is an array of strings:

- `argv[0]` is normally the program name.
- `argv[1]` through `argv[argc - 1]` are the supplied arguments.
- `argv[argc]` is `NULL`.

## Command Execution Journey

```text
USER enters command + arguments
              |
              v
        arg_executor
              |
           fork()
              |
              v
        CHILD PROCESS
              |
     execvp(command, argv)
              |
              v
        NEW PROGRAM
              |
       receives argc/argv
              |
              v
         arg_printer
              |
       prints arguments
              |
       child terminates
              |
           waitpid()
              |
              v
       PARENT CONTINUES
```

This demonstrates the shell role, the command execution journey, user-space process code requesting kernel-managed process operations, process creation, execution, synchronization, and termination.

## Compilation

From this directory, run:

```sh
make
```

The command creates these executables:

```text
arg_executor
arg_printer
```

To remove the executables:

```sh
make clean
```

## Execution

Start the mini-shell with:

```sh
./arg_executor
```

At the `argsh>` prompt, enter commands such as:

```text
./arg_printer hello world 42
./arg_printer Apple 100 Linux OS
echo Hello
ls -la
date
```

You can also run the helper directly:

```sh
./arg_printer hello world 42
```

Expected helper output:

```text
=== arg_printer: Received Arguments ===
argc = 4
argv[0] = ./arg_printer
argv[1] = hello
argv[2] = world
argv[3] = 42
========================================
```

A typical shell execution includes messages like:

```text
[Parent] Forking child to execute: ./arg_printer
[Parent] Child PID: 12345
=== arg_printer: Received Arguments ===
argc = 4
argv[0] = ./arg_printer
argv[1] = hello
argv[2] = world
argv[3] = 42
========================================
[Parent] Child (PID 12345) exited with status 0
```

The PID varies on every run.

## Testing Procedure

1. Build with `make` and confirm both executables are created.
2. Run `./arg_printer hello world 42` to verify direct argument reception.
3. Run `./arg_executor`.
4. Enter `./arg_printer hello world 42` and confirm every `argv` element is printed.
5. Enter `./arg_printer Apple 100 Linux OS` to verify a different argument list.
6. Enter `echo Hello`, `ls -la`, and `date` to verify different commands.
7. Press Enter on an empty line and confirm the prompt returns.
8. Enter `invalidcommand test`. The child should report an `exec` error and the parent should continue.
9. Enter `ls /nonexistent`. The parent should report a non-zero child exit status.
10. Enter `exit`, then repeat with `quit`, to verify both built-in termination commands.

For a non-interactive smoke test, use:

```sh
printf '%s\n' './arg_printer hello world 42' 'invalidcommand test' 'ls /nonexistent' 'exit' | ./arg_executor
```

## Invalid Commands and Failure Status

An unknown command causes `execvp()` to fail in the child. The child prints a clear error and exits with `EXIT_FAILURE`; this does not terminate the parent shell. A valid command that returns an error, such as `ls /nonexistent`, is reported by the parent through its non-zero exit status.

## Conclusion

The project demonstrates the basic process lifecycle used by a command shell: read a command, parse its arguments, create a child with `fork()`, execute a new program with `execvp()`, pass `argc/argv`, wait with `waitpid()`, inspect termination, and continue in the parent. It directly supports CO-1 topics about shell operation and system calls and CO-2 topics about process creation, execution, synchronization, and termination.
