#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();

    //TODO: implement the scheduler.
    key_t key_id;
    int send_val;
    struct msgbuff message;
    key_id = ftok("keyfile", 65); 
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT); 
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    //TODO: upon termination release the clock resources.
    destroyClk(true);
}
