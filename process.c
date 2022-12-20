#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    remainingtime = atoi(argv[1]);

    while (remainingtime > 0)
    {
        // remainingtime = ??;
    }

    destroyClk(false);
    exit(0);
}
