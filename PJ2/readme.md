# Hybrid Simulator

> Name: Yanxiang Zhu  
> Student ID: 22307130073  

This project is the final project for the *Electronic Design Automation (EDA)* course at Fudan University’s School of Microelectronics. It aims to implement parallel accelerated simulation for hybrid systems involving both analog and digital signals.

---

## Project Overview

This project develops a **multi-threaded parallel hybrid simulator with configurable thread counts**, designed to enhance simulation speed while **minimizing rollbacks**. The core design focuses on the asynchronous triggering and synchronized progression of Analog (A) events and Digital (D) events.

Key implementations include:

* A hybrid simulation framework based on the **Second-Event Synchronization (SES)** mechanism;
* Flexible thread scheduling strategies;
* Balancing simulation accuracy with parallel efficiency;
* Incorporation of critical pause control logic in `MMSimulator.c` to prevent event misalignment.

---

## File Structure

```bash
PJ2/
├── config.c / config.h          # Configuration and parsing modules
├── simulator.c / simulator.h    # A/D event generation modules
├── MMSimulator.c / MMSimulator.h# Main simulation control module (core)
├── whitebox/                    # Input configurations for 6 test cases
├── backup_BASIC_METHOD1_LockStep/
├── backup_BASIC_METHOD2_LeapFrogSES/
├── backup_ADVANCED_METHOD3/
```

### Module Descriptions

* `MMSimulator.c`: Main control logic, handling multi-threaded scheduling, initial waiting for D events, concurrent execution of A/D events, simulation merging, and rollback control.

  * Key pause control logic locations:
    * Line 50: Within `generate_A_parallel`
    * Line 64: Within `generate_A`
    * Line 78: Within `generate_D`
  * The `generate_next_A` and `generate_next_D` functions are called after each `usleep`, occurring three times in total.

* `config.c/h`: Handles reading and parsing of the configuration file `con.txt`.
* `simulator.c/h`: Manages event generation and scheduling logic, built upon the framework from the midterm assignment.

---

### Pause Logic Details

The pause logic in `MMSimulator.c` is implemented at lines 50, 64, and 78, within the `generate_A_parallel`, `generate_A`, and `generate_D` functions, respectively. The `generate_next_A` and `generate_next_D` functions are invoked only after a pause, triggered by `usleep`, with a total of three occurrences.

```c
usleep(tA * 1000);
AEvent a_parallel = generate_next_A(a, a.T, Vth);
```

```c
usleep(args->tD * 1000);
DEvent d_parallel = generate_next_D(d, d.T);
```

---

## Compilation and Execution

1. Compile using `gcc` (requires pthread support):

```bash
gcc -O2 config.c simulator.c MMSimulator.c -o MMSimulator -lpthread
```

2. Run the program:

```bash
./MMSimulator
```

* Input file: `con.txt` in the current directory
* Output file: `sim_res.txt` in the current directory

---

## Test Cases

The project includes 6 test cases located in the `whitebox/` folder. Each subfolder contains a `con.txt` file that can be copied to the main directory for testing.

---

## Historical Versions

To document the evolution of simulation strategies, three versions are preserved:

* `backup_BASIC_METHOD1_LockStep`: Basic lockstep method implementation;
* `backup_BASIC_METHOD2_LeapFrogSES`: Initial implementation of a leapfrog-style synchronization mechanism;
* `backup_ADVANCED_METHOD3`: Final parallel hybrid simulation acceleration solution.

---

## Development Environment

* System: Linux
* Compiler: gcc 7.0+
* Libraries: pthread, <stdio.h>, <stdlib.h>, <time.h>, <sys/time.h>, <unistd.h>, <math.h>

---
