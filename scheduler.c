#include "headers.h"

int msgqID;

int main(int argc, char *argv[])
{
    // Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT); 
    if(msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    struct Message msg;

    msgrcv(msgqID, &msg, sizeof(msg)-sizeof(msg.mType), getpid(), !IPC_NOWAIT);

    destroyClk(true);
    msgctl(msgqID, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
