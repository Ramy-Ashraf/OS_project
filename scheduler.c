#include "headers.h"

int msgqID;

void Algorithm_SJF(struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber);
void Algorithm_HPF(struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber);
void Algorithm_RR(int* const quantum, struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber);
void Algorithm_MLFQ(struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber);

int main(int argc, char *argv[])
{
    // Receive arguments
    enum Algorithm schedulingAlgorithm = (enum Algorithm) atoi(argv[1]);
    int quantum = atoi(argv[2]);

    //CPU Parameters
    struct Queue_Log* logQueue = createQueue_Log();
    int processorIdleTime;
    int processWaitingTime;
    int processWeightedTurnaround;
    int processNumber;

    // Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);

    //TODO: implement the scheduler.
    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT); 
    if(msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    //TODO: upon termination release the clock resources.
    msgctl(msgqID, IPC_RMID, (struct msqid_ds*) 0);
    destroyClk(true);
    exit(0);
}

/*IMPLEMENT SHORTEST JOB FIRST ALGORITHM*/
void Algorithm_SJF(struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber)
{
    struct Queue_PCB* queue = createQueue_PCB();

    struct Message msg;

    if (msgrcv(msgqID, &msg, sizeof(msg)-sizeof(msg.mType), getpid(), IPC_NOWAIT) != -1)
    {
        struct Node_PCB* newPCB = createPCB(0, createProcess(msg.attachedProcess.id, msg.attachedProcess.arrivalTime, msg.attachedProcess.runTime, msg.attachedProcess.priority));
        /*  GOOD LUCK   */
    }
}

/*IMPLEMENT HIGHEST PRIORITY FIRST ALGORITHM*/
void Algorithm_HPF(struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber)
{
    struct Queue_PCB* queue = createQueue_PCB();

    struct Message msg;

    if (msgrcv(msgqID, &msg, sizeof(msg)-sizeof(msg.mType), getpid(), IPC_NOWAIT) != -1)
    {
        struct Node_PCB* newPCB = createPCB(0, createProcess(msg.attachedProcess.id, msg.attachedProcess.arrivalTime, msg.attachedProcess.runTime, msg.attachedProcess.priority));
        /*  GOOD LUCK   */
    }
}

/*IMPLEMENT ROUND ROBIN ALGORITHM*/
void Algorithm_RR(int* const quantum, struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber)
{
    struct Queue_PCB* queue = createQueue_PCB();

    struct Message msg;

    if (msgrcv(msgqID, &msg, sizeof(msg)-sizeof(msg.mType), getpid(), IPC_NOWAIT) != -1)
    {
        struct Node_PCB* newPCB = createPCB(0, createProcess(msg.attachedProcess.id, msg.attachedProcess.arrivalTime, msg.attachedProcess.runTime, msg.attachedProcess.priority));
        /*  GOOD LUCK   */
    }
}

/*IMPLEMENT MULTILEVEL FEEDBACK QUEUE ALGORITHM*/
void Algorithm_MLFQ(struct Queue_Log** const logQueue, int* const processorIdleTime, int* const processWaitingTime, int* const processWeightedTurnaround, int* const processNumber)
{
    struct Queue_PCB* queue = createQueue_PCB();

    struct Message msg;

    if (msgrcv(msgqID, &msg, sizeof(msg)-sizeof(msg.mType), getpid(), IPC_NOWAIT) != -1)
    {
        struct Node_PCB* newPCB = createPCB(0, createProcess(msg.attachedProcess.id, msg.attachedProcess.arrivalTime, msg.attachedProcess.runTime, msg.attachedProcess.priority));
        /*  GOOD LUCK   */
    }
}
