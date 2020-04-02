void  HPF() {
    //initializing some staff
    signal(SIGINT, hpfHandler);
    printf("HPF Scheduler Initialized\n");
    bool firstIteration = 1;
    initClk();
    msgqid1 = msgget(16599, 0644);
    priority_heap_t *Q = (priority_heap_t*)calloc(1,sizeof(priority_heap_t));
    printf("Priority Queu created\n");
    logFilePtr = fopen("scheduler.log", "w");
    if (logFilePtr == NULL)
    {
        printf("Unable to create log file for the scheduler ... \n");
    }




    //Beginning of the Sceduler logic
    while(1) {
        //initializing the priority Q parameters
        if(firstIteration) {
            initializePQueue(Q);
            printf("Priority Q initialized\n");
            firstIteration = 0;
        }

        //Creating a container for the recieved process
        pData recievedData;

        //Recieving Processes that are in the Message Queue
        while(1) {
            size_t rcv = msgrcv(msgqid1, &recievedData, sizeof(recievedData), 0, IPC_NOWAIT);
            if (rcv == -1) {
                //perror("Error in recieve\n");
                break;
            }
            else {
                printf("Message Recieved\n");

                //Forking  processes corresponding to the new Data recived
                int cpid = fork();
                if(cpid == -1) {
                    printf("Error in fork..\n");
                }
                else if(cpid == 0) {
                    //Passing the runtime to the child proccess
                    char buf[3];
                    sprintf(buf, "%d", recievedData.runtime);
                    char *argv[] = { "./process.out", buf, 0};
                    printf("process %d forked\n", getpid());
                    execve(argv[0], &argv[0], NULL);
                }
                else {
                    //Adding the new processes received to the ready Queue
                    recievedData.id = cpid;
                    recievedData.isRunning == false;
                    push(Q, recievedData);
                    kill(cpid, SIGSTOP);
                    printf("Process %d is in ready state\n", cpid);
                }
            }
        }
        //If the processor is not busy and there is ready processes set the appropriate process to active
        if (Q->len != 0 && available == true) {
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
    //upon termination release the clock resources
    destroyClk(true);
}
