#include "headers.h"
#include "pri_queue.h"

void clearResources(int);
int read_processes(pri_queue* processes);

int main(int argc, char* argv[]) {
	signal(SIGINT, clearResources);

	// process queue (priority = arrivalTime) incase processes.txt isnt sorted by AT
	pri_queue processesQueue;
	pri_queue_init(&processesQueue);



	pri_queue_free(&processesQueue);

	// TODO Initialization
	// 1. Read the input files.
	// 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
	// 3. Initiate and create the scheduler and clock processes.
	// 4. Use this function after creating the clock process to initialize clock
	initClk();
	// To get time use this
	int x = getClk();
	printf("current time is %d\n", x);
	// TODO Generation Main Loop
	// 5. Create a data structure for processes and provide it with its parameters.
	// 6. Send the information to the scheduler at the appropriate time.
	// 7. Clear clock resources
	destroyClk(true);
}

void clearResources(int signum) {
	//TODO Clears all resources in case of interruption
}


//int read_processes(pri_queue* processes) {
//	if (!processes) return 0;
//
//	FILE* f = fopen("processses.txt", "r");
//	if (!f) {
//		// invalid file?
//		return 0;
//	}
//	
//	char* line = 0;
//	size_t lineLen = 0;
//	while (getline(&line, &lineLen, f) != EOF) {
//		// we have a line :P
//		// ignore empty lines or lines that start with #
//		if (lineLen == 0 || 
//			strlen(line) == 0 ||
//			line[0] == '#') continue;
//
//		// allocate process
//		struct process_data* p = malloc(sizeof(process_data));
//		memset(p, 0, sizeof(process_data));
//
//		// read proc data
//		sscanf(line, "%d%d%d%d", &p->id, &p->arrival_time, &p->running_time, &p->priority);
//
//		// insert in queue
//		pri_queue_enqueue(processes, p->arrival_time, p);
//	}
//
//	// close file
//	fclose(f);
//
//	// free line
//	if (line) {
//		free(line);
//	}
//
//	return 1;
//}
