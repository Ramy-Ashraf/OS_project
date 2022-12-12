#include "headers.h"

void clearResources(int);

int main(int argc, char *argv[])
{
    //checking number of arguments entered in the terminal
    if (argc<4)
    {	 
		printf("Too few arguments. Exiting!\n");         
		exit(1);
    }

    signal(SIGINT, clearResources);
    
    // TODO Initialization
    // 1. Read the input files.

    char str[100];
    int Nlines = 0 ,i = 0;
    char ch;

    FILE *fp;
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("Error opening file");
        exit(1);
        printf("\n");
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

    // // print Processes
    // for (int i = 0; i < Nlines; i++)
    // {
    //     for (int j = 0; j < 4; j++)
    //     {
    //         printf("%d ", Processes[i][j]);
    //     }
    //     printf("\n");
    // }

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.

        char algo;
        algo=atoi(argv[2]);

        printf("The algorithm you chose is : ");
        if(algo==1)
        {
            printf("Shortest job first\n");
        }
        else if(algo==2)
        {
            printf("Preemptive Highest Priority First\n");
        }
        else if(algo==3)
        {
            printf("Round Robin\n");
        }
        else if(algo==4)
        {
            printf("Multiple level Feedback Loop\n");
        }

    // 3. Initiate and create the scheduler and clock processes.

    // 4. Use this function after creating the clock process to initialize clock.

    //  initClk();
     // To get time use this function. 
     int x = getClk();
    //  printf("Current Time is %d\n", x);

     // TODO Generation Main Loop
     // 5. Create a data structure for processes and provide it with its parameters.
     // 6. Send the information to the scheduler at the appropriate time.
    
     destroyClk(true);
}
     
// 7. Clear clock resources
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    destroyClk(false);
    exit(signum);

}
