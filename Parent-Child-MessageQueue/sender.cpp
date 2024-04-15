#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
using namespace std;
// declare my global message buffer
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
    cout << "Sender, PID " << getpid() << ", begins execution" << endl;
    cout << "Sender with PID " << getpid() << " received message queue id " << qid << " through commandline argument" << endl;
    cout << "Sender with PID " << getpid() << " Please input your message: " << endl;
    string message;
    getline(cin, message);
    cout << "Sender with PID " << getpid() << " sends the following message into the message queue" << endl;
    strcpy(msg.greeting, message.c_str());
    msg.mtype = 114;
    msgsnd(qid, (struct mesgb *)&msg, size, 0);
    cout << msg.greeting << endl;
    msgrcv(qid, (struct mesgb *)&msg, size, 115, 0);
    cout << "Sender with PID " << getpid() << " receives the following acknowledgement message" << endl;
    cout << msg.greeting << endl;
    cout << "Sender with PID " << getpid() << " terminates" << endl;
    exit(0);
}
