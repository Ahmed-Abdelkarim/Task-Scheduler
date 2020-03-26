#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE

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
    int line_count;
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
    
    line_count = count_lines(fp1);
    // printf("Line Count: %d\n", line_count);
    int ids[line_count];
    int arrivals[line_count];
    int runtimes[line_count];
    int priorities[line_count];
    
    int** ret_arr = create_array(4, line_count-1);
    
    
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
    for (int i = 0; i < line_count-1; i++)
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

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    FILE *fp;
    int** processes_info;
    int line_count = 0;
    fp = fopen("processes.txt", "r");
    if (fp != NULL)
    {
        line_count = count_lines(fp);
        printf("line count: %d\n", line_count);
        
        // processes_info = malloc((line_count-1)*sizeof(int*));
        // 1. Read the input files.
        processes_info = read_processes();
        // memcpy(processes_info, read_processes(), sizeof(read_processes()));
        // verify stored data
        for (int i = 0; i < line_count-1; i++)
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
            printf("RR is selected \n");
            keep_going = false;
            
            
            break;
        }
        else
        {
            printf("Invalid Choice ... take another shot. \n");
        }
    }
    
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    // initClk();
    // To get time use this
    // int x = getClk();
    // printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    // destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
