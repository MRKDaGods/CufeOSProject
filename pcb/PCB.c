#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>


// Process Control Block (PCB) structure

typedef struct pcb {

    pid_t pid; // Process ID

    int state; // Process state (running, waiting, etc.)

    int execution_time; // Total execution time

    int remaining_time; // Remaining execution time

    int waiting_time; // Total waiting time

} PCB;


// Initialize a new PCB

PCB* pcb_init(pid_t pid) {

    PCB* pcb = (PCB*) malloc(sizeof(PCB));

    pcb->pid = pid;

    pcb->state = 0; // Running

    pcb->execution_time = 0;

    pcb->remaining_time = 0;

    pcb->waiting_time = 0;

    return pcb;

}


// Update the state of a PCB

void pcb_update_state(PCB* pcb, int state) {

    pcb->state = state;

}


// Update the execution time of a PCB

void pcb_update_execution_time(PCB* pcb, int time) {

    pcb->execution_time += time;

    pcb->remaining_time -= time;

}


// Update the waiting time of a PCB

void pcb_update_waiting_time(PCB* pcb, int time) {

    pcb->waiting_time += time;

}


// Delete a PCB

void pcb_delete(PCB* pcb) {

    free(pcb);

}