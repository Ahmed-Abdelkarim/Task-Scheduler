#include "priority_queue.h"
#include "headers.h"
key_t msgqid;
int pid;
int spid;


int main(){
  pData p[3];
  p[0].arrival = 3;
  p[0].runtime = 5;
  p[0].priority = 2;

  p[1].arrival = 4;
  p[1].runtime = 5;
  p[1].priority = 7;

  p[2].arrival = 6;
  p[2].runtime = 5;
  p[2].priority = 3;

  msgqid = msgget(16599, IPC_CREAT | 0644);
  pid = getpid();
  spid = fork();
  if(spid == -1){
    perror("Error in forking");
  }
  else if(spid == 0){
    printf("Initializing HPF Scheduler\n");
    char *argv[] = { "./scheduler.out", "-SRTN", 0};
    execve(argv[0], &argv[0], NULL);
  }
  else{
    int cpid = fork();
    if(cpid == -1){
      perror("Error in forking\n");
    }
    else if(cpid == 0){
      printf("Initializing Clock test\n");
      char *argv[] = { "./clk.out",0};
      execve(argv[0], &argv[0], NULL);
    }
    else{
      initClk();
      int c = 0;
      while(c < 3){
        if(getClk() == p[c].arrival){
          int send_val = msgsnd(msgqid, &p[c], sizeof(p[c]), !IPC_NOWAIT);
          if(send_val == -1){
            perror("Error in send\n");
          }
          else{
            printf("Process sent\n");
            c++;
          }
          sleep(1);
        }

      }
    }
  }

  int status;
  pid = wait(&status);
  if(!(status & 0x00FF)){
    destroyClk(true);
    exit(0);
  }
  return 0;
}
