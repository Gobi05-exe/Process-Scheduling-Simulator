
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
Enter number of processes (max 10): 4

Select process types and arrival times:

Available process types for P1:
1. File Write
2. Console Echo
3. Compute
4. Add record to database
Select process type (1-4): 1
Enter arrival time (in milliseconds): 200
Measuring burst time for P1 (file_write)...
Measured burst time for P1 (file_write): 1243.20 ms

Available process types for P2:
1. File Write
2. Console Echo
3. Compute
4. Add record to database
Select process type (1-4): 2
Enter arrival time (in milliseconds): 0
Measuring burst time for P2 (console_echo)...
Measured burst time for P2 (console_echo): 1247.65 ms

Available process types for P3:
1. File Write
2. Console Echo
3. Compute
4. Add record to database
Select process type (1-4): 3
Enter arrival time (in milliseconds): 100
Measuring burst time for P3 (compute)...
Measured burst time for P3 (compute): 1247.39 ms

Available process types for P4:
1. File Write
2. Console Echo
3. Compute
4. Add record to database
Select process type (1-4): 4
Enter arrival time (in milliseconds): 300
Measuring burst time for P4 (db_write)...
Measured burst time for P4 (db_write): 1251.45 ms

Initial Process Set:
Process Task Type   Arrival Time (ms)   Measured Burst Time (ms)
P1      file_write        200               1243.20
P2      console_echo      0                 1247.65
P3      compute           100               1247.39
P4      db_write          300               1251.45

Choose scheduling algorithm:
1. First Come First Serve (FCFS)
2. Round Robin (RR)
3. Shortest Job First (SJF)
3

Executing Shortest Job First Scheduling...
Starting P2 at time 0.00 ms
Process 5539 echoing line 0
Process 5539 echoing line 1
...
Process 5539 echoing line 99
Completed P2 at time 1019.90 ms

Starting P1 at time 1019.90 ms
Process 5540 writing line 0
...
Completed P1 at time 2115.16 ms

Starting P3 at time 2115.16 ms
Process 5559 computed sum up to 10000000: 50000005000000
...
Process 5559 computed sum up to 2000000000: 2000000100000000000
Process 5559 suspended at iteration 205775000, current sum: 2117167541538750
Completed P3 at time 3363.58 ms

Starting P4 at time 3363.58 ms
Process 5560 adding Record 0
Process 5560 adding Record 1
...
Process 5560 adding Record 89
Completed P4 at time 4476.73 ms

Process Statistics:
Process  PID   Task         Arrival   Burst     Completion   Turnaround   Waiting
P2      5539  console_echo   0        1247.65    1019.90     1019.90      0.00
P3      5559  compute        100      1247.39    3363.58     3263.58      2016.18
P1      5540  file_write     200      1243.20    2115.16     1915.16      671.97
P4      5560  db_write       300      1251.45    4476.73     4176.73      2925.28

Average Metrics:
Turnaround Time: 2593.84 ms
Waiting Time: 1403.36 ms

Gantt Chart:
  ------------ ------------ ------------ ------------ 
 |     P2     |     P1     |     P3     |     P4     |
  ------------ ------------ ------------ ------------ 
0           1019         2115         3364         4477

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
