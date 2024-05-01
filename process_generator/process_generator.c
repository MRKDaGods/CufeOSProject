#include "headers.h"
#include "pri_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
key_t msgq_id;
struct msgbuf
{
    long mtype;
    process *p;
};
 // TODO: 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any. 
void clearResources(int);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
	// 1. Read the input files.
    if (argc != 2)
    {
        return fprintf(stderr, "Incorrect usage, to run use: ./bin/process_generator.out <algorithm number>\n");
    }
    FILE *file = fopen("test.txt", "r");
    if (file == NULL)
    {
        perror("Failed to open the file.\n");
        return 1;
    }
    char *line;
    size_t len = 0;
    ssize_t read;

    pri_queue *q = malloc(sizeof(*q));
    pri_queue_init(q);

    int id, remainingtime, arrivaltime, priority, numProcesses = 0;
    process *p = malloc(sizeof(*p));
    while ((read = getline(&line, &len, file)) != -1)
    {
        if (strncmp(line, "#", 1) != 0)
        {
            numProcesses++;
            if (sscanf(line, "%d %d %d %d", &p->id, &p->arrivaltime, &p->remainingtime, &p->priority) != 4)
            {
                printf("Failed to parse line: %s\n", line);
                continue;
            }
            pri_queue_enqueue(q, p->arrivaltime, p);
            printf("Process with id %d, arrivaltime %d, remainingtime %d, priority %d\n", p->id, p->arrivaltime, p->remainingtime, p->priority);
        }
    }
    print_pri_queue(q);
    fclose(file);
/////////////////////

// 3. Initiate and create the scheduler and clock processes.
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

    pid_t scheduler_child = fork();
    if (scheduler_child == -1)
    {
        perror("Failed to fork scheduler_child");
        exit(EXIT_FAILURE);
    }

    else if (scheduler_child == 0)
    {
        char *arr = malloc(sizeof(*arr));
        sprintf(arr, "%d", numProcesses);
        execl("./scheduler.out", "./scheduler.out", argv[1], arr, NULL);
    }
	/////////////////////////

    initClk(); //initialize clock
    int x = getClk();
    printf("Current Time is %d\n", x);
    // Message Queue Generation to send the process data to the scheduler
    key_t msgq_id = msgget(MSGKEY, 0666 | IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("Error in create Message Queue");
        exit(-1);
    }
    int send_val;
    while (pri_queue_dequeue(q, (void **)&p))
    {
        printf("Process with id %d, arrivaltime %d, remainingtime %d, priority %d\n", p->id, p->arrivaltime, p->remainingtime, p->priority);
        printf("%d\n", getClk());
        if (p->arrivaltime > getClk())
        {
            printf("After Sleeping %d\n", p->arrivaltime - getClk());
            sleep(p->arrivaltime - getClk());
        }
        print_pri_queue(q);
        struct msgbuf *msg = malloc(sizeof(*msg));
        msg->mtype = 1;
        msg->p = p;
        send_val = msgsnd(msgq_id, &msg, sizeof(msg->p), !IPC_NOWAIT);
        if (send_val == -1)
        {
            perror("Error in send");
            exit(-1);
        }
        printf("Process with id %d sent\n", p->id);
    }
    free(p);
    pri_queue_free(q);
    wait(NULL);
}

void clearResources(int signum)
{
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    exit(0);
}