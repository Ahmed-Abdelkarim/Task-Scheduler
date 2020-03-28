#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int runtime;
int main(int agrc, char * argv[])
{
    initClk();

    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    remainingtime = runtime - (clock()/CLOCKS_PER_SEC);
    while (remainingtime > 0)
    {
        remainingtime = runtime - (clock()/CLOCKS_PER_SEC);
    }

    destroyClk(false);

    return 0;
}
