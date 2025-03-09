#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <sqlite3.h>

#define MAX_PROCESSES 10
#define MS_PER_SECOND 1000
#define MAX_GANTT_WIDTH 100

/* Structure to track execution events for Gantt chart visualization
 * Records the task type, process name, and timing information
 * for each process execution segment
 */

typedef struct {
    char task_type[20];      // Type of task being executed
    char process_name[5];    // Name of the process (e.g., P1, P2)
    double start_time;       // Start time of execution segment
    double end_time;         // End time of execution segment
} ExecutionEvent;

/* Structure to maintain process information and state
 * Contains all necessary attributes for process scheduling and execution
 * including timing metrics and process identification
 */
typedef struct {
    pid_t pid;               // Process ID
    char process_name[5];    // Process name identifier
    int arrival_time;        // Time when process arrives
    double burst_time;       // Total CPU time needed
    double remaining_time;   // Remaining execution time
    double completion_time;  // Time when process completes
    double waiting_time;     // Total time spent waiting
    double turnaround_time;  // Total time in system
    char task_type[20];      // Type of task
    void (*task_function)(); // Pointer to task implementation
    int is_active;          // Flag for process state
    int first_run;          // Flag for first execution
} Process;

/* Structure for inter-process communication and control
 * Used to manage process execution and scheduling decisions
 * Shared between parent and child processes
 */
typedef struct {
    volatile sig_atomic_t should_run;  // Control flag for process execution
    volatile sig_atomic_t progress;    // Track task progress
    volatile sig_atomic_t quantum;     // Time quantum for scheduling
    int isPreemptive;                 // Flag for preemptive scheduling
} ProcessControl;

ProcessControl* process_control;

/* Signal handler for stopping processes
 * Called when a process needs to be interrupted
 */
//void handle_stop(int signum) {
 //   process_control->should_run = 0;
//}

ExecutionEvent events[1000];
int event_count = 0;

/* Records an execution event for Gantt chart visualization
 * Stores timing and process information for each execution segment
 */

void record_event(const char* task_type, const char* process_name, double start_time, double end_time) {
    if (event_count < 1000) {
        strcpy(events[event_count].task_type, task_type);
        strcpy(events[event_count].process_name, process_name);
        events[event_count].start_time = start_time;
        events[event_count].end_time = end_time;
        event_count++;
    }
}

/* Generates and prints a Gantt chart visualization
 * Shows the execution timeline of all processes
 */

void print_gantt_chart() {
    if (event_count == 0) return;

    printf("\nGantt Chart:\n\n");

    int *block_widths = (int *)malloc(event_count * sizeof(int));
    for (int i = 0; i < event_count; i++) {
        int time_digits = snprintf(NULL, 0, "%.0f", events[i].end_time);
        block_widths[i] = strlen(events[i].process_name) > time_digits ? 
                         strlen(events[i].process_name) : time_digits;
        block_widths[i] += 4;
    }

    printf(" ");
    for (int i = 0; i < event_count; i++) {
        for (int j = 0; j < block_widths[i]; j++) printf("-");
        printf(" ");
    }
    printf("\n");

    printf("|");
    for (int i = 0; i < event_count; i++) {
        int padding = (block_widths[i] - strlen(events[i].process_name)) / 2;
        int extra_pad = (block_widths[i] - strlen(events[i].process_name)) % 2;

        for (int j = 0; j < padding; j++) printf(" ");
        printf("%s", events[i].process_name);
        for (int j = 0; j < padding + extra_pad; j++) printf(" ");
        printf("|");
    }
    printf("\n");

    printf(" ");
    for (int i = 0; i < event_count; i++) {
        for (int j = 0; j < block_widths[i]; j++) printf("-");
        printf(" ");
    }
    printf("\n");

    printf("0");

    for (int i = 0; i < event_count; i++) {
        char time_str[20];
        snprintf(time_str, sizeof(time_str), "%.0f", events[i].end_time);
        int time_digits = strlen(time_str);
        int spaces = block_widths[i] + 1 - time_digits;

        for (int j = 0; j < spaces; j++) printf(" ");
        printf("%s", time_str);
    }
    printf("\n");

    free(block_widths);
}

/* Gets current system time in milliseconds
 * Used for timing and scheduling calculations
 */

double get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

/* Implements milliexplain the second-precision sleep
 * Used for controlled process delays
 */

void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}
/* Task implementation: File writing operation
 * Writes sequential lines to an output file
 */
void task_file_write() {
    FILE *fp = fopen("output.txt", "a");
    if (fp != NULL) {
        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for(int i = process_control->progress; i < 1000; i++) {

            clock_gettime(CLOCK_MONOTONIC, &current);
            double elapsed = (current.tv_sec - start.tv_sec) * 1000.0 + 
                           (current.tv_nsec - start.tv_nsec) / 1000000.0;

            if (elapsed >= process_control->quantum) {
                process_control->progress = i;
                fclose(fp);
                raise(SIGSTOP);  
                fp = fopen("output.txt", "a");
                if (fp == NULL) break;

                clock_gettime(CLOCK_MONOTONIC, &start);
            }

            fprintf(fp, "Process %d writing line %d\n", getpid(), i);
            fflush(fp);
            usleep(1000); 
        }
        fclose(fp);
    }
}

/* Task implementation: Console output operation
 * Prints sequential lines to console
 */

void task_console_echo() {
    int total_lines = 100;
    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(int i = process_control->progress; i < total_lines; i++) {

        clock_gettime(CLOCK_MONOTONIC, &current);
        double elapsed = (current.tv_sec - start.tv_sec) * 1000.0 + 
                        (current.tv_nsec - start.tv_nsec) / 1000000.0;

        if (elapsed >= process_control->quantum) {

            process_control->progress = i;
            raise(SIGSTOP);

            clock_gettime(CLOCK_MONOTONIC, &start);
        }

        printf("Process %d echoing line %d\n", getpid(), i);
        fflush(stdout);
        usleep(10000); 
    }

    process_control->progress = 0;
}
/* Task implementation: CPU-bound computation
 * Performs intensive mathematical calculations
 */
void task_compute() {
    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    long total_iterations = process_control->quantum == __DBL_MAX__ ? 50000000 : 500000000;

    long long sum = 0;
    volatile long i = process_control->progress;

    for (long j = 0; j < i; j++) {
        sum += j;
    }

    for (; i < total_iterations; i++) {
        sum += i;

        if (i % 1000 == 0) {
            clock_gettime(CLOCK_MONOTONIC, &current);
            double elapsed = (current.tv_sec - start.tv_sec) * 1000.0 + 
                           (current.tv_nsec - start.tv_nsec) / 1000000.0;

            if (elapsed >= process_control->quantum) {

                process_control->progress = i + 1;  
                printf("Process %d suspended at iteration %ld, current sum: %lld\n", 
                       getpid(), i, sum);
                fflush(stdout);
                if (!process_control->isPreemptive){
                    exit(0);
                    break;
                }
                raise(SIGSTOP);

                clock_gettime(CLOCK_MONOTONIC, &start);
            }
        }

        if (i % 10000000 == 0) {
            printf("Process %d computed sum up to %ld: %lld\n", getpid(), i, sum);
            fflush(stdout);
        }
    }

    printf("Process %d completed computation. Final sum: %lld\n", getpid(), sum);
    fflush(stdout);
}
/* Task implementation: Database write operation
 * Performs sequential database record insertions
 */
void task_db_write() {
    int total_records = 90;
    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(int i = process_control->progress; i < total_records; i++) {

        clock_gettime(CLOCK_MONOTONIC, &current);
        double elapsed = (current.tv_sec - start.tv_sec) * 1000.0 + 
                        (current.tv_nsec - start.tv_nsec) / 1000000.0;

        if (elapsed >= process_control->quantum) {

            process_control->progress = i;
            raise(SIGSTOP);

            clock_gettime(CLOCK_MONOTONIC, &start);
        }

        printf("Process %d adding Record %d\n", getpid(), i);

    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    pid_t pid;

    rc = sqlite3_open("os_project.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char *create_table_sql = 
        "CREATE TABLE IF NOT EXISTS student ("
        "id INTEGER, "
        "name TEXT, "
        "age INTEGER);";

    rc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    srand(time(NULL));

    pid = getpid();

    sqlite3_stmt *stmt;
    const char *insert_sql = "INSERT INTO student (id, name, age) VALUES (?, ?, ?);";
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char name[50];
    snprintf(name, sizeof(name), "name_%d", pid);

    int age = 10 + rand() % 11;

    sqlite3_bind_int(stmt, 1, i);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, age);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insertion failed: %s\n", sqlite3_errmsg(db));
    } 

    sqlite3_finalize(stmt);
    sqlite3_close(db);

        usleep(10000); 
    }

    process_control->progress = 0;
}
/* Measures actual burst time for a given task
 * Creates test process and measures execution time
 */

double measure_burst_time(void (*task_function)()) {
    double start_time = get_current_time();

    pid_t pid = fork();
    if (pid == 0) {
        task_function();
        exit(0);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        double end_time = get_current_time();
        return (end_time - start_time);
    }
    return 0;
}
/* Initializes a process structure with given parameters
 * Sets up process attributes and measures burst time
 */

void initialize_process(Process* p, void (*task_function)(), const char* task_name, 
                       int arrival_time_ms, int process_num) {
    p->task_function = task_function;
    strcpy(p->task_type, task_name);
    sprintf(p->process_name, "P%d", process_num);  
    p->arrival_time = arrival_time_ms;

    printf("Measuring burst time for %s (%s)...\n", p->process_name, task_name);
    p->burst_time = measure_burst_time(task_function);
    p->remaining_time = p->burst_time;
    p->completion_time = 0;
    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->pid = 0;
    p->is_active = 0;
    p->first_run = 1;

    printf("Measured burst time for %s (%s): %.2f ms\n", p->process_name, task_name, p->burst_time);
}
/* Creates a new process using fork()
 * Initializes child process for task execution
 */

void create_process(Process* p) {
    pid_t pid = fork();

    if (pid == 0) {
        p->task_function();
        exit(0);
    } else if (pid > 0) {
        p->pid = pid;
    }
}
/* Implements First-Come-First-Serve scheduling algorithm
 * Executes processes in order of arrival
 */
void fcfs(Process processes[], int n) {
    printf("\nExecuting FCFS Scheduling...\n");
    event_count = 0;

    process_control = mmap(NULL, sizeof(ProcessControl), 
                         PROT_READ | PROT_WRITE, 
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (process_control == MAP_FAILED) {
        perror("mmap failed");
        return;
    }

    double current_time = 0;
    for (int i = 0; i < n; i++) {

        if (current_time < processes[i].arrival_time) {
            sleep_ms(processes[i].arrival_time - current_time);
            current_time = processes[i].arrival_time;
        }

        if (processes[i].first_run) {
            processes[i].first_run = 0;
        }

        printf("Starting %s at time %.2f ms\n", processes[i].process_name, current_time);

        process_control->should_run = 1;
        process_control->progress = 0;
        process_control->quantum = processes[i].burst_time;  
        process_control->isPreemptive=0;

        double start_time = current_time;
        create_process(&processes[i]);

        int status;
        struct timespec start_ts, current_ts;
        clock_gettime(CLOCK_MONOTONIC, &start_ts);

        while (1) {
            pid_t result = waitpid(processes[i].pid, &status, WNOHANG);
            if (result > 0) break;  

            clock_gettime(CLOCK_MONOTONIC, &current_ts);
            usleep(1000);  
        }

        clock_gettime(CLOCK_MONOTONIC, &current_ts);
        double elapsed = (current_ts.tv_sec - start_ts.tv_sec) * 1000.0 + 
                        (current_ts.tv_nsec - start_ts.tv_nsec) / 1000000.0;

        current_time += elapsed;
        record_event(processes[i].task_type, processes[i].process_name, start_time, current_time);

        processes[i].completion_time = current_time;
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;

        printf("Completed %s at time %.2f ms\n", processes[i].process_name, current_time);
    }

    munmap(process_control, sizeof(ProcessControl));
}
/* Implements Round Robin scheduling algorithm
 * Executes processes with time quantum-based preemption
 */
void round_robin(Process processes[], int n, int time_quantum_ms) {
    printf("\nExecuting Round Robin Scheduling (Time Quantum: %d ms)...\n", time_quantum_ms);
    event_count = 0;

    process_control = mmap(NULL, sizeof(ProcessControl), 
                         PROT_READ | PROT_WRITE, 
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (process_control == MAP_FAILED) {
        perror("mmap failed");
        return;
    }

    process_control->should_run = 1;
    process_control->quantum = time_quantum_ms;
    process_control->isPreemptive=1;

    double current_time = 0;
    int completed = 0;
    int* terminated = (int*)calloc(n, sizeof(int));

    double* first_execution_time = (double*)calloc(n, sizeof(double));
    for (int i = 0; i < n; i++) {
        first_execution_time[i] = -1;
    }

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSTOP, &sa, NULL);

    while (completed < n) {
        int work_done = 0;

        for (int i = 0; i < n; i++) {

            if (terminated[i] || processes[i].arrival_time > current_time) {
                continue;
            }

            if (processes[i].remaining_time <= 0) {
                if (!terminated[i]) {
                    terminated[i] = 1;
                    completed++;
                    processes[i].completion_time = current_time;
                }
                continue;
            }

            work_done = 1;

            if (first_execution_time[i] == -1) {
                first_execution_time[i] = current_time;
            }

            if (processes[i].pid == 0) {
                if (processes[i].first_run) {
                    processes[i].first_run = 0;
                    process_control->progress = 0;
                }

                pid_t pid = fork();
                if (pid == 0) {
                    struct sigaction child_sa;
                    child_sa.sa_handler = SIG_DFL;
                    sigemptyset(&child_sa.sa_mask);
                    child_sa.sa_flags = 0;
                    sigaction(SIGSTOP, &child_sa, NULL);

                    processes[i].task_function();
                    exit(0);
                } else if (pid > 0) {
                    processes[i].pid = pid;
                }
            }

            double exec_time = (processes[i].remaining_time > time_quantum_ms) ? 
                              time_quantum_ms : processes[i].remaining_time;

            printf("Executing %s for %.2f ms at %.2f (Progress: %d)\n", 
                   processes[i].process_name, exec_time, current_time, 
                   process_control->progress);

            double start_time = current_time;
            struct timespec start, now;
            clock_gettime(CLOCK_MONOTONIC, &start);

            kill(processes[i].pid, SIGCONT);

            int process_stopped = 0;
            while (!process_stopped) {
                clock_gettime(CLOCK_MONOTONIC, &now);
                double elapsed = (now.tv_sec - start.tv_sec) * 1000.0 + 
                               (now.tv_nsec - start.tv_nsec) / 1000000.0;

                if (elapsed >= exec_time) {
                    kill(processes[i].pid, SIGSTOP);
                    process_stopped = 1;
                }

                int status;
                pid_t result = waitpid(processes[i].pid, &status, WNOHANG | WUNTRACED);
                if (result > 0) {
                    if (WIFSTOPPED(status)) {
                        process_stopped = 1;
                        elapsed = elapsed > exec_time ? exec_time : elapsed;
                    } else if (WIFEXITED(status)) {
                        process_stopped = 1;
                        processes[i].remaining_time = 0;
                        terminated[i] = 1;
                        completed++;
                        processes[i].completion_time = current_time + elapsed;
                    }
                }

                if (!process_stopped) {
                    usleep(1000);
                }
            }

            double actual_exec_time = (now.tv_sec - start.tv_sec) * 1000.0 + 
                                    (now.tv_nsec - start.tv_nsec) / 1000000.0;
            actual_exec_time = actual_exec_time > exec_time ? exec_time : actual_exec_time;

            current_time += actual_exec_time;
            processes[i].remaining_time -= actual_exec_time;

            record_event(processes[i].task_type, processes[i].process_name, 
                        start_time, current_time);
        }

        if (!work_done) {
            double next_arrival = __DBL_MAX__;
            for (int i = 0; i < n; i++) {
                if (!terminated[i] && processes[i].arrival_time > current_time) {
                    next_arrival = (next_arrival < processes[i].arrival_time) ? 
                                  next_arrival : processes[i].arrival_time;
                }
            }
            if (next_arrival != __DBL_MAX__) {
                current_time = next_arrival;
            }
        }
    }

    for (int i = 0; i < n; i++) {

        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;

        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
    }

    free(first_execution_time);
    free(terminated);
    munmap(process_control, sizeof(ProcessControl));
}
/* Implements Shortest Job First scheduling algorithm
 * Executes processes ordered by burst time
 */
void sjf(Process processes[], int n) {
    printf("\nExecuting Shortest Job First Scheduling...\n");
    event_count = 0;

    process_control = mmap(NULL, sizeof(ProcessControl), 
                         PROT_READ | PROT_WRITE, 
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (process_control == MAP_FAILED) {
        perror("mmap failed");
        return;
    }

    double current_time = 0;
    int completed = 0;
    int* completed_processes = (int*)calloc(n, sizeof(int));

    while (completed < n) {
        int shortest = -1;
        double shortest_burst = __DBL_MAX__;

        for (int i = 0; i < n; i++) {
            if (!completed_processes[i] && 
                processes[i].arrival_time <= current_time && 
                processes[i].burst_time < shortest_burst) {
                shortest_burst = processes[i].burst_time;
                shortest = i;
            }
        }

        if (shortest == -1) {
            double next_arrival = __DBL_MAX__;
            for (int i = 0; i < n; i++) {
                if (!completed_processes[i] && processes[i].arrival_time > current_time) {
                    if (processes[i].arrival_time < next_arrival) {
                        next_arrival = processes[i].arrival_time;
                    }
                }
            }
            if (next_arrival != __DBL_MAX__) {
                sleep_ms(next_arrival - current_time);
                current_time = next_arrival;
            }
            continue;
        }

        printf("Starting %s at time %.2f ms\n", 
               processes[shortest].process_name, current_time);

        if (processes[shortest].first_run) {
            processes[shortest].first_run = 0;
        }

        process_control->should_run = 1;
        process_control->progress = 0;
        process_control->quantum = processes[shortest].burst_time;  
        process_control->isPreemptive=0;

        double start_time = current_time;
        create_process(&processes[shortest]);

        int status;
        struct timespec start_ts, current_ts;
        clock_gettime(CLOCK_MONOTONIC, &start_ts);

        while (1) {
            pid_t result = waitpid(processes[shortest].pid, &status, WNOHANG);
            if (result > 0) break;  

            clock_gettime(CLOCK_MONOTONIC, &current_ts);
            usleep(1000);  
        }

        clock_gettime(CLOCK_MONOTONIC, &current_ts);
        double elapsed = (current_ts.tv_sec - start_ts.tv_sec) * 1000.0 + 
                        (current_ts.tv_nsec - start_ts.tv_nsec) / 1000000.0;

        current_time += elapsed;
        record_event(processes[shortest].task_type, 
                    processes[shortest].process_name, 
                    start_time, current_time);

        processes[shortest].completion_time = current_time;
        processes[shortest].turnaround_time = processes[shortest].completion_time - 
                                            processes[shortest].arrival_time;
        processes[shortest].waiting_time = processes[shortest].turnaround_time - 
                                         processes[shortest].burst_time;

        completed_processes[shortest] = 1;
        completed++;

        printf("Completed %s at time %.2f ms\n", 
               processes[shortest].process_name, current_time);
    }

    free(completed_processes);
    munmap(process_control, sizeof(ProcessControl));
}
/* Prints detailed statistics for all processes
 * Shows timing metrics and generates Gantt chart
 */
void print_stats(Process processes[], int n) {
    printf("\nProcess Statistics:\n");
    printf("%-8s %-8s %-12s %-12s %-12s %-12s %-12s %-12s\n",
           "Process", "PID", "Task", "Arrival", "Burst", "Completion", "Turnaround", "Waiting");
    printf("---------------------------------------------------------------------------------------\n");

    double avg_turnaround = 0, avg_waiting = 0;

    for (int i = 0; i < n; i++) {
        if (processes[i].waiting_time<0){
            processes[i].waiting_time=0;
        }
        printf("%-8s %-8d %-12s %-12d %-12.2f %-12.2f %-12.2f %-12.2f\n",
               processes[i].process_name,
               processes[i].pid,
               processes[i].task_type,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);

        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
    }

    printf("\nAverage Metrics:\n");
    printf("Turnaround Time: %.2f ms\n", avg_turnaround/n);
    printf("Waiting Time: %.2f ms\n", avg_waiting/n);

    print_gantt_chart();
}
/* Handles process type selection and initialization
 * Provides user interface for process configuration
 */

void select_process(Process* p, int index) {
    printf("\nAvailable process types for P%d:\n", index + 1);
    printf("1. File Write\n");
    printf("2. Console Echo\n");
    printf("3. Compute\n");
    printf("4. Add record to database\n"); 

    int choice;
    printf("Select process type (1-4): ");
    scanf("%d", &choice);

    int arrival_time_ms;
    printf("Enter arrival time (in milliseconds): ");
    scanf("%d", &arrival_time_ms);

    switch(choice) {
        case 1:
            initialize_process(p, task_file_write, "file_write", arrival_time_ms, index + 1);
            break;
        case 2:
            initialize_process(p, task_console_echo, "console_echo", arrival_time_ms, index + 1);
            break;
        case 3:
            initialize_process(p, task_compute, "compute", arrival_time_ms, index + 1);
            break;
       case 4:  
            initialize_process(p, task_db_write, "db_write", arrival_time_ms, index + 1);
            break;
        default:
            printf("Invalid choice! Defaulting to compute process.\n");
            initialize_process(p, task_compute, "compute", arrival_time_ms, index + 1);
    }
}
/* Sorts processes array by arrival time
 * Used to order processes for scheduling
 */
void sort_processes_by_arrival_time(Process processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j+1].arrival_time) {

                Process temp = processes[j];
                processes[j] = processes[j+1];
                processes[j+1] = temp;
            }
        }
    }
}

int main() {
    int n;
    printf("Enter number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &n);

    if(n <= 0 || n > MAX_PROCESSES) {
        printf("Invalid number of processes!\n");
        return 1;
    }

    Process processes[MAX_PROCESSES];

    printf("\nSelect process types and arrival times:\n");
    for(int i = 0; i < n; i++) {
        select_process(&processes[i], i);
    }

    printf("\nInitial Process Set:\n");
    printf("Process\tTask Type\tArrival Time (ms)\tMeasured Burst Time (ms)\n");
    for(int i = 0; i < n; i++) {
        printf("%s\t%s\t\t%d\t\t\t%.2f\n", 
               processes[i].process_name,
               processes[i].task_type,
               processes[i].arrival_time,
               processes[i].burst_time);
    }

    printf("\nChoose scheduling algorithm:\n");
    printf("1. First Come First Serve (FCFS)\n");
    printf("2. Round Robin (RR)\n");
    printf("3. Shortest Job First (SJF)\n");

    int choice;
    scanf("%d", &choice);

    switch(choice) {
        case 1: {
            Process fcfs_processes[MAX_PROCESSES];
            memcpy(fcfs_processes, processes, sizeof(Process) * n);
            sort_processes_by_arrival_time(fcfs_processes, n);  
            fcfs(fcfs_processes, n);
            print_stats(fcfs_processes, n);
            break;
        }
        case 2: {
            int time_quantum_ms;
            int valid=0;
            while (!valid){
            printf("Enter time quantum (in milliseconds): ");
            scanf("%d", &time_quantum_ms);
            if (time_quantum_ms>0)
            valid=1;
            else
            printf("Invalid time quantum.\n");
            }
            Process rr_processes[MAX_PROCESSES];
            memcpy(rr_processes, processes, sizeof(Process) * n);
            sort_processes_by_arrival_time(rr_processes, n);  
            round_robin(rr_processes, n, time_quantum_ms);
            print_stats(rr_processes, n);
            break;
        }
        case 3: {
            Process sjf_processes[MAX_PROCESSES];
            memcpy(sjf_processes, processes, sizeof(Process) * n);
            sort_processes_by_arrival_time(sjf_processes, n);  
            sjf(sjf_processes, n);
            print_stats(sjf_processes, n);
            break;
        }
        default:
            printf("Invalid choice!\n");
            return 1;
    }

    return 0;
}