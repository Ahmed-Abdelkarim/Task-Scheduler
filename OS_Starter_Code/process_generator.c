#include "headers.h"

#define _GNU_SOURCE

pid_t pid;
key_t msgqid;

list process_list = NULL;

void clearResources(int);

void initiate_RR_scheduler()
{

    char *s_argv[] = {"./scheduler.out", "-RR", 0};
	execve(s_argv[0], &s_argv[0], NULL);
}

void initiate_SRTN_scheduler()
{

    char *s_argv[] = {"./scheduler.out", "-SRTN", 0};
	execve(s_argv[0], &s_argv[0], NULL);
}

void initiate_HPF_scheduler()
{

    char *s_argv[] = {"./scheduler.out", "-SRTN", 0};
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
      printf("clock initialzed\n" );
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
            sscanf(line, "%d\t%d\t%d\t%d", &temp.id, &temp.arrival, &temp.runtime, &temp.priority);
            process_list = addToList(process_list,temp);
            printf("this is a process \n");
    }
    // closing file and  disposing pointers
    if (line)
    {
        free(line);
    }
    printf("the file is read\n");

    // initiate the clock

    int sch_fork = fork();
    if(sch_fork == -1)
    {
      perror("Error in initializing scheduler ..\n");
    }
    else if(sch_fork == 0)
    {
      //choose the algorithm
          char ch[1];
      printf("Choose scheduler from the list below: \n1. Non-preemptive Highest Priority First (HPF) \n2. Shortest Remaining time next (SRTN) \n3. Round Robin (RR) \nyour choice: ");
        scanf("%s", &ch);
        if (strcmp(ch, "1") == 0)
        {
            printf("HPF is selected \n");
            initiate_HPF_scheduler();
        }
        else if (strcmp(ch, "2") == 0)
        {
            printf("SRTN is selected \n");
            initiate_SRTN_scheduler();

        }
        else if (strcmp(ch, "3") == 0)
        {
          printf("RR is selected \n");
          initiate_RR_scheduler();
        }

    }else{
      initiate_clock();

      sleep(3);

        pid = getpid();
      	msgqid = msgget(16599, IPC_CREAT | 0644);

      list templist = process_list;
      while(templist->next != NULL){
        if(templist->data.arrival == getClk())
        {
          struct Data *dat;
          templist = DeqFromList(templist,dat);
          int send_val = msgsnd(msgqid,dat, sizeof(dat), !IPC_NOWAIT);

      	   if (send_val == -1)
      	    {
      		      perror("Error in send ");
      	    }
      	   else
      	    {
      		      printf("Process Data Sent Successfully\n");
      	    }
        }
      }
    }
					sleep(1);
          //  initClk();
          // To get time use this
          int x = getClk();
          printf("current time is %d\n", x);
            destroyClk(true);
}
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    // handle kill signal
    msgctl(msgqid, IPC_RMID, (struct msqid_ds *) 0);
    printf(" Interruption detected ... ");
    exit(0);
}
