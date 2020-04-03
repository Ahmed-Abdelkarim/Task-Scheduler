
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
  printf("Process %d has finished at time %d\n",  runningProcess.id, getClk());
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
          else if (strcmp(argv[1], "-SRTN") == 0)
            {
              printf("intered SRTN algorithm\n" );
              long processInterTime;
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

            }  else {
                printf("Message Recieved\n");
              //fork a process for the comming data
              printf("\n %d %d\n",recievedData.arrival,recievedData.runtime );
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
                  }else {
                    recievedData.id = cpid;
                    recievedData.isRunning == false;
                    //push(Q, recievedData);
                    printf("Process added %d to Q\n", cpid);
                    kill(cpid, SIGSTOP);
                    printf("Process %d is in ready state\n", cpid);

                      if (available == true)
                        {
                          printf("there was no processes running so this process started\n" );
                          runningProcess.id = recievedData.id;
                          runningProcess.runtime = recievedData.runtime;
                          //  printf("\n data id %d\n",cpid );
                          runningProcess.isRunning = 1;
                          available = false;
                          processInterTime = getClk();
                          printf("process inter time %d\n",processInterTime);
                          kill(runningProcess.id,SIGCONT);

                        }
                      else {
                        //if there is no process running run the current one
                        //calculate the time passed in the currentProc
                        runningProcess.remainingT = runningProcess.runtime - clock()/CLOCKS_PER_SEC;
                        //if it is there one running compare between the running time
                        printf("runningProcess.remainingT %d\n",runningProcess.remainingT );
                          if(recievedData.remainingT < runningProcess.remainingT )
                            {
                                kill(runningProcess.id,SIGSTOP);

                                //kill(recievedData.id,SIGCONT); //it is already running
                                //reaProc = insertSorted(reaProc,currentProc);
                                runningProcess.priority = runningProcess.remainingT;
                                push(Q,runningProcess);
                                // it will be more effiecient if you write copy constructor
                                runningProcess.id = recievedData.id;
                                runningProcess.arrival = recievedData.arrival;
                                runningProcess.priority = recievedData.priority;
                                runningProcess.remainingT = recievedData.remainingT;
                                runningProcess.runtime = recievedData.runtime;
                                processInterTime = getClk();
                                kill(recievedData.id,SIGCONT);
                            }
                            else
                            {
                              // use insert sorted to insert the one you want in the ready queue
                              //reaProc = insertSorted(reaProc,recievedData);
                              recievedData.priority = recievedData.runtime;
                              push(Q,recievedData);
                            }}
}}
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
