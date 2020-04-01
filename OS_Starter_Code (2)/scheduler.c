#include "headers.h"
#include "queue.h"
#include <string.h>
#include <sys/mman.h>	// for the purpose of shared memory
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

pid_t pid;
key_t msgqid1;
// key_t msgqid2;
// Log File for the scheduler
FILE * logFilePtr;

void cleanUpThings(int signum);

// function to receive the processes queue 
struct Queue recv_msg(key_t msgqid)
{
	int recv_val;
	struct Queue readyQueue;
	
	// receive all messages 
	recv_val = msgrcv(msgqid, &readyQueue, sizeof(readyQueue.dataArray), 0, !IPC_NOWAIT);
	
	if (recv_val == -1)
	{
		perror("Error in receive, bad data returned");
		return readyQueue;
	}
	else
	{
		printf("\nProcesses received \n");
		return readyQueue;
	}
}

// function to receive the process data
struct Data recv_msg_process(key_t msgqid)
{
	int recv_val;
	struct Data processData;
	
	// receive all messages
	recv_val = msgrcv(msgqid, &processData, sizeof(processData), 0, !IPC_NOWAIT);
	
	if (recv_val == -1)
	{
		perror("Error in receive, bad data returned");
		return processData;
	}
	else
	{
		printf("\nProcess Data received\n");
		return processData;
	}
}

void main(int argc, char * argv[])
{
	// for cleaning up functionality
	signal(SIGINT, cleanUpThings);
	
	pid = getpid();
	msgqid1 = msgget(16599, 0644);
	
	logFilePtr = fopen("scheduler.log", "w");
	if (logFilePtr == NULL)
	{
		printf("Unable to create log file for the scheduler ... \n");
	}
	
	
	// scheduler logic
	initClk();
	// processes ready data
	struct Queue readyQueue;
	// initializeQueue(readyQueue);
	readyQueue.front = readyQueue.rear = 0;
	printf("ready Queue front %d\n", readyQueue.front);
	
	// Create a shared memory for the ready queue between child and parent
	int shm_fd;
	struct Queue * shared_processes_memory;
	const char *shared_memory_name = "ProcessesData";
	int msize = sizeof(struct Queue);	// size of the shared memory segment (in bytes)
	// free up shared memory (if it exists)
	shm_unlink(shared_memory_name);
	// open the memory
	shm_fd = shm_open(shared_memory_name, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
	if (shm_fd < 0)
	{
		fprintf(stderr, "Error in shm_open()");
		return;
	}
	printf("Created Shared Memory Object %s\n", shared_memory_name);
	// attach the shared memory segment 
	ftruncate(shm_fd, msize);
	printf("shmat returned\n");
	// allocating the shared memory segment
	shared_processes_memory = (struct Queue *)mmap(NULL, msize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shared_processes_memory == NULL)
	{
		fprintf(stderr, "Error in mmap()");
		return;
	}
	printf("Shared memory segment allocated correctly (%d bytes) original size(%d bytes).\n", msize, sizeof(readyQueue));

	
	// Create a shared memory for the quanta between child and parent
	int shm_fd1;
	int * shared_quanta_memory;
	const char *shared_memory_name1 = "QuantaData";
	int msize1 = sizeof(int);	// size of the shared memory segment (in bytes)
	// free up shared memory (if it exists)
	shm_unlink(shared_memory_name1);
	// open the memory
	shm_fd1 = shm_open(shared_memory_name1, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
	if (shm_fd1 < 0)
	{
		fprintf(stderr, "Error in shm_open()");
		return;
	}
	printf("Created Shared Memory Object %s\n", shared_memory_name1);
	// attach the shared memory segment 
	ftruncate(shm_fd1, msize1);
	printf("shmat returned\n");
	// allocating the shared memory segment
	shared_quanta_memory = (int *)mmap(NULL, msize1, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
	printf("Shared memory segment allocated correctly (%d bytes) original size(%d bytes).\n", msize1, sizeof(int));
	

	//TODO implement the scheduler :)
	//upon termination release the clock resources
	if (strcmp(argv[1], "-RR") == 0)
	{
	    int q;
		/* Uncomment this to receive the whole processes queue */
		// prc_queue = recv_msg(msgqid2);
		int waiting_interval = 0;
		printf("Q: ");
		scanf("%d", &q);
		printf("RR is selected with Q = %d\n", q);
		// receive process by process (fork a process to wait for messages)
		int serv_pid = fork();
		if (serv_pid == -1)
		{
			perror("Error in forking .. ");
		}
		else if (serv_pid == 0)
		{
			shared_quanta_memory[0] = q;
			initClk();
			printf("Receiving Messages from process generator .. \n");
			// processes received data
			struct Queue tempQueue;
			initializeQueue(tempQueue);
			shared_processes_memory->rear = shared_processes_memory->front = 0;
			int i = tempQueue.front;
			while (1)
			{
				struct Data data;
				if (msgrcv(msgqid1, &data, sizeof(data), 0, !IPC_NOWAIT) == -1)
				{
					printf("Error in receiving messages ...\n");
				}
				else
				{
					shared_processes_memory->dataArray[shared_processes_memory->rear] = data;
					strcpy(shared_processes_memory->dataArray[shared_processes_memory->rear].status, "waiting");
					shared_processes_memory->rear++;
					i++;
				}
				
				sleep(1);
			}
		}
		else
		{
			// current running process
			struct Data currentProcessData;
			// for CPU Utilization
			int total_time = 0;
			int use_time = 0;
			
			while (1)
			{
				printf("Scheduler Running .. with Q = %d\n", shared_quanta_memory[0]);
				for (int i = readyQueue.rear; i < shared_processes_memory->rear; i++)
				{
					// Check to see if any processes are in the ready queue
					readyQueue.dataArray[i] = shared_processes_memory->dataArray[i];
					readyQueue.rear++;
					shared_processes_memory->rear--;
					// add its runtime to the sum 
					total_time += readyQueue.dataArray[i].runtime;
					printf("Process Added to Queue\n");
				}
				// readyQueue.rear = shared_processes_memory->rear;
				printf("No. of processes in ready queue: %d\n", readyQueue.rear);
				if (readyQueue.rear > 1)
				{
					printf("Ready Processes Count: %d\n", readyQueue.rear);
					// new data received then reset the timer
					waiting_interval = 0;
					// fork a process then wait for its termination 
					// (you should be able to detect when it will terminate
					// (or pause)
					
					for (int i = 0; i < readyQueue.rear; i++)
					{
						// Forking a process
						/* Check if it is new or waiting */
						/* Add the frontier process to the processing queue */
						currentProcessData = readyQueue.dataArray[readyQueue.front];
						
						// run the process for a specific quanta
						//if (strcmp("waiting", currentProcessData.status) == 0)
						//{
						//	
						//}
						for (int j = 0; j < readyQueue.rear-1; j++)
							{
								readyQueue.dataArray[j] = readyQueue.dataArray[j+1];
							}
							readyQueue.rear--;
							printf("Ready Queue Frontier (ID): %d\n", readyQueue.dataArray[readyQueue.front].id);
							/* fork the process */
							int prc_pid = fork();
							if (prc_pid == -1)
							{
								perror("Could not start process ... \n");
							}
							else if (prc_pid == 0)
							{
								printf("Process %d started \n", currentProcessData.id);
								fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n", getClk(), currentProcessData.id, currentProcessData.arrival, shared_processes_memory->dataArray[currentProcessData.id-1].runtime, getClk()-currentProcessData.last_processed);
								// currentProcessData.status = "running";
								strcpy(currentProcessData.status, "running");
								alarm(shared_quanta_memory[0]);
								char buf[3];
								// itoa(currentProcessData.runtime, buf, 10);
								sprintf(buf, "%d", currentProcessData.runtime);
								char *prc_argv[] = {"./process.out", buf, 0};
								execve(prc_argv[0], &prc_argv[0], NULL);
							}
							else 
							{
								int prc_status;
								int prc_ret = wait(&prc_status);
								if (prc_ret >= 0) 
								{
									if (WEXITSTATUS(prc_status) == 1)
									{
										// process terminated with pause exit code
										/* put it at the rear of the queue */
										fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n", getClk()-shared_quanta_memory[0], currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
										currentProcessData.runtime = currentProcessData.runtime - shared_quanta_memory[0];
										printf("Expected runtime for p%d: %d\n", currentProcessData.id, currentProcessData.runtime);
										readyQueue.dataArray[readyQueue.rear] = currentProcessData;
										readyQueue.rear++;
										// output something here to the log file
										printf("Process %d paused \n", currentProcessData.id);
										strcpy(currentProcessData.status, "paused");
										fprintf(logFilePtr, "At time %d process %d paused arr %d remain %d wait %d\n", getClk(), currentProcessData.id, currentProcessData.arrival, shared_processes_memory->dataArray[currentProcessData.id-1].runtime, getClk()-currentProcessData.last_processed);
									}
									if (WEXITSTATUS(prc_status) == 0)
									{
										// process terminated with finish exit code
										/* drop it */
										// output something here to the log file 
										fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n", getClk()-currentProcessData.runtime, currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
										printf("Process %d stopped\n", currentProcessData.id);
										strcpy(currentProcessData.status, "stopped");
										fprintf(logFilePtr, "At time %d process %d stopped arr %d remain %d wait %d\n", getClk(), currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
									}
								}
							}
							use_time = getClk();
							// sleep(1);
					}
				}
				else if (readyQueue.rear == 1)
				{
					// transfer the process to the processing queue
					// and replace readyQueue process id with 0
					// to mark the queue as empty
					printf("Ready Processes Count: %d\n", readyQueue.rear);
					printf("Ready Queue front: %d\n", readyQueue.front);
					// take a process from the queue
					currentProcessData = readyQueue.dataArray[readyQueue.front];
					// decrease the ready processes count
					readyQueue.rear--;
					// verify id of the process
					printf("Ready Queue Frontier (ID): %d\n", readyQueue.dataArray[readyQueue.front].id);
					/* fork the process */
					int prc_pid = fork();
					if (prc_pid == -1)
					{
						perror("Could not start process ... \n");
					}
					else if (prc_pid == 0)
					{
						printf("Process %d started \n", currentProcessData.id);
						// currentProcessData.status = "running";
						strcpy(currentProcessData.status, "running");
						alarm(shared_quanta_memory[0]);
						char buf[3];
						// itoa(currentProcessData.runtime, buf, 10);
						sprintf(buf, "%d", currentProcessData.runtime);
						char *prc_argv[] = {"./process.out", buf, 0};
						execve(prc_argv[0], &prc_argv[0], NULL);
					}
					else 
					{
						int prc_status;
						int prc_ret = wait(&prc_status);
						if (prc_ret >= 0) 
						{
							if (WEXITSTATUS(prc_status) == 1)
							{
								// process terminated with pause exit code
								/* put it at the rear of the queue */
								fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n", getClk()-shared_quanta_memory[0], currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
								currentProcessData.runtime = currentProcessData.runtime - shared_quanta_memory[0];
								printf("Expected runtime for p%d: %d\n", currentProcessData.id, currentProcessData.runtime);
								readyQueue.dataArray[readyQueue.rear] = currentProcessData;
								readyQueue.rear++;
								// output something here to the log file
								printf("Process %d paused \n", currentProcessData.id);
								strcpy(currentProcessData.status, "paused");
								fprintf(logFilePtr, "At time %d process %d paused arr %d remain %d wait %d\n", getClk(), currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
							}
							if (WEXITSTATUS(prc_status) == 0)
							{
								// process terminated with finish exit code
								/* drop it */
								// output something here to the log file 
								fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n", getClk()-currentProcessData.runtime, currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
								printf("Process %d stopped\n", currentProcessData.id);
								strcpy(currentProcessData.status, "stopped");
								fprintf(logFilePtr, "At time %d process %d stopped arr %d remain %d wait %d\n", getClk(), currentProcessData.id, currentProcessData.arrival, currentProcessData.runtime, getClk()-currentProcessData.last_processed);
							}
						}
					}
					use_time = getClk();
					// sleep(1);
				}
				else
				{
					// increment timer for exiting 
					waiting_interval++;
					sleep(1);
					// printf("Waiting ... \n");
				}
				printf("Clock Now: %d\n", getClk());
				// check here to see if the scheduler waited for too long
				// (more than 30 seconds with no incoming processes
				if (waiting_interval >= 30)
				{
					// Closing the log file
					fclose(logFilePtr);
					printf("Waited for too long. Exiting ... \n");
					// printf("CPU Utilization: %lf\n", (double)(use_time/total_time)*100);
					exit(4);
				}
			}
			printf("Exiting Scheduler ... \n");
			int serv_status;
			serv_pid = wait(&serv_status);
			if (serv_pid >= 0)
			{
				if (WEXITSTATUS(serv_status) == 4)
				{
					// Scheduler terminated normally
					exit(0);
				}
			}
		}
	}
}

void cleanUpThings(int signum)
{
	// close the file 
	fclose(logFilePtr);
	printf("Saving log file ... \n");
	exit(2);
}
