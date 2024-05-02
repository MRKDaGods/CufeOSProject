#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>


// Process Control Block (PCB) structure

typedef struct pcb {

    int pid; // Process ID

    int state; // Process state (running, waiting, etc.)

    int execution_time; // Total execution time

    int remaining_time; // Remaining execution time

    int waiting_time; // Total waiting time

} PCB;


// Initialize a new PCB

PCB* pcb_init(int pid) {

    PCB* pcb = (PCB*)malloc(sizeof(PCB));

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

///////////////////////////////////////////////

typedef struct process_table {

    PCB** table; // Array of PCBs

    int size; // Number of PCBs

    int capacity; // Capacity of the table

} ProcessTable;

ProcessTable* process_table_init(int capacity) {

    ProcessTable* table = (ProcessTable*)malloc(sizeof(ProcessTable));

    table->table = (PCB**)malloc(capacity * sizeof(PCB*));

    table->size = 0;

    table->capacity = capacity;

    return table;

}

void process_table_insert(ProcessTable* table, PCB* pcb) {

    if (table->size < table->capacity) {

        table->table[table->size++] = pcb;

    }

}

void process_table_delete(ProcessTable* table) {

    for (int i = 0; i < table->size; i++) {

        pcb_delete(table->table[i]);

    }

    free(table->table);

    free(table);

}

PCB* process_table_get(ProcessTable* table, int pid) {

    for (int i = 0; i < table->size; i++) {

        if (table->table[i]->pid == pid) {

            return table->table[i];

        }

    }

    return NULL;

}