#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
using namespace std;

int main(int argc, char const *argv[])
{
    // check if there is enough arguments
    if (argc <= 1)
    {
        cout << "Not enough arguments" << endl;
        return 1;
    }

    // message queue initialization
    int qid = msgget(IPC_PRIVATE, IPC_EXCL | IPC_CREAT | 0600);

    /* initialization message */
    cout << "Master, PID " << getpid() << ", begins execution" << endl;
    cout << "Master acquired a message queue, id " << qid << endl;
    cout << "Master created " << atoi(argv[1]) << " child processes to serve as sender" << endl;
    cout << "Master created " << atoi(argv[1]) << " child processes to serve as receiver" << endl;
    cout << "Master waits for all child processes to terminate" << endl;

    // loop through each sender and receiver interaction
    for (int i = 0; i < atoi(argv[1]); i++)
    {
        /* initializes sender process */
        pid_t spid = fork();
        if (spid == 0)
        {
            char str[10];
            snprintf(str, sizeof(str), "%d", qid);   // prints integer onto a char array
            execlp("./sender", "sender", str, NULL); // executes sender.cpp w/ qid as argument
        }

        /* initializes receiver process */
        pid_t rpid = fork();
        if (rpid == 0)
        {
            char str2[10];
            snprintf(str2, sizeof(str2), "%d", qid);      // prints integer onto a char array
            execlp("./reciever", "reciever", str2, NULL); // execute reciever.cpp w/ qid as argument
        }

        /* Waits for both children to complete execution */
        while (wait(NULL) != -1)
            ;
    }
    msgctl(qid, IPC_RMID, NULL); // removes message queue

    // master termination message
    cout << "Master received termination signals from all child processes, removed message queue, and terminates" << endl;
    return 0;
}
