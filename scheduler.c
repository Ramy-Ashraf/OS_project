#include "headers.h"

void passTime(int signum);
void clearResources(int signum);

int msgqID;
int currentTime = 0;
int clkTime = 0;
int skipTime = 0;
bool running = false;

void Algorithm_SJF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber);
void Algorithm_HPF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber);
void Algorithm_RR(int *const quantum, struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber);
void Algorithm_MLFQ(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber);

int main(int argc, char *argv[])
{
    signal(SIGUSR1, passTime);
    signal(SIGINT, clearResources);

    // Receive arguments
    enum Algorithm schedulingAlgorithm = (enum Algorithm)atoi(argv[1]);
    int quantum = atoi(argv[2]);

    // CPU Parameters
    struct Queue_Log *logQueue = createQueue_Log();
    int processorIdleTime = 0;
    int processWaitingTime = 0;
    float processWeightedTurnaround = 0;
    int processNumber = 0;

    // Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    printf("Current Time is %d\n", getClk());

    // TODO: implement the scheduler.
    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT);
    if (msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    // Wait for "start" action sent by Process Generator before continuing
    struct Message_Action receivedAction;
    msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), !IPC_NOWAIT);

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
        if (strcmp(event, "finished") == 0)
        {
            printf(" TA %d WTA %.2f\n", logQueue->start->totalTime + logQueue->start->waitingTime, ((float)(logQueue->start->totalTime + logQueue->start->waitingTime) / logQueue->start->totalTime));
        }
        else
        {
            printf("\n");
        }

        free(dequeue_Log(logQueue));
    }

    free(logQueue);

    printf("\n");
    printf("CPU utilization = %.2f%%\n", ((float)(currentTime - processorIdleTime) * 100 / currentTime));
    printf("Avg WTA = %.2f\n", ((float)processWeightedTurnaround / processNumber));
    printf("Avg Waiting = %.2f\n", ((float)processWaitingTime / processNumber));

    // TODO: upon termination release the clock resources.
    destroyClk(false);
    exit(0);
}

void passTime(int signum)
{
    clkTime++;
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    destroyClk(true);
    exit(0);
}

/*IMPLEMENT SHORTEST JOB FIRST ALGORITHM*/
void Algorithm_SJF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB = NULL;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        bool stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        while (!stopped)
        {
            if (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getppid(), IPC_NOWAIT) == -1)
            {
                break;
            }

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

            struct Message_Action sentAction;
            struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));

            /*Change this line as needed*/
            normalEnqueue_PCB(queue, newPCB);

            sentAction.mType = newPCB->pid;
            sentAction.time = currentTime;
            sentAction.action = ACT_STOP;

            msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

            stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        }

        if (stopped)
        {
            running = receivedAction.action == ACT_START;
        }

        if (currentTime < clkTime)
        {

            currentTime++;
        }

        if (clkTime < getClk())
        {
            if (skipTime == 0)
            {
                kill(getppid(), SIGUSR1);
                for (struct Node_PCB *i = queue->start; i != NULL; i = i->next)
                {
                    kill(i->pid, SIGUSR1);
                }

                skipTime = getClk() + CYCLE_SKIPS;
            }
            else 
            {
                if (skipTime <= getClk())
                {
                    passTime(SIGUSR1);
                    skipTime = 0;
                }
            }
        }
    }
    free(queue);
}

/*IMPLEMENT HIGHEST PRIORITY FIRST ALGORITHM*/
void Algorithm_HPF(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB = NULL;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        bool stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        while (!stopped)
        {
            if (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getppid(), IPC_NOWAIT) == -1)
            {
                break;
            }

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

            struct Message_Action sentAction;
            struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));

            /*Change this line as needed*/
            normalEnqueue_PCB(queue, newPCB);

            sentAction.mType = newPCB->pid;
            sentAction.time = currentTime;
            sentAction.action = ACT_STOP;

            msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

            stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        }

        if (stopped)
        {
            running = receivedAction.action == ACT_START;
        }

        if (currentTime < clkTime)
        {

            currentTime++;
        }

        if (clkTime < getClk())
        {
            if (skipTime == 0)
            {
                kill(getppid(), SIGUSR1);
                for (struct Node_PCB *i = queue->start; i != NULL; i = i->next)
                {
                    kill(i->pid, SIGUSR1);
                }

                skipTime = getClk() + CYCLE_SKIPS;
            }
            else 
            {
                if (skipTime <= getClk())
                {
                    passTime(SIGUSR1);
                    skipTime = 0;
                }
            }
        }
    }
    free(queue);
}

/*IMPLEMENT ROUND ROBIN ALGORITHM*/
void Algorithm_RR(int *const quantum, struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB = NULL;
    int nextEvent = -1;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        bool stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        while (!stopped)
        {
            if (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getppid(), IPC_NOWAIT) == -1)
            {
                break;
            }

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

            struct Message_Action sentAction;
            struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));

            /*Change this line as needed*/
            normalEnqueue_PCB(queue, newPCB);

            sentAction.mType = newPCB->pid;
            sentAction.time = currentTime;
            sentAction.action = ACT_STOP;

            msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

            stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        }

        if (stopped)
        {
            running = receivedAction.action == ACT_START;
        }

        if (currentTime < clkTime)
        {

            currentTime++;
            nextEvent--;
        }

        if (clkTime < getClk())
        {
            if (skipTime == 0)
            {
                kill(getppid(), SIGUSR1);
                for (struct Node_PCB *i = queue->start; i != NULL; i = i->next)
                {
                    kill(i->pid, SIGUSR1);
                }

                skipTime = getClk() + CYCLE_SKIPS;
            }
            else 
            {
                if (skipTime <= getClk())
                {
                    passTime(SIGUSR1);
                    skipTime = 0;
                }
            }
        }
    }
    free(queue);
}

/*IMPLEMENT MULTILEVEL FEEDBACK QUEUE ALGORITHM*/
void Algorithm_MLFQ(struct Queue_Log **const logQueue, int *const processorIdleTime, int *const processWaitingTime, float *const processWeightedTurnaround, int *const processNumber)
{
    struct Queue_PCB *queue = createQueue_PCB();
    struct Node_PCB *runningPCB = NULL;
    int nextEvent = -1;

    int *const quantum = malloc(sizeof(int));
    (*quantum) = 2;

    struct Message_Action receivedAction;
    struct Message_Process receivedProcess;

    while (running || queue->start)
    {
        bool stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        while (!stopped)
        {
            if (msgrcv(msgqID, &receivedProcess, sizeof(receivedProcess.attachedProcess), getppid(), IPC_NOWAIT) == -1)
            {
                break;
            }

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

            struct Message_Action sentAction;
            struct Node_PCB *newPCB = createPCB(processPID, createProcess(receivedProcess.attachedProcess.id, receivedProcess.attachedProcess.arrivalTime, receivedProcess.attachedProcess.runTime, receivedProcess.attachedProcess.priority));
            normalEnqueue_PCB(queue, newPCB);

            sentAction.mType = newPCB->pid;
            sentAction.time = currentTime;
            sentAction.action = ACT_STOP;

            msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

            stopped = (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getppid(), IPC_NOWAIT) != -1);
        }

        if (stopped)
        {
            running = receivedAction.action == ACT_START;
        }

        if (currentTime < clkTime)
        {
            if (runningPCB)
            {
                if (nextEvent == 0)
                {
                    struct Message_Action sentAction;

                    sentAction.mType = runningPCB->pid;
                    sentAction.time = currentTime;
                    sentAction.action = ACT_STOP;

                    msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                }

                if (msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), nextEvent == 0 ? !IPC_NOWAIT : IPC_NOWAIT) != -1)
                {
                    runningPCB->status = STAT_READY;
                    runningPCB->executionTime += receivedAction.time;
                    runningPCB->remainingTime -= receivedAction.time;

                    struct Node_PCB *next = runningPCB->next;

                    if (runningPCB->remainingTime == 0)
                    {
                        int pStatus;
                        while (true)
                        {
                            waitpid(runningPCB->pid, &pStatus, 0);
                            if (WIFEXITED(pStatus))
                            {
                                break;
                            }
                        }

                        enqueue_Log(*logQueue, createLog(currentTime, EV_FINISHED, runningPCB));

                        (*processWaitingTime) += runningPCB->waitingTime;
                        (*processWeightedTurnaround) += (float)((runningPCB->executionTime + runningPCB->waitingTime) / runningPCB->executionTime);
                        (*processNumber)++;

                        free(runningPCB->process);
                        free(remove_PCB(queue, runningPCB));
                    }
                    else
                    {
                        enqueue_Log(*logQueue, createLog(currentTime, EV_STOPPED, runningPCB));
                    }

                    if (!next)
                    {
                        next = queue->start;
                    }

                    if (next)
                    {
                        struct Message_Action sentAction;

                        runningPCB = next;
                        nextEvent = *quantum;

                        sentAction.mType = runningPCB->pid;
                        sentAction.time = currentTime;
                        sentAction.action = ACT_START;

                        msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                        msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), !IPC_NOWAIT);

                        runningPCB->status = STAT_RUNNING;
                        runningPCB->waitingTime += receivedAction.time;

                        if (runningPCB->executionTime == 0)
                        {
                            enqueue_Log(*logQueue, createLog(currentTime, EV_STARTED, runningPCB));
                        }
                        else
                        {
                            enqueue_Log(*logQueue, createLog(currentTime, EV_RESUMED, runningPCB));
                        }
                    }
                }
            }
            else
            {
                if (queue->start)
                {
                    struct Message_Action sentAction;

                    runningPCB = queue->start;
                    nextEvent = *quantum;

                    sentAction.mType = runningPCB->pid;
                    sentAction.time = currentTime;
                    sentAction.action = ACT_START;

                    msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);
                    msgrcv(msgqID, &receivedAction, sizeof(receivedAction.time) + sizeof(receivedAction.action), getpid(), !IPC_NOWAIT);

                    runningPCB->status = STAT_RUNNING;
                    runningPCB->waitingTime += receivedAction.time;

                    if (runningPCB->executionTime == 0)
                    {
                        enqueue_Log(*logQueue, createLog(currentTime, EV_STARTED, runningPCB));
                    }
                    else
                    {
                        enqueue_Log(*logQueue, createLog(currentTime, EV_RESUMED, runningPCB));
                    }
                }
                else
                {
                    (*processorIdleTime)++;
                }
            }

            currentTime++;
            nextEvent--;
        }

        if (clkTime < getClk())
        {
            if (skipTime == 0)
            {
                kill(getppid(), SIGUSR1);
                for (struct Node_PCB *i = queue->start; i != NULL; i = i->next)
                {
                    kill(i->pid, SIGUSR1);
                }

                skipTime = getClk() + CYCLE_SKIPS;
            }
            else 
            {
                if (skipTime <= getClk())
                {
                    passTime(SIGUSR1);
                    skipTime = 0;
                }
            }
        }
    }
    free(queue);
}
