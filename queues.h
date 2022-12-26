struct Node_Process
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
    struct Node_Process *next;
};

struct Node_Process *createProcess(int _id, int _arrivalTime, int _runTime, int _priority)
{
    struct Node_Process *newProcess = malloc(sizeof(struct Node_Process));
    newProcess->id = _id;
    newProcess->arrivalTime = _arrivalTime;
    newProcess->runTime = _runTime;
    newProcess->priority = _priority;
    newProcess->next = NULL;
    return newProcess;
}

/*====================================================================================================*/

struct Queue_Process
{
    struct Node_Process *start;
    struct Node_Process *end;
};

struct Queue_Process *createQueue_Process()
{
    struct Queue_Process *newQueue = malloc(sizeof(struct Queue_Process));
    newQueue->start = NULL;
    newQueue->end = NULL;
    return newQueue;
}

void enqueue_Process(struct Queue_Process *const queue, struct Node_Process *const process)
{
    if (queue->end == NULL)
    {
        queue->start = process;
        queue->end = process;
    }
    else
    {
        queue->end->next = process;
        queue->end = process;
    }
}

struct Node_Process *dequeue_Process(struct Queue_Process *const queue)
{
    struct Node_Process *process = NULL;
    if (queue->start)
    {
        process = queue->start;
        queue->start = queue->start->next;
        if (queue->start == NULL)
            queue->end = NULL;
    }
    return process;
}

/*====================================================================================================*/
/*====================================================================================================*/
/*====================================================================================================*/
enum Status
{
    STAT_READY = 1,
    STAT_RUNNING,
    STAT_BLOCKED
};

struct Node_PCB
{
    struct Node_Process *process;
    pid_t pid;
    enum Status status;
    int priority;
    int remainingTime;
    int executionTime;
    int waitingTime;
    struct Node_PCB *next;
};

struct Node_PCB *createPCB(pid_t _pid, struct Node_Process *const _process)
{
    struct Node_PCB *newPCB = malloc(sizeof(struct Node_PCB));
    newPCB->process = _process;
    newPCB->pid = _pid;
    newPCB->status = STAT_READY;
    newPCB->priority = _process->priority;
    newPCB->remainingTime = _process->runTime;
    newPCB->executionTime = 0;
    newPCB->waitingTime = 0;
    newPCB->next = NULL;
    return newPCB;
}

/*====================================================================================================*/

struct Queue_PCB
{
    struct Node_PCB *start;
    struct Node_PCB *end;
};

struct Queue_PCB *createQueue_PCB()
{
    struct Queue_PCB *newQueue = malloc(sizeof(struct Queue_PCB));
    newQueue->start = NULL;
    newQueue->end = NULL;
    return newQueue;
}

void normalEnqueue_PCB(struct Queue_PCB *const queue, struct Node_PCB *const PCB)
{
    if (queue->end == NULL)
    {
        queue->start = PCB;
        queue->end = PCB;
    }
    else
    {
        queue->end->next = PCB;
        queue->end = PCB;
    }
}

void priorityEnqueue_PCB(struct Queue_PCB *const queue, struct Node_PCB *const PCB, int priority)
{
    if (queue->end == NULL)
    {
        queue->start = PCB;
        queue->end = PCB;
    }
    else
    {
        struct Node_PCB *current = queue->start;
        while (current->next && current->next->priority <= priority)
        {
            current = current->next;
        }

        PCB->next = current->next;
        current->next = PCB;
    }
}

struct Node_PCB *dequeue_PCB(struct Queue_PCB *const queue)
{
    struct Node_PCB *PCB = NULL;
    if (queue->start)
    {
        PCB = queue->start;
        queue->start = queue->start->next;
        if (queue->start == NULL)
            queue->end = NULL;
    }
    return PCB;
}

struct Node_PCB *remove_PCB(struct Queue_PCB *const queue, struct Node_PCB *const PCB)
{
    if (queue->start == PCB)
    {
        queue->start = queue->start->next;
        if (queue->start == NULL)
            queue->end = NULL;
    }
    else
    {
        struct Node_PCB *current = queue->start;
        while (current->next && current->next != PCB)
        {
            current = current->next;
        }
        if (!current->next)
            return NULL;
        if (current->next == queue->end)
            queue->end = current;
        current->next = current->next->next;
    }
    return PCB;
}

/*====================================================================================================*/
/*====================================================================================================*/
/*====================================================================================================*/
enum Event
{
    EV_STARTED = 1,
    EV_RESUMED,
    EV_STOPPED,
    EV_FINISHED
};

struct Node_Log
{
    int time;
    enum Event event;
    int id;
    int arrivalTime;
    int totalTime;
    int remainingTime;
    int waitingTime;
    struct Node_Log *next;
};

struct Node_Log *createLog(int _time, enum Event _event, struct Node_PCB *const _PCB)
{
    struct Node_Log *newLog = malloc(sizeof(struct Node_Log));
    newLog->time = _time;
    newLog->event = _event;
    newLog->id = _PCB->process->id;
    newLog->arrivalTime = _PCB->process->arrivalTime;
    newLog->totalTime = _PCB->process->runTime;
    newLog->remainingTime = _PCB->remainingTime;
    newLog->waitingTime = _PCB->waitingTime;
    newLog->next = NULL;
    return newLog;
}

/*====================================================================================================*/

struct Queue_Log
{
    struct Node_Log *start;
    struct Node_Log *end;
};

struct Queue_Log *createQueue_Log()
{
    struct Queue_Log *newQueue = malloc(sizeof(struct Queue_Log));
    newQueue->start = NULL;
    newQueue->end = NULL;
    return newQueue;
}

void enqueue_Log(struct Queue_Log *const queue, struct Node_Log *const log)
{
    if (queue->end == NULL)
    {
        queue->start = log;
        queue->end = log;
    }
    else
    {
        queue->end->next = log;
        queue->end = log;
    }
}

struct Node_Log *dequeue_Log(struct Queue_Log *const queue)
{
    struct Node_Log *log = NULL;
    if (queue->start)
    {
        log = queue->start;
        queue->start = queue->start->next;
        if (queue->start == NULL)
            queue->end = NULL;
    }
    return log;
}