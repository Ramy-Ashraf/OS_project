#include "headers.h"

void passTime(int signum);

int msgqID;
int currentTime = 0;
int clkTime = 0;
bool running = false;

int main(int agrc, char *argv[])
{
    signal(SIGUSR1, passTime);

    // TODO The process needs to get the remaining time from somewhere
    // remainingtime = ??;
    int remainingtime = atoi(argv[1]);

    // Use this function after creating the clock process to initialize clock.
    /*INITIALIZED IN SCHEDULER*/

    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT);
    if (msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    struct Message_Action receivedAction;
    int eventTime;

    msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), !IPC_NOWAIT);
    running = receivedAction.action == ACT_START;
    clkTime = receivedAction.time;
    currentTime = receivedAction.time;
    eventTime = 0;

    while (true)
    {
        if (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), IPC_NOWAIT) != -1)
        {
            struct Message_Action sentAction;
            sentAction.mType = getppid();
            sentAction.time = eventTime;
            sentAction.action = receivedAction.action;

            msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

            running = receivedAction.action == ACT_START;
            eventTime = 0;
        }

        if (currentTime < clkTime)
        {
            if (running)
            {
                if (remainingtime > 0)
                {
                    remainingtime--;
                }
            }
            eventTime++;

            if (remainingtime == 0)
            {
                struct Message_Action sentAction;
                sentAction.mType = getppid();
                sentAction.time = eventTime;
                sentAction.action = ACT_STOP;

                msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                break;
            }
            
            currentTime++;
        }
    }

    destroyClk(false);
    exit(0);
}

void passTime(int signum)
{
    clkTime++;
}
