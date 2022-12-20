#include "headers.h"

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    int remainingtime = atoi(argv[1]);

    while (remainingtime > 0)
    {
        // remainingtime = ??;
    }

    destroyClk(false);
    exit(0);
}
