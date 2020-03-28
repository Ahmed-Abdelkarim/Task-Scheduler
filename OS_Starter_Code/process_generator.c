#include "headers.h"

#define _GNU_SOURCE

pid_t pid;
key_t msgqid1;
key_t msgqid2;

list process_list = NULL;

void clearResources(int);

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
    // TODO Initialization
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

    while ((read = getline(&line, &len, fp)) != -1)
    {
            struct Data temp;
            sscanf(line, "%d\t%d\t%d\t%d", &temp.id, &temp.arrival, &temp.runtime, &temp.Priority);
            process_list = addToList(process_list,temp);
    }
    // closing file and  disposing pointers
    if (line)
    {
        free(line);
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
			        int sc_status;
			        ss_pid = wait(&sc_status);


			    }
			    else
			    {
					sleep(1);
			        // send processes info to the
			        // scheduler process
			        // sleep(1);
			      //  struct Queue prc_queue;
			       // initializeQueue(prc_queue);
			        /* Uncomment this to send the whole processes queue */
			        // send_processes_queue(prc_queue, msgqid2, read_processes());
			        // printf("Queue Sent\n");
					// send process by arrival time
					printf("Before sending\n");
					// printf("no. of processes: %d\n", processes_count);
					// sleep(1);
					printf("reading processes ... \n");

			        initClk();
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
