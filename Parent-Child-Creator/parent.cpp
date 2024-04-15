#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
using namespace std;

/// @brief A parent process that forks a child process for each argument passed in
/// @param argc number of arguments
/// @param argv array of names of children
int main(int argc, char *argv[])
{
    // Check for correct number of arguments
    if (argc < 2)
    {
        cout << "Too few arguments" << endl;
        exit(1);
    }
    cout << "I have " << argc - 1 << " children" << endl; // print out number of children

    // loop through arguments and fork a child process for each
    for (int i = 1; i < argc; i++)
    {
        // initialize child pid
        pid_t pid;

        /* fork a child process */
        pid = fork();

        /* fork error */
        if (pid < 0)
        {
            fprintf(stderr, "fork failed");
            return 1;
        }
        /* child process */
        else if (pid == 0)
        {
            char str[10];
            snprintf(str, sizeof(str), "%d", i);            // prints integer onto a char array
            execlp("./child", "child", str, argv[i], NULL); // executes child process, passing in child number and name
            // execlp("./child", "child", to_string(i).c_str(), argv[i], NULL);
        }
        /* parent process */
        else
        {
            wait(NULL); // wait for child process status to return 0
        }
    }
    // completion message
    printf("All child processes terminated. Parent exits.\n");

    return 0;
}
