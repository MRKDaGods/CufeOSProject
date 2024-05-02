#include "headers.h"
void clearResources(int);
int read_processes(pri_queue *processes);
void fork_clk();
void fork_scheduler(int numprocesses, char *argv[]);
void send_process(pri_queue *processes, key_t clk_child);
key_t msgq_id;
pid_t clk_child, scheduler_child;

int main(int argc, char *argv[])
{
    // TODO Initialization
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    signal(SIGINT, clearResources);
    // process queue (priority = arrivalTime) incase processes.txt isnt sorted by AT
    pri_queue processesQueue;
    pri_queue_init(&processesQueue);
    int x = read_processes(&processesQueue);
    fork_clk();
    int size = pri_queue_size(&processesQueue);
    fork_scheduler(size, argv);
    initClk();
    send_process(&processesQueue, clk_child);
    pri_queue_free(&processesQueue);
    wait(NULL);
}

void clearResources(int signum)
{
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    exit(0);
}

int read_processes(pri_queue *processes)
{
    if (!processes)
        return 0;

    FILE *f = fopen("processes.txt", "r");
    if (!f)
    {
        // invalid file?
        return 0;
    }

    char *line = 0;
    size_t lineLen = 0;
    while (getline(&line, &lineLen, f) != EOF)
    {
        // we have a line :P
        // ignore empty lines or lines that start with #
        if (lineLen == 0 ||
            strlen(line) == 0 ||
            line[0] == '#')
            continue;

        // allocate process
        struct process_data *p = malloc(sizeof(process_data));
        memset(p, 0, sizeof(process_data));

        // read proc data
        sscanf(line, "%d%d%d%d", &p->id, &p->arrival_time, &p->running_time, &p->priority);

        // insert in queue
        pri_queue_enqueue(processes, p->arrival_time, p);
        printf("Process with id %d, arrivaltime %d, remainingtime %d, priority %d\n", p->id, p->arrival_time, p->running_time, p->priority);
    }

    // close file
    fclose(f);

    // free line
    if (line)
    {
        free(line);
    }

    return 1;
}
void fork_clk()
{
    pid_t clk_child = fork();
    if (clk_child == -1)
    {
        perror("Failed to fork clk_child");
        exit(EXIT_FAILURE);
    }

    else if (clk_child == 0)
    {
        if (execl("./clk.out", "./clk.out", NULL) == -1)
        {
            perror("Failed to execl clk.out");
            exit(EXIT_FAILURE);
        }
    }
    return;
}
void fork_scheduler(int numprocesses, char *argv[])
{
    scheduler_child = fork();
    if (scheduler_child == -1)
    {
        perror("Failed to fork scheduler_child");
        exit(EXIT_FAILURE);
    }
    else if (scheduler_child == 0)
    {
        char *arr = malloc(sizeof(*arr));
        sprintf(arr, "%d", numprocesses);
        execl("./scheduler.out", "./scheduler.out", argv[1], arr, NULL);
    }
    return;
}
void send_process(pri_queue *processes, key_t clk_child)
{
    // Message Queue Generation to send the process data to the scheduler
    msgq_id = msgget(MSGKEY, 0666 | IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("Error in create Message Queue");
        exit(-1);
    }
    int send_val;
    struct process_data *p = malloc(sizeof(process_data));
    memset(p, 0, sizeof(process_data));
    struct process_data pro;
    while (pri_queue_dequeue(processes, (void **)&p))
    {
        // printf("Process with id %d, arrivaltime %d, remainingtime %d, priority %d\n", p->id, p->arrival_time, p->running_time, p->priority);
        //   printf("%d\n", getClk());
        if (p->arrival_time > getClk())
        {
            // printf("waiting for %d\n", p->arrival_time - getClk());
            sleep(p->arrival_time - getClk());
        }
        printf("sending process with id %d and running time %d and arrivaltime  %d and priority %d at time %d\n", p->id, p->running_time, p->arrival_time, p->priority, getClk());
        struct msgbuff *buf = malloc(sizeof(struct msgbuff));
        // memset(buf, 0, sizeof(struct msgbuff));
        pro = *p;
        buf->mtype = 1;
        buf->data = pro;
        send_val = msgsnd(msgq_id, buf, sizeof(buf->data), !IPC_NOWAIT);
        if (send_val == -1)
        {
            perror("Error in send");
            printf("errno: %d\n", errno);
            exit(-1);
        }
        kill(clk_child, SIGCONT);
    }
    free(p);
}