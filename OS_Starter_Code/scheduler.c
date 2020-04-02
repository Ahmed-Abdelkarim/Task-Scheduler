
#include "headers.h"
#include "queue.h"
#include "priority_queue.h"
#include <string.h>


pid_t pid;
key_t msgqid1;
//key_t msgqid2;
FILE * logFilePtr;
bool available = true;
pData runningProcess;

 pData recv_msg_process(key_t msgqid)
{
    int recv_val;
     pData processData;

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


void hpfHandler(int signum){
  printf("Process %d has finished at time %d\n", runningProcess.id, getClk());
  available = true;
  signal(SIGINT, hpfHandler);
}


void main(int argc, char * argv[])
{
  signal(SIGINT, hpfHandler);
   printf("HPF Scheduler Initialized\n");
   bool firstIteration = 1;


    // scheduler logic
    initClk();
    msgqid1 = msgget(16599, 0644);
    priority_heap_t *Q = (priority_heap_t*)calloc(1,sizeof(priority_heap_t));
    printf("priority_heap_t created\n");
    logFilePtr = fopen("scheduler.log", "w");
    if (logFilePtr == NULL)
    {
        printf("Unable to create log file for the scheduler ... \n");
    }
    //TODO implement the scheduler :)
    if (strcmp(argv[1], "-HPF") == 0) {
        while(1){
            if(firstIteration){
                initializePQueue(Q);
                printf("Priority Q initialized\n");
                pid = getpid();
                firstIteration = 0;
            }
            pData recievedData;
            size_t rcv = msgrcv(msgqid1, &recievedData, sizeof(recievedData), 0, IPC_NOWAIT);
            if (rcv == -1) {
                //perror("Error in recieve\n");

            }
            else {
                printf("Message Recieved\n");
                int cpid = fork();
                if(cpid == -1) {
                    printf("Error in fork..\n");
                }
                else if(cpid == 0) {
                    char buf[3];
                    // itoa(currentProcessData.runtime, buf, 10);
                    sprintf(buf, "%d", recievedData.runtime);
                    char *argv[] = { "./process.out", buf, 0};
                    printf("process %d forked\n", getpid());
                    execve(argv[0], &argv[0], NULL);
                }
                else {
                    recievedData.id = cpid;
                    recievedData.isRunning == false;
                    push(Q, recievedData);
                    printf("Process added %d to Q\n", cpid);
                    kill(cpid, SIGSTOP);
                    printf("Process %d is in ready state\n", cpid);
                }
              }
                    if (Q->len != 0 && available == true) {
                        printf("Condition is true\n");
                        runningProcess = pop(Q);
                        runningProcess.isRunning = 1;
                        printf("Process %d is now active\n", runningProcess.id);
                        fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n",
                                getClk(),runningProcess.id, runningProcess.arrival, runningProcess.runtime, (runningProcess.arrival-getClk()));
                        kill(runningProcess.id, SIGCONT);
                        available = false;
                    }

                    sleep(1);

            }

        }

    //upon termination release the clock resources
    destroyClk(true);

}
