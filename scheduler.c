#include "headers.h"
#include "queue.h"
#include <string.h>

pid_t pid;
key_t msgqid1;
key_t msgqid2;

struct Data
{
	int id;
	int arrival;
	int priority;
	int runtime;
};

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

	// scheduler logic
	initClk();

	//TODO implement the scheduler :)
	//upon termination release the clock resources

	if (strcmp(argv[1], "-RR") == 0)
	{
		// printf("Test");
		// processes data
		// struct Queue prc_queue;
		// initializeQueue(prc_queue);
		pid = getpid();
		msgqid1 = msgget(16599, 0644);
		msgqid2 = msgget(165999, 0644);
		// RR algorithm implementation
	    // import q param
	    // int q = atoi(argv[2]);
	    int q;
		/* Uncomment this to receive the whole processes queue */
		// prc_queue = recv_msg(msgqid2);
		// receive process by process (fork a process to wait for messages)
		int serv_pid = fork();
		if (serv_pid == -1)
		{
			perror("Error in forking .. ");
		}
		else if (serv_pid == 0)
		{
			printf("Q: ");
			scanf("%d", &q);
			printf("RR is selected with Q = %d\n", q);
			initClk();
			printf("Receiving Messages from process generator .. \n");
			// processes data
			struct Queue readyQueue;
			initializeQueue(readyQueue);
			// printf("ID: %d\tArrival:%d\tRuntime:%d\n", readyQueue.dataArray[readyQueue.front].id, readyQueue.dataArray[readyQueue.front].arrival, readyQueue.dataArray[readyQueue.front].runtime);
			int i = readyQueue.front;
			while (1)
			{
				readyQueue.dataArray[readyQueue.rear] = recv_msg_process(msgqid1);
				readyQueue.rear++;
				// verify incoming data
				printf("ID: %d\tArrival:%d\tRuntime:%d\n", readyQueue.dataArray[readyQueue.front].id, readyQueue.dataArray[readyQueue.front].arrival, readyQueue.dataArray[readyQueue.front].runtime);
				// logic for Round Robin
				printf("Clock Now: %d\n", getClk());
				// i++;
				sleep(1);
			}
			// printf("Test");
		}
		else
		{
			printf("Scheduling Logic\n");
			int serv_status;
			serv_pid = wait(&serv_status);
			if (!readyQueue.isEmpty()){
			Data processX = removeData(readyQueue);

			}
		}
	}


}
