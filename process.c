#include "headers.h"

int msgqID;
int currentTime = 0;
bool running = false;

int main(int agrc, char *argv[])
{
    // TODO The process needs to get the remaining time from somewhere
    // remainingtime = ??;
    int remainingtime = atoi(argv[1]);

    // Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    printf("Current Time is %d\n", x);

    currentTime = getClk();

    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT);
    if (msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    struct Message_Action receivedAction;
    int eventTime = 0;

    receivedAction.mType = 0;

    while (true)
    {
        if (receivedAction.mType != getpid())
        {
            msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), IPC_NOWAIT);
        }
        else
        {
            if (receivedAction.action == ACT_START)
            {
                currentTime = receivedAction.time;
            }
            if (receivedAction.time <= currentTime)
            {
                eventTime = eventTime - (currentTime - receivedAction.time);
                struct Message_Action sentAction;
                sentAction.mType = getpid();
                sentAction.time = eventTime;
                sentAction.action = receivedAction.action;

                msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

                currentTime = receivedAction.time;
                receivedAction.mType = 0;
                running = receivedAction.action == ACT_START;
                eventTime = 0;
            }
        }

        if (currentTime < getClk())
        {
            if (running)
            {
                if (remainingtime > 0)
                {
                    remainingtime--;

                    if (remainingtime == 0)
                    {
                        struct Message_Action sentAction;
                        sentAction.mType = getpid();
                        sentAction.time = eventTime;
                        sentAction.action = ACT_STOP;

                        msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                        break;
                    }
                }
            }

            eventTime++;
            currentTime++;
        }
    }

    destroyClk(false);
    exit(0);
}
