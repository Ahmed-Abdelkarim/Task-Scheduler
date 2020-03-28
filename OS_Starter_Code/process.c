#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
double allocated_quanta = 0;

int main(int argc, char * argv[])
{
    initClk();
    
    if (argc > 1)
    {
		// allocated_quanta = argv[1];
	}
	
	if (allocated_quanta != 0)
	{
		// case of RR algorithm 
	}
	else
	{
		//TODO it needs to get the remaining time from somewhere
		remainingtime = clock();	/* number of processor ticks since
		the begining of the process */
		while (remainingtime > 0)
		{
			remainingtime = clock();
		}
	}
    
    
    destroyClk(false);
    
    return 0;
}
