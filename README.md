
# Real-Time Process Scheduling Simulator

## Overview

This project simulates three fundamental CPU scheduling algorithms:

- **First Come First Serve (FCFS)**
- **Round Robin (RR)**
- **Shortest Job First (SJF)**

The simulation provides:

- Interactive setup of processes with different task types
- Real-time execution tracking
- Performance metric calculation (Turnaround Time, Waiting Time)
- Gantt chart visualization of process execution
- Real-world system calls integration (`mmap`, `fork`, `waitpid`, `signal`)

## Features

- Interactive process configuration  
- Task variety reflecting real-world workloads (File I/O, Console, Computation, Database)  
- Accurate burst time measurement  
- Preemptive and non-preemptive scheduling support  
- Shared memory management using `mmap`  
- Detailed performance statistics  
- Visual Gantt chart generation  

## Task Types

| Task Type       | Description                         |
|-----------------|-------------------------------------|
| File Write      | Writes lines to an output file      |
| Console Echo    | Prints lines to console             |
| Compute         | Performs CPU-bound calculations     |
| Database Write  | Inserts records into SQLite database|

## Scheduling Algorithms

- **FCFS (First Come First Serve):**
  - Executes processes in order of arrival
  - Non-preemptive
  - Susceptible to convoy effect

- **RR (Round Robin):**
  - Allocates CPU time slices (quantum) to processes
  - Preemptive
  - Enhances fairness and responsiveness

- **SJF (Shortest Job First):**
  - Executes process with the shortest burst time first
  - Non-preemptive
  - Optimizes average turnaround time

## How It Works

1. **User Input:**
   - Number of processes (max 10)
   - For each process:
     - Select task type
     - Provide arrival time

2. **Burst Time Estimation:**
   - Burst time is dynamically measured based on task execution

3. **Scheduling Selection:**
   - Choose between FCFS, RR (with time quantum), or SJF

4. **Process Simulation:**
   - Real-time process execution with shared memory management
   - Metrics tracked:
     - Arrival Time
     - Burst Time
     - Completion Time
     - Turnaround Time
     - Waiting Time

5. **Visualization:**
   - Gantt chart shows process execution timeline

## System Requirements

- GCC-compatible C compiler
- POSIX-compliant operating system (Linux recommended)
- SQLite3 library

## Compilation & Execution

### Compile:

```bash
gcc -o scheduler scheduler.c -lsqlite3 -lrt
```

### Run:

```bash
./scheduler
```

Follow the on-screen instructions to:

- Configure processes  
- Select scheduling algorithm  
- View execution statistics and Gantt chart  

## Example Output

```
Enter number of processes (max 10): 3

Available process types for P1:
1. File Write
2. Console Echo
3. Compute
4. Add record to database
Select process type (1-4): 2
Enter arrival time (in milliseconds): 0

... (similar for other processes)

Choose scheduling algorithm:
1. First Come First Serve (FCFS)
2. Round Robin (RR)
3. Shortest Job First (SJF)
```

After execution, the program displays:

- Process Statistics Table
- Average Turnaround and Waiting Times
- Gantt Chart

## Technologies Used

- C Programming
- Process management (`fork`, `waitpid`)
- Shared memory with `mmap`
- Signal handling
- SQLite3 database operations

## References

- Silberschatz, A., Galvin, P. B., & Gagne, G. *Operating System Concepts*, Wiley, 2018  
- Stallings, W. *Operating Systems: Internals and Design Principles*, Pearson, 2018  
- Tanenbaum, A. S., & Bos, H. *Modern Operating Systems*, Pearson, 2015  
- Linux Man Pages: `mmap`, `fork`, `waitpid`
