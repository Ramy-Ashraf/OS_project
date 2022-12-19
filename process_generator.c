#include "headers.h"

void clearResources(int);

int msgqID;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    struct Queue_Process* processQueue = createQueue_Process();
    
    {
        if(argc < 2)
        {
            perror("Error! Please make sure you added a correct input file name as your first argument\n");
            exit(1);
        }

        FILE* inputFile = fopen(argv[1], "r");
        if (!inputFile)
        {
            perror("Error! Please make sure you added a correct input file name as your first argument\n");
            exit(1);
        }

        int id;
        int arrivalTime;
        int runTime;
        int priority;

        while(true)
        {
            int result = scanf("%d\t%d\t%d\t%d", &id, &arrivalTime, &runTime, &priority);

            if(result == -1)
            {
                break;
            }
            else if (result == 4)
            {
                enqueue_Process(processQueue, createProcess(id, arrivalTime, runTime, priority));
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
        if(argc < 4)
        {
            perror("Error! Please make sure you chose a scheduling algorithm\n");
            exit(1);
        }
        if(argv[2] != "-sch")
        {
            perror("Error! Please make sure you chose a scheduling algorithm\n");
            exit(1);
        }

        int sch = atoi(argv[3]);
        if( sch < (int) ALGO_SJF || sch > (int) ALGO_MLFQ)
        {
            perror("Error! Please make sure you chose a correct scheduling algorithm\n");
            exit(1);
        }

        schedulingAlgorithm = (enum Algorithm) sch;
        if (schedulingAlgorithm == ALGO_RR)
        {
            if(argc < 6)
            {
                perror("Error! Please specify a quantum size for Round Robin Algorithm\n");
                exit(1);
            }
            if(argv[4] != "-q")
            {
                perror("Error! Please specify a quantum size for Round Robin Algorithm\n");
                exit(1);
            }

            int q = atoi(argv[5]);
            if(q <= 0)
            {
                perror("Error! Please specify a correct quantum size for Round Robin Algorithm\n");
                exit(1);
            }
            quantum = q;
        }       
    }

    // 3. Initiate and create the scheduler and clock processes.
    pid_t clockPID = fork();
    if(clockPID == -1)
    {
        perror("Error creating clock process!\n");
        exit(1);
    }
    else if(clockPID == 0)
    {
        execl("clk.out", "clk.out", NULL);
        perror("Error creating clock process!\n");
        exit(1);
    }

    pid_t schedulerPID = fork();
    if(schedulerPID == -1)
    {
        perror("Error creating scheduler process!\n");
        exit(1);
    }
    if(schedulerPID == 0)
    {
        execl("scheduler.out", "scheduler.out", argv[3], schedulingAlgorithm == ALGO_RR ? argv[5] : "-1", NULL);
        perror("Error creating scheduler process!\n");
        exit(1);
    }

    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    /*DONE WHILE READING THE FILE DURING REQUIREMENT#1*/

    // 6. Send the information to the scheduler at the appropriate time.  
    key_t msgqKey = ftok("keyfile.txt", MQKEY);
    msgqID = msgget(msgqKey, 0666 | IPC_CREAT); 
    if(msgqID == -1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    struct Message msg;

    while (processQueue->start)
    {
        while(processQueue->start->arrivalTime < getClk());
        msg.mType = schedulerPID;
        msg.action = ACT_NOACT;
        msg.attachedProcess.id = processQueue->start->id;
        msg.attachedProcess.arrivalTime = processQueue->start->arrivalTime;
        msg.attachedProcess.runTime = processQueue->start->runTime;
        msg.attachedProcess.priority = processQueue->start->priority;
        free(dequeue_Process(processQueue));

        msgsnd(msgqID, &msg, sizeof(msg)-sizeof(msg.mType), !IPC_NOWAIT);
    }

    free(processQueue);

    // 7. Clear clock resources
    int pStatus;
    while(true)
    {
        waitpid(schedulerPID, &pStatus, 0);
        if (WIFEXITED(pStatus))
        {
            break;
        } 
    }
    
    msgctl(msgqID, IPC_RMID, (struct msqid_ds*) 0);
    destroyClk(true);
    exit(0);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    msgctl(msgqID, IPC_RMID, (struct msqid_ds*) 0);
    destroyClk(true);
}
