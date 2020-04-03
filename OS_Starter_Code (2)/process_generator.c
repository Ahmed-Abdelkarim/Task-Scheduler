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


#define _GNU_SOURCE

pid_t pid;
key_t msgqid1;
// key_t msgqid2;

int processes_count;
// int** processes_info;

void clearResources(int);

// count lines in a file 
int count_lines(FILE *fp)
{
    int lines = 0;
    char chr;
    
    chr = getc(fp);
    while (chr != EOF)
    {
        if (chr == '\n')
        {
            lines++;
        }
        
        chr = getc(fp);
    }
    
    fclose(fp);
    return lines;
}

int** create_array(int m, int n)
{
    int* values = calloc(m*n, sizeof(float));
    int** rows = malloc(n*sizeof(int*));
    for (int i = 0; i < n; i++)
    {
        rows[i] = values + i*m;
    }
    return rows;
}

int** read_processes()
{
    // implement read file functionality here
    FILE *fp1;
    FILE *fp2;
    ssize_t read;
    size_t len;
    char * line = NULL;
    // int line_count;
    int c = 0;
    int temp_id;
    int temp_arrival;
    int temp_runtime;
    int temp_priority;
    
    fp1 = fopen("processes.txt", "r");
    fp2 = fopen("processes.txt", "r");
    
    if (fp1 == NULL || fp2 == NULL)
    {
        printf("No such file ... \n");
        exit(EXIT_FAILURE);
    }
    
    processes_count = count_lines(fp1);
    // printf("Line Count: %d\n", line_count);
    int ids[processes_count];
    int arrivals[processes_count];
    int runtimes[processes_count];
    int priorities[processes_count];
    
    int** ret_arr = create_array(4, processes_count-1);
    
    
    while ((read = getline(&line, &len, fp2)) != -1)
    {
        // printf("%s\n", line);
        if (c == 0)
        {
            c++;
        }
        else
        {
            c++;
            sscanf(line, "%d\t%d\t%d\t%d", &temp_id, &temp_arrival, &temp_runtime, &temp_priority);   
            // printf("this is test\n");
            // printf("ID: %d\n", temp_id);
        }
        
        // passing parameters to list stats
        ids[c-2] = temp_id;
        arrivals[c-2] = temp_arrival;
        runtimes[c-2] = temp_runtime;
        priorities[c-2] = temp_priority;
    }
    
    // passing local arrays to return array 
    // ret_arr = {ids, arrivals, runtimes, priorities};
    for (int i = 0; i < processes_count-1; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (j == 0)
            {
                ret_arr[i][j] = ids[i];
            }   
            else if (j == 1)
            {
                ret_arr[i][j] = arrivals[i];
            }   
            else if (j == 2)
            {
                ret_arr[i][j] = runtimes[i];
            }
            else
            {
                ret_arr[i][j] = priorities[i];
            }  
            // printf("Data: %d  ",ret_arr[i][j]);  
        }
        // printf("\n");
    }
    
    // closing file and  disposing pointers
    if (line)
    {
        free(line);
    }
    // exit(EXIT_SUCCESS); /*can cause the function to return NULL, be careful */
    
    return ret_arr;
}

// function to send processes queue to the scheduler process
void send_processes_queue(struct Queue processes_queue, key_t msgqid, int** processes_info)
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
		// printf("Process Data Sent Successfully\n");
	}
}

// function to read input processes and store them into a queue (working)
struct Queue readProcessesInfo(struct Queue prc_queue, int** processes_info)
{
	struct Data processes_data;
	printf("Beginning of ReadAndSendFunction ... \n");
	// initializeQueue(prc_queue);
	
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
		prc_queue.dataArray[prc_queue.rear] = processes_data;
		prc_queue.rear++;
		// printf("Inserted: %d\n", processes_data.runtime);
	}
	printf("Size of sent buffer: %d\n", size(prc_queue));
	// (working) printf("Id: %d\tRuntime: %d\tArrival: %d\n", prc_queue.dataArray[prc_queue.front].id, prc_queue.dataArray[prc_queue.front].runtime, prc_queue.dataArray[prc_queue.front].arrival);
	return prc_queue;
}

void initiate_RR_scheduler()
{
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
    pid = getpid();
	msgqid1 = msgget(16599, IPC_CREAT | 0644);
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
					// sleep(1);
					initiate_RR_scheduler();
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
			        prc_queue = readProcessesInfo(prc_queue, read_processes());
			        initClk();
			        int c = 0;
			        while (getClk() <= prc_queue.dataArray[prc_queue.rear-1].arrival)
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
							// sleep(1);
						}
					}
			        // send_process(prc_queue.front, prc_queue, msgqid1);
			        // printf("After:	Id: %d\tRuntime: %d\tArrival: %d\n", prc_queue.dataArray[prc_queue.front].id, prc_queue.dataArray[prc_queue.front].runtime, prc_queue.dataArray[prc_queue.front].arrival);
					printf("Processes Sent\n");
			        int ss_status;
			        ss_pid = wait(&ss_status);
			        if(!(ss_status & 0x00FF))
			        {
					  	printf("Scheduler with pid %d terminated with exit code %d\n", ss_pid, ss_status>>8);
						destroyClk(true);
						exit(0);
					}
			    }
			}
			else 	// parent process (process generator)
			{
			    // initiate the clock
		        initiate_clock();
				int status;
				spid = wait(&status);
				if (spid >= 0)
				{
					if (WEXITSTATUS(status) == 0)
					{
						// Program Terminated Okay
						printf("Good Bye ... \n");
						exit(0);
					}
				}
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
    // msgctl(msgqid2, IPC_RMID, (struct msqid_ds *) 0);
    printf(" Interruption detected ... ");
    exit(0);
}
