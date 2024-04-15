#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
using namespace std;

struct mesgb
{
    long mtype;        // required
    char greeting[50]; // message content
};

int main(int argc, char const *argv[])
{
    int qid = atoi(argv[1]);
    mesgb msg;
    int size = sizeof(msg) - sizeof(long);
    msgrcv(qid, (struct mesgb *)&msg, size, 114, 0);
    cout << "Reciever, PID " << getpid() << ", begins execution" << endl;
    cout << "Reciever with PID " << getpid() << " received message queue id " << qid << " through commandline argument" << endl;
    cout << "Reciever with PID " << getpid() << " retrieved the following message from message queue " << endl;
    msg.mtype = 115;
    cout << msg.greeting << endl;
    cout << "Receiver with PID " << getpid() << " sent the following acknowledgement message into message queue" << endl;
    char message[50] = "Receiver with PID ";
    char s[10];
    snprintf(s, sizeof(s), "%d", getpid());
    strcpy(msg.greeting, message);
    strcat(msg.greeting, s);
    strcat(msg.greeting, " acknowledges receipt of message");
    cout << msg.greeting << endl;
    msgsnd(qid, (struct mesgb *)&msg, size, 0);
    // cout << "Receiver with PID " << getpid() << " acknowledges receipt of message" << endl;
    cout << "Receiver with PID " << getpid() << " terminates" << endl;
    exit(0);
}
