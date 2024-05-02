#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
void Stop_Handler(int signum);
void Continue_Handler(int signum);
int prev_time;
int next_time;

int main(int argc, char *argv[])
{
    signal(SIGUSR1, Stop_Handler);
    signal(SIGUSR2, Continue_Handler);
    initClk();
    printf("Process %d started\n", getpid());
    if (argc < 2)
    {
        fprintf(stderr, "Incorrect usage, to run use: ./bin/process.out <time>\n");
        return 1;
    }
    remainingtime = atoi(argv[1]);
    prev_time = getClk();
    while (remainingtime > 0)
    {
        next_time = getClk();
        remainingtime -= next_time - prev_time;
    }

    destroyClk(false);
    printf("Process %d finished\n", getpid());
    kill(getppid(), SIGUSR1);
}

void Stop_Handler(int signum)
{
    raise(SIGTSTP);
}
void Continue_Handler(int signum)
{
    prev_time = getClk();
    raise(SIGCONT);
}