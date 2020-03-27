/*#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
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

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
*/
#include "headers.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <sys/types.h>
#include "LinkedList.c"

#define _GNU_SOURCE

pid_t pid;
key_t msgqid1;
key_t msgqid2;

list process_list = NULL;

int processes_count;
// int** processes_info;

void clearResources(int);

    // implement read file functionality here
    FILE *fp;
    ssize_t read;
    size_t len;
    char * line = NULL;
    // int line_count;
    int c = 0;

    fp = fopen("processes.txt", "r");

    if (fp == NULL )
    {
        printf("No such file ... \n");
        exit(EXIT_FAILURE);
    }

    processes_count = count_lines(fp);
    while ((read = getline(&line, &len, fp)) != -1)
    {
            struct data temp;
            sscanf(line, "%d\t%d\t%d\t%d", &temp->id, &temp->arrival, &temp->runtime, &temp->Priority);
            process_list = addToList(process_list,temp);
    }
    // closing file and  disposing pointers
    if (line)
    {
        free(line);
    }
    // exit(EXIT_SUCCESS); /*can cause the function to return NULL, be careful */


// function to send processes queue to the scheduler process
void send_processes_queue(struct Queue processes_queue, key_t msgqid,list plist)
{
	int send_val;
	struct Data processes_data;

	// make sure the queue is properly set up
	initializeQueue(processes_queue);

	// read processes from Matrix
	for (int i = 0; i < processes_count-1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (j == 0)
			{
				processes_data.id = processes_info[i][j];
			}
			else if (j == 1)
			{
				processes_data.arrival = processes_info[i][j];
			}
			else if (j == 2)
			{
				processes_data.runtime = processes_info[i][j];
			}
			// insert(processes_queue, processes_data);  // not working
		}
		processes_queue.dataArray[processes_queue.rear] = processes_data;
		processes_queue.rear++;
		printf("Size of sent buffer: %d\n", size(processes_queue));
		// (working) printf("Inserted: %d\n", processes_data.runtime);
	}


	printf("before sending(manual): id=%d\truntime=%d\n", processes_queue.dataArray[0].id, processes_queue.dataArray[0].runtime);
	// printf("before sending(peek): id=%d\truntime=%d\n", peek(processes_queue).id, peek(processes_queue).runtime);
	send_val = msgsnd(msgqid, &processes_queue, sizeof(processes_queue.dataArray), !IPC_NOWAIT);

	if (send_val == -1)
    {
        perror("Error in send");
    }
    else
    {
        printf("\nProcesses Queue Sent\n");
    }

}

// function to send process
void send_process(int id, struct Queue prc_queue, key_t msgqid)
{
	int send_val;

	struct Data sentData = prc_queue.dataArray[id];

	send_val = msgsnd(msgqid, &sentData, sizeof(sentData), !IPC_NOWAIT);

	if (send_val == -1)
	{
		perror("Error in send ");
	}
	else
	{
		printf("Process Data Sent Successfully\n");
	}
}

void initiate_RR_scheduler()
{
    // char q_str[2];
    // itoa(q, q_str, 10); // non-standard
    // sprintf(q_str, "%d", q);
    // printf("Q equals %s", q_str);
    char *s_argv[] = {"./scheduler.out", "-RR", 0};
	execve(s_argv[0], &s_argv[0], NULL);
}

void initiate_clock()
{
    char *c_argv[] = {"./clk.out", 0};
	execve(c_argv[0], &c_argv[0], NULL);
	initClk();
}

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    FILE *fp;
    int line_count = 0;
    fp = fopen("processes.txt", "r");
    if (fp != NULL)
    {
        processes_count = count_lines(fp);
        printf("line count: %d\n", processes_count);

        // processes_info = malloc((line_count-1)*sizeof(int*));
        // 1. Read the input files.
        int** processes_info;
        processes_info = read_processes();
        // memcpy(processes_info, read_processes(), sizeof(read_processes()));
        // verify stored data
        for (int i = 0; i < processes_count-1; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                printf("%d  ", processes_info[i][j]);
            }
            printf("\n");
        }
    }
    else
    {
        printf("No such file ... \n");
    }

    pid = getpid();
	msgqid1 = msgget(16599, IPC_CREAT | 0644);
	msgqid2 = msgget(165999, IPC_CREAT | 0644);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    char ch[1];
    bool keep_going = true;
    while (keep_going)
    {
        printf("Choose scheduler from the list below: \n1. Non-preemptive Highest Priority First (HPF) \n2. Shortest Remaining time next (SRTN) \n3. Round Robin (RR) \nyour choice: ");
        scanf("%s", &ch);
        if (strcmp(ch, "1") == 0)
        {
            // HPF is selected
            printf("HPF is selected \n");
            keep_going = false;


            break;
        }
        else if (strcmp(ch, "2") == 0)
        {
            // SRTN is selected
            printf("SRTN is selected \n");
            keep_going = false;


            break;
        }
        else if (strcmp(ch, "3") == 0)
        {
            // RR is selected
            keep_going = false;

            // fork a process for the scheduler
            int spid = fork();
            if (spid == -1)
            {
				perror("Error in initializing scheduler ..\n");
			}
			else if (spid == 0)	// child process (scheduler)
			{
			    // 3. Initiate and create the
				// scheduler and clock processes.
				// initiate scheduler and clock processes then
				// we need two forks as execve replaces the
				// code of the current process and and jumps
				// to its code
				// initiate the scheduler
				printf("Initializing Scheduler\n");
			    int ss_pid = fork();
			    if (ss_pid == -1)
			    {
			        perror("Error in initializing scheduler");
			    }
			    else if (ss_pid == 0)
			    {
					sleep(1);
					initiate_RR_scheduler();
			        // int sc_status;
			        // ss_pid = wait(&sc_status);


			    }
			    else
			    {
					// sleep(1);
			        // send processes info to the
			        // scheduler process
			        // sleep(1);
			        struct Queue prc_queue;
			        initializeQueue(prc_queue);
			        /* Uncomment this to send the whole processes queue */
			        // send_processes_queue(prc_queue, msgqid2, read_processes());
			        // printf("Queue Sent\n");
					// send process by arrival time
					printf("Before sending\n");
					// printf("no. of processes: %d\n", processes_count);
					// sleep(1);
					printf("reading processes ... \n");
			        prc_queue = readProcessesInfo(prc_queue, read_processes());
			        // test send
			        initClk();
			        // printf("Before:	Id: %d\tRuntime: %d\tArrival: %d\n", prc_queue.dataArray[prc_queue.front].id, prc_queue.dataArray[prc_queue.front].runtime, prc_queue.dataArray[prc_queue.front].arrival);
			        printf("The maximum arrival: %d\n", prc_queue.dataArray[prc_queue.rear-1].arrival);
			        int c = 0;
			        while (getClk() < prc_queue.dataArray[prc_queue.rear-1].arrival)
			        {
						if (getClk() == prc_queue.dataArray[c].arrival)
						{
							send_process(c, prc_queue, msgqid1);
							// for (int i = 0; i < prc_queue.rear-1; i++)
							// {
							// 	prc_queue.dataArray[i] = prc_queue.dataArray[i+1];
							// }
							// prc_queue.rear--;
							c++;
							sleep(1);
						}
					}
			        // send_process(prc_queue.front, prc_queue, msgqid1);
			        // printf("After:	Id: %d\tRuntime: %d\tArrival: %d\n", prc_queue.dataArray[prc_queue.front].id, prc_queue.dataArray[prc_queue.front].runtime, prc_queue.dataArray[prc_queue.front].arrival);
					printf("Process Sent\n");
			        int ss_status;
			        ss_pid = wait(&ss_status);
			        if(!(ss_status & 0x00FF))
					 	printf("\nScheduler with pid %d terminated with exit code %d\n", ss_pid, ss_status>>8);
			    }
			}
			else 	// parent process (process generator)
			{
			    // initiate the clock
		        initiate_clock();
				int status;
				spid = wait(&status);
                // TODO Generation Main Loop
                // 5. Create a data structure for
                // processes and provide it with its
                // parameters.
                // 6. Send the information to the scheduler
                // at the appropriate time.
                // 7. Clear clock resources
                // signal(SIGINT, clearResources);
                destroyClk(true);
			}
			break;
        }
        else
        {
            printf("Invalid Choice ... take another shot. \n");
        }
    }
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    // handle kill signal
    msgctl(msgqid1, IPC_RMID, (struct msqid_ds *) 0);
    msgctl(msgqid2, IPC_RMID, (struct msqid_ds *) 0);
    printf(" Interruption detected ... ");
    exit(0);
}
