#include "pcb.h"

// Scheduler responsibilities:
// implement HPF, SRTN, RR
// 1-start new process
// 2-Switch between two processes according to the scheduling algorithm. (Stop the old process and save its state and start/resume another one.)
// 3-Keep a process control block (PCB) for each process in the system. A PCB should keep track of the state of a process; running/waiting, execution time, remaining time, waiting time, etc
// 4-Delete the data of a process when it gets notifies that it finished. When a process finishes it should notify the scheduler on termination, the scheduler does NOT terminate the process.
// 4- Report:  cpu utilization, AVG waiting time,AVG weighted turnaround time, and Standard deviation for average weighted turnaround time.
// 5- generate 2 output files
FILE *log_file;
FILE *perf_file;
struct process_data *running_process;
struct process_data *received_process;
struct process_table pt;
struct process_data process;
struct pcb *running_pcb;
double AVG_WTA, AVG_Waiting, STD_WTA, SUM_WTA, SUM_Waiting, SUM_burst_time;
process_data *receive_process(process_data process);
void create_child_process(process_data *p, process_table *pt);
void log_process_data(pcb *p);
void log_process_term(pcb *p);
void perf_process_final();
void add_Sums(pcb *p);
void HPF_Scheduler();
void HPF(pri_queue *processes);
int process_num, curr_process_num, algorithm;

int main(int argc, char *argv[])
{
    initClk();
    if (argc < 3)
    {
        perror("Incorrect usage, to run use: ./bin/scheduler.out <process_num> <algorithm>\n");
        exit(1);
    }
    algorithm = atoi(argv[1]);
    process_num = atoi(argv[2]);
    if (algorithm == 1)
    {
        HPF_Scheduler();
    }
    else if (algorithm == 2)
    {
        // SRTN_Scheduler();
    }
    else if (algorithm == 3)
    {
        // RR_Scheduler();
    }
    else
    {
        perror("Incorrect usage, to run use: ./bin/scheduler.out <process_num> <algorithm>\n");
        exit(1);
    }
    destroyClk(true);
}
void log_process_data(pcb *p)
{
    // log process data to log file
    log_file = fopen("scheduler.log", "a");
    if (!log_file)
    {
        perror("Error in opening log file");
        exit(-1);
    }
    fprintf(log_file, "At time %d process %d ", getClk(), p->id);
    switch (p->state)
    {
    case 1: // started
        fprintf(log_file, "started ");
        p->start_time = getClk();
    case 2: // stopped
        fprintf(log_file, "stopped ");
    case 3: // Resumed
        fprintf(log_file, "resumed ");
    case 4: // terminated
        fprintf(log_file, "terminated ");
    default:
        break;
    }
    fprintf(log_file, "arr %d total %d remain %d wait %d\n", p->arrival_time, p->total_time, p->remaining_time, p->waiting_time);
    fclose(log_file);
}
void log_process_term(pcb *p)
{
    // log process termination to log file
    log_file = fopen("scheduler.log", "a");
    if (!log_file)
    {
        perror("Error in opening log file");
        exit(-1);
    }
    fprintf(log_file, "At time %d process %d ", getClk(), p->id);
    fprintf(log_file, "terminated ");
    p->total_time =
        p->TA = getClk() - p->arrival_time;
    p->WTA = (float)p->TA / p->total_time;
    fprintf(log_file, "arr %d total %d remain %d wait %d TA %d WTA %.2f \n", p->arrival_time, p->total_time, p->remaining_time, p->waiting_time, p->TA, p->WTA);
    fclose(log_file);
}
void add_Sums(pcb *p)
{
    SUM_WTA += p->WTA;
    SUM_Waiting += p->waiting_time;
    SUM_burst_time += p->running_time;
}
void perf_process_final()
{
    // log process termination to log file
    perf_file = fopen("scheduler.perf", "a");
    if (!perf_file)
    {
        perror("Error in opening log file");
        exit(-1);
    }
    AVG_WTA = SUM_WTA / process_num;
    AVG_Waiting = SUM_Waiting / process_num;
    fprintf(perf_file, "CPU utilization = %.2f%%\n", (double)(SUM_burst_time / getClk()) * 100);
    fprintf(perf_file, "Avg WTA = %.2f\n", AVG_WTA);
    fprintf(perf_file, "Avg Waiting = %.2f\n", AVG_Waiting);
    fclose(perf_file);
}
key_t getMessageQueue()
{
    // get the message queue
    key_t msgq_id = msgget(MSGKEY, 0666 | IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    return msgq_id;
}

process_data *receive_process(process_data process)
{
    // Receive Process from the message queue
    struct msgbuff *buf = malloc(sizeof(struct msgbuff));
    // memset(buf, 0, sizeof(struct msgbuff));
    key_t msgq_id = getMessageQueue();
    // receive process from the message queue
    int sendVal = 0;
    if ((sendVal = msgrcv(msgq_id, buf, sizeof(*buf), 1, IPC_NOWAIT)) == -1)
    {
        free(buf);
        return NULL;
    }
    if (!buf)
        return NULL;
    // since I send and Receive as a process_data object
    process = buf->data;
    process_data *p = malloc(sizeof(*p));
    p = &process;
    create_child_process(p, &pt);
    return p;
}

void create_child_process(process_data *p, process_table *pt)
{
    // create a child process (process.c)
    int process_child = fork();
    if (process_child == -1)
    {
        perror("Failed to fork child");
        exit(EXIT_FAILURE);
    }
    else if (process_child == 0)
    { // child :process.c
        char *arr = malloc(sizeof(*arr));
        sprintf(arr, "%d", p->running_time);
        execl("./process.out", "./process.out", arr, NULL);
    }
    else
    {
        // scheduler.c
        // send SIGSTOP signal to the process until it runs and change its id to forked id because when It runs I can send SIGCONT to it
        // kill(process_child, SIGUSR1);
        p->id = process_child;
        process_table_add(pt, p);
    }
    return;
}

void HPF_Handler(int signum)
{
    raise(SIGCONT);
}

void HPF(pri_queue *processes)
{
    signal(SIGUSR1, HPF_Handler);
    int x = pri_queue_dequeue(processes, (void **)&running_process);
    if (x == 0)
        return;
    // start the process
    running_pcb = process_table_find(&pt, running_process);
    running_pcb->state = 1;
    log_process_data(running_pcb);
    kill(running_pcb->id, SIGCONT);
    // waitpid(running_pcb->id, NULL, WUNTRACED);
    //  wait for the process to finish
    raise(SIGSTOP);
    // stop the process and log its data
    running_pcb->state = 4;
    log_process_term(running_pcb);
    add_Sums(running_pcb);
    // remove the process from the process table
    process_table_remove(&pt, running_process);
    curr_process_num++;
}

void HPF_Scheduler()
{
    struct pri_queue hpf_ready_queue;
    pri_queue_init(&hpf_ready_queue);
    while (1)
    {
        HPF(&hpf_ready_queue);
        while (1)
        {
            received_process = receive_process(process);
            pri_queue_enqueue(&hpf_ready_queue, received_process->priority, (void *)received_process);
        }
        if (curr_process_num >= process_num)
            break;
    }
    perf_process_final(running_pcb);
}