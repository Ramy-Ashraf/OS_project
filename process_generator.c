#include "headers.h"

void clearResources(int);

int main(int argc, char *argv[])
{
    char str[100];
    int Nlines = 0 ,i = 0;
    char ch;

    signal(SIGINT, clearResources);
    
    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock.
    FILE *fp;
    fp = fopen("processes.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file");
        exit(1);
    }

    // get number of lines in fp but skip any line stating with "#"
    while (fgets(str, 100, fp) != NULL)
    {
        if (str[0] != '#')
            Nlines++;
    }

    // store numerical values from fp in 2d array
    int Processes[Nlines][4];
    fseek(fp, 0, SEEK_SET);
    while (fgets(str, 100, fp) != NULL)
    {
        if (str[0] != '#')
        {
            sscanf(str, "%d %d %d %d", &Processes[i][0], &Processes[i][1], &Processes[i][2], &Processes[i][3]);
            i++;
        }
    }
    fclose(fp);

    // // print Processes
    // for (int i = 0; i < Nlines; i++)
    // {
    //     for (int j = 0; j < 4; j++)
    //     {
    //         printf("%d ", Processes[i][j]);
    //     }
    //     printf("\n");
    // }

     initClk();
     // To get time use this function. 
     int x = getClk();
     printf("Current Time is %d\n", x);
     // TODO Generation Main Loop
     // 5. Create a data structure for processes and provide it with its parameters.
     // 6. Send the information to the scheduler at the appropriate time.
     // 7. Clear clock resources
    
    
     destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    destroyClk(false);
    exit(signum);

}
