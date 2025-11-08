# SYSC4001 – Assignment 2 Part II  

### Team Information  
**Language:** C++  
**Team Members:** Gabriel Bugarija and Mithushan Ravichandramohan  

---

## Overview  
This project implements two concurrent Unix processes in C++ using the system calls `fork()`, `exec()`, `wait()`, and System V shared memory with semaphores.  

The goal is to demonstrate process creation, execution control, synchronization, and inter-process communication as outlined in Part II of the SYSC4001 assignment.  

The implementation progresses through the following stages:  
1. Creation of two concurrent processes using `fork()`.  
2. Replacement of the child process image using `exec()`.  
3. Synchronization using `wait()`.  
4. Shared memory communication (`shmget`, `shmat`, `shmdt`, `shmctl`).  
5. Protection of shared memory using semaphores (`semget`, `semop`, `semctl`).  

---

## Project Structure
- Makefile # Build instructions
- Process1.cpp # Parent process (creates and manages shared memory)
- Process2.cpp # Child process (exec'd from Process1 and interacts with shared memory)
- Process1 # Compiled executable
- README.md # Project documentation

---

## Compilation and Execution  

### Build
To compile both programs, run:
```bash
make

This command will generate the executables for Process1 and Process2.

To start the main process: ./Process1
To terminate both processes: kill <PID1> <PID2>

