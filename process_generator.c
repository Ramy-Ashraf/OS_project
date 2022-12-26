#include "headers.h"

void passTime(int signum);
void clearResources(int signum);

int msgqID;
int currentTime = 0;
int clkTime = 0;

int main(int argc, char *argv[])
{
    signal(SIGUSR1, passTime);
    signal(SIGINT, clearResources);

    // TODO Initialization
    // 1. Read the input files.
    struct Queue_Process *processQueue = createQueue_Process();

    {
        if (argc < 2)
        {
            perror("Error! Please make sure you added a correct input file name as your first argument\n");
            exit(1);
        }

        FILE *inputFile = fopen(argv[1], "r");
        if (!inputFile)
        {
            perror("Error! Please make sure you added a correct input file name as your first argument\n");
            exit(1);
        }

        int id;
        int arrivalTime;
        int runTime;
        int priority;

        while(true) //While loop to read from the file
        {
            int result = fscanf(inputFile, "%d\t%d\t%d\t%d", &id, &arrivalTime, &runTime, &priority);

            if (result == EOF)  //If at end oof file, break out of loop
            {
                break;
            }
            else if (result == 4)   //If line is correct (found 4 integers), enqueue a new process
            {
                enqueue_Process(processQueue, createProcess(id, arrivalTime, runTime, priority));
            }
            else    //Skip this line
            {
                while (fgetc(inputFile) != '\n')
                    ;
            }
        }

        if (processQueue->start == NULL)
        {
            perror("Error! Please make sure you added a correct input file name as your first argument\n");
            exit(1);
        }
    }

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    enum Algorithm schedulingAlgorithm;
    int quantum = -1;

    {
        if (argc < 4)
        {
            perror("Error! Please make sure you chose a scheduling algorithm\n");
            exit(1);
        }
        if (strcmp(argv[2], "-sch") != 0)
        {
            perror("Error! Please make sure you chose a scheduling algorithm\n");
            exit(1);
        }

        int sch = atoi(argv[3]);
        if (sch < (int)ALGO_SJF || sch > (int)ALGO_MLFQ)
        {
            perror("Error! Please make sure you chose a correct scheduling algorithm\n");
            exit(1);
        }

        schedulingAlgorithm = (enum Algorithm)sch;
        if (schedulingAlgorithm == ALGO_RR)
        {
            if (argc < 6)
            {
                perror("Error! Please specify a quantum size for Round Robin Algorithm\n");
                exit(1);
            }
            if (strcmp(argv[4], "-q") != 0)
            {
                perror("Error! Please specify a quantum size for Round Robin Algorithm\n");
                exit(1);
            }

            int q = atoi(argv[5]);
            if (q <= 0)
            {
                perror("Error! Please specify a correct quantum size for Round Robin Algorithm\n");
                exit(1);
            }
            quantum = q;
        }
        else
        {
            if (argc > 4)
            {
                perror("Error! This scheduling algorithm doesn't take any parameters\n");
                exit(1);
            }
        }

        if (argc > 6)
        {
            perror("Error! Too many arguments\n");
            exit(1);
        }
    }

    // 3. Initiate and create the scheduler and clock processes.
    pid_t clockPID = fork();
    if (clockPID == -1)
    {
        perror("Error creating clock process!\n");
        exit(1);
    }
    else if (clockPID == 0)
    {
        execl("clk.out", "clk.out", NULL);
        perror("Error creating clock process!\n");
        exit(1);
    }

    pid_t schedulerPID = fork();
    if (schedulerPID == -1)
    {
        perror("Error creating scheduler process!\n");
        exit(1);
    }
    if (schedulerPID == 0)
    {
        char sch[5];
        sprintf(sch, "%d", schedulingAlgorithm);
        char q[5];
        sprintf(q, "%d", quantum);

        execl("scheduler.out", "scheduler.out", sch, q, NULL);
        perror("Error creating scheduler process!\n");
        exit(1);
    }

    // 4. Use this function after creating the clock process to initialize clock.
    /*INITIALIZED IN SCHEDULER*/

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    /*DONE WHILE READING THE FILE DURING REQUIREMENT#1*/

    // 6. Send the information to the scheduler at the appropriate time.
    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT);
    if (msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    struct Message_Action sentAction;

    sentAction.mType = getpid();
    sentAction.time = currentTime;
    sentAction.action = ACT_START;

    //Send "start" action to scheduler
    msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

    struct Message_Process sentProcess;

    while (processQueue->start) //If process queue is not empty
    {
        if (currentTime < clkTime)  //If the process generator is late and should execute it's code
        {
            while (processQueue->start && processQueue->start->arrivalTime <= currentTime)  //If the current process should be sent out (Arrival time <= Current time)
            {
                sentProcess.mType = getpid();
                sentProcess.attachedProcess.id = processQueue->start->id;
                sentProcess.attachedProcess.arrivalTime = processQueue->start->arrivalTime;
                sentProcess.attachedProcess.runTime = processQueue->start->runTime;
                sentProcess.attachedProcess.priority = processQueue->start->priority;
                free(dequeue_Process(processQueue));

                msgsnd(msgqID, &sentProcess, sizeof(sentProcess.attachedProcess), !IPC_NOWAIT);
            }

            currentTime++;
        }
    }

    //Send "stop" action to scheduler
    sentAction.mType = getpid();
    sentAction.time = currentTime;
    sentAction.action = ACT_STOP;

    msgsnd(msgqID, &sentAction, sizeof(sentAction.time) + sizeof(sentAction.action), !IPC_NOWAIT);

    free(processQueue);

    // 7. Clear clock resources
    int pStatus;
    while (true)
    {
        waitpid(schedulerPID, &pStatus, 0);
        if (WIFEXITED(pStatus))
        {
            break;
        }
    }

    clearResources(SIGINT);
}

void passTime(int signum)
{
    clkTime++;
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    msgctl(msgqID, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    exit(0);
}
