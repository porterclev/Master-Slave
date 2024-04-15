#include <cstdlib>
#include <unistd.h>
#include <iostream>
using namespace std;

/// @brief Child process that prints out its name and number
/// @param argc number of arguments
/// @param argv [command, child number, child name]
int main(int argc, char const *argv[])
{
    /* Process recieved too many arguments */
    if (argc > 3)
    {
        cout << "Too many arguments" << endl;
        exit(1); // updates child process to failed status
    }
    /* Process didn't recieve enough arguments */
    else if (argc < 3)
    {
        cout << "Too few arguments" << endl;
        exit(1); // updates child process to failed status
    }

    // initialize variables
    int child_number = atoi(argv[1]); // converts child number from string to int
    string child_name = argv[2];

    /* child process */
    cout << "I am child number " << child_number << ", and my name is " << child_name << endl;

    /* deallocates child process memory and sets process status to 0 */
    exit(0);
}
