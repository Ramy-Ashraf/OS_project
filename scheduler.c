#include "headers.h"

void clearResources(int signum);

int msgqID;
int currentTime = 0;
bool running = false;

void Algorithm_SJF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber);
void Algorithm_HPF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber);
void Algorithm_RR(int *const quantum, struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber);
void Algorithm_MLFQ(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // Receive arguments
    enum Algorithm schedulingAlgorithm = (enum Algorithm)atoi(argv[1]);
    int quantum = atoi(argv[2]);

    // CPU Parameters
    struct Queue_Log *logQueue = createQueue_Log();
    int processorIdleTime;
    int processWaitingTime;
    int processWeightedTurnaround;
    int processNumber;

    // Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    printf("Current Time is %d\n", x);

    currentTime = getClk();

    // TODO: implement the scheduler.
    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT);
    if (msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    struct Message_Action receivedAction;
    msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), !IPC_NOWAIT);

    running = receivedAction.action == ACT_START;

    switch (schedulingAlgorithm)
    {
    case ALGO_SJF:
        Algorithm_SJF(&logQueue, &processorIdleTime, &processWaitingTime, &processWeightedTurnaround, &processNumber);
        break;

    case ALGO_HPF:
        Algorithm_HPF(&logQueue, &processorIdleTime, &processWaitingTime, &processWeightedTurnaround, &processNumber);
        break;

    case ALGO_RR:
        Algorithm_RR(&quantum, &logQueue, &processorIdleTime, &processWaitingTime, &processWeightedTurnaround, &processNumber);
        break;

    case ALGO_MLFQ:
        Algorithm_MLFQ(&logQueue, &processorIdleTime, &processWaitingTime, &processWeightedTurnaround, &processNumber);
        break;

    default:
        break;
    }

    while (logQueue->start)
    {
        char event[10];

        switch (logQueue->start->event)
        {
        case EV_STARTED:
            strcpy(event, "started");
            break;

        case EV_RESUMED:
            strcpy(event, "resumed");
            break;

        case EV_STOPPED:
            strcpy(event, "stopped");
            break;

        case EV_FINISHED:
            strcpy(event, "finished");
            break;
        default:
            break;
        }
        printf("#At time %d process %d %s arr %d total %d remain %d wait %d", logQueue->start->time, logQueue->start->id, event, logQueue->start->arrivalTime, logQueue->start->totalTime, logQueue->start->remainingTime, logQueue->start->waitingTime);
        free(dequeue_Log(logQueue));
    }

    // TODO: upon termination release the clock resources.
    destroyClk(false);
    exit(0);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    destroyClk(true);
    exit(0);
}

/*IMPLEMENT SHORTEST JOB FIRST ALGORITHM*/
void Algorithm_SJF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        if (currentTime < getClk())
        {
            if (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), IPC_NOWAIT) != -1)
            {
                running = receivedAction.action == ACT_START;
            }
            while (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getpid(), IPC_NOWAIT) != -1)
            {
                pid_t processPID = fork();
                if (processPID == -1)
                {
                    perror("Error creating new process!\n");
                    exit(1);
                }
                else if (processPID == 0)
                {
                    char rt[5];
                    sprintf(rt, "%d", receivedProcess.attachedProcess.runTime);

                    execl("process.out", "process.out", rt, NULL);
                    perror("Error creating new process!\n");
                    exit(1);
                }

                struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));
                /*  GOOD LUCK   */
            }

            currentTime++;
        }
    }
}

/*IMPLEMENT HIGHEST PRIORITY FIRST ALGORITHM*/
void Algorithm_HPF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        if (currentTime < getClk())
        {
            if (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), IPC_NOWAIT) != -1)
            {
                running = receivedAction.action == ACT_START;
            }
            while (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getpid(), IPC_NOWAIT) != -1)
            {
                pid_t processPID = fork();
                if (processPID == -1)
                {
                    perror("Error creating new process!\n");
                    exit(1);
                }
                else if (processPID == 0)
                {
                    char rt[5];
                    sprintf(rt, "%d", receivedProcess.attachedProcess.runTime);

                    execl("process.out", "process.out", rt, NULL);
                    perror("Error creating new process!\n");
                    exit(1);
                }

                struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));
                /*  GOOD LUCK   */
            }

            currentTime++;
        }
    }
}

/*IMPLEMENT ROUND ROBIN ALGORITHM*/
void Algorithm_RR(int *const quantum, struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB = NULL;
    int nextEvent;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    receivedAction.mType = 0;

    while (running || queue->start)
    {
        if (currentTime < getClk())
        {
            if (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), IPC_NOWAIT) != -1)
            {
                running = receivedAction.action == ACT_START;

            }
            while (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getpid(), IPC_NOWAIT) != -1)
            {

                pid_t processPID = fork();
                if (processPID == -1)
                {
                    perror("Error creating new process!\n");
                    exit(1);
                }
                else if (processPID == 0)
                {
                    char rt[5];
                    sprintf(rt, "%d", receivedProcess.attachedProcess.runTime);

                    execl("process.out", "process.out", rt, NULL);
                    perror("Error creating new process!\n");
                    exit(1);
                }

                struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));
                normalEnqueue_PCB(queue, newPCB);
            }

            if (runningPCB)
            {
                struct Message_Action sentAction;

                if (nextEvent <= currentTime)
                {
                    sentAction.mType = runningPCB->pid;
                    sentAction.time = currentTime;
                    sentAction.action = ACT_STOP;

                    msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                }

                if (receivedAction.mType != runningPCB->pid)
                {
                    msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), runningPCB->pid, IPC_NOWAIT);
                }
                else
                {

                    if (receivedAction.time <= currentTime && receivedAction.action == ACT_START)
                    {
                        runningPCB->status = STAT_RUNNING;
                        runningPCB->waitingTime += receivedAction.time;
                        receivedAction.mType = 0;

                        if (runningPCB->executionTime == 0)
                        {
                            enqueue_Log(*logQueue, createLog(currentTime, EV_STARTED, runningPCB));
                        }
                        else
                        {
                            enqueue_Log(*logQueue, createLog(currentTime, EV_RESUMED, runningPCB));
                        }
                    }
                    else if (receivedAction.time == currentTime && receivedAction.action == ACT_STOP)
                    {
                        runningPCB->status = STAT_READY;
                        runningPCB->executionTime += receivedAction.time;
                        runningPCB->remainingTime -= receivedAction.time;
                        receivedAction.mType = 0;

                        if (runningPCB->remainingTime == 0)
                        {
                            struct Node_PCB *next = runningPCB->next;

                            int pStatus;
                            while (true)
                            {
                                waitpid(runningPCB->pid, &pStatus, 0);
                                if (WIFEXITED(pStatus))
                                {
                                    break;
                                }
                            }

                            free(runningPCB->process);
                            free(remove_PCB(queue, runningPCB));
                            runningPCB = next;
                            nextEvent = currentTime + *quantum;

                            enqueue_Log(*logQueue, createLog(currentTime, EV_FINISHED, runningPCB));
                        }
                        else
                        {
                            runningPCB = runningPCB->next;
                            nextEvent = currentTime + *quantum;

                            enqueue_Log(*logQueue, createLog(currentTime, EV_STOPPED, runningPCB));
                        }

                        if (runningPCB)
                        {
                            sentAction.mType = runningPCB->pid;
                            sentAction.time = currentTime;
                            sentAction.action = ACT_START;

                            msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                        }
                        else
                        {
                            runningPCB = queue->start;
                            nextEvent = currentTime + *quantum;

                            if (runningPCB)
                            {
                                sentAction.mType = runningPCB->pid;
                                sentAction.time = currentTime;
                                sentAction.action = ACT_START;

                                msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                            }
                        }
                    }
                }
            }
            else
            {
                struct Message_Action sentAction;

                runningPCB = queue->start;
                nextEvent = currentTime + *quantum;

                if (runningPCB)
                {
                    sentAction.mType = runningPCB->pid;
                    sentAction.time = currentTime;
                    sentAction.action = ACT_START;

                    msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                }
            }

            currentTime++;
        }
    }
}

/*IMPLEMENT MULTILEVEL FEEDBACK QUEUE ALGORITHM*/
void Algorithm_MLFQ(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, int *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        if (currentTime < getClk())
        {
            if (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), IPC_NOWAIT) != -1)
            {
                running = receivedAction.action == ACT_START;
            }
            while (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getpid(), IPC_NOWAIT) != -1)
            {
                pid_t processPID = fork();
                if (processPID == -1)
                {
                    perror("Error creating new process!\n");
                    exit(1);
                }
                else if (processPID == 0)
                {
                    char rt[5];
                    sprintf(rt, "%d", receivedProcess.attachedProcess.runTime);

                    execl("process.out", "process.out", rt, NULL);
                    perror("Error creating new process!\n");
                    exit(1);
                }

                struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));
                /*  GOOD LUCK   */
            }

            currentTime++;
        }
    }
}
