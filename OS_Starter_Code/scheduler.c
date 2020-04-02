
#include "headers.h"
#include "queue.h"
#include "priority_queue.h"
#include <string.h>


pid_t pid;
key_t msgqid1;
//key_t msgqid2;
FILE * logFilePtr;
pData recievedData;
bool available = true;

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



void main(int argc, char * argv[])
{
   printf("HPF Scheduler Initialized\n");
   bool firstIteration = 1;

    // scheduler logic
    initClk();
    priority_heap_t *Q = (priority_heap_t*)calloc(1,sizeof(priority_heap_t));
    printf("priority_heap_t created\n");
    logFilePtr = fopen("scheduler.log", "w");
    if (logFilePtr == NULL)
    {
        printf("Unable to create log file for the scheduler ... \n");
    }
    //TODO implement the scheduler :)
    pData runningProcess;
    if (strcmp(argv[1], "-HPF") == 0) {
        while(1){
            if(firstIteration){
                initializePQueue(Q);
                printf("Priority Q initialized\n");
                pid = getpid();
                msgqid1 = msgget(16599, 0644);
                firstIteration = 0;
                printf("first iteration\n");
            }
            int rcv = msgrcv(msgqid1, &recievedData, sizeof(recievedData), 0, IPC_NOWAIT);
            if (rcv = -1) {
                //printf("Error in recieve\n");
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
                    execve(argv[0], &argv[0], NULL);
                }
                else {
                    recievedData.id = cpid;
                    recievedData.isRunning == false;
                    push(Q, recievedData);
                    kill(cpid, SIGSTOP);

                    if (Q->len != 0 && available == true) {
                        runningProcess = pop(Q);
                        runningProcess.isRunning = 1;
                        fprintf(logFilePtr, "At time %d process %d started arr %d remain %d wait %d\n",
                                getClk(),runningProcess.id, runningProcess.arrival, runningProcess.runtime, (runningProcess.arrival-getClk()));
                        kill(runningProcess.id, SIGCONT);
                    }
                    int status;
                    int prc_ret = wait(&status);
                    if(WIFEXITED(status))
                        fprintf(logFilePtr, "\nAt time %d process %d finished running with exit code %d\n", getClk(), runningProcess.id, WEXITSTATUS(status));

                }
            }

        }
    }
    //upon termination release the clock resources
    destroyClk(true);

}
