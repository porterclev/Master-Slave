#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include "myShm.h"
void display(char *prog, char *bytes, int n);

int main(int argc, char const *argv[])
{
    // check if there is enough arguments
    if (argc <= 1)
    {
        printf("Not enough arguments\n");
        return 1;
    }
    int slaveCount = atoi(argv[1]) + 1; // initializes slave count
    char nameArg[100];
    strcpy(nameArg, argv[2]); // copies name of shared memory
    char n[50] = "/";
    strcat(n, nameArg);    // appends shared memory to a path of the current directory
    const char *name = n;  /* file name */
    const int SIZE = 4096; /* file size */
    int shm_fd;            /* file descriptor, from shm_open() */
    char *shm_base;        /* base address, from mmap() */

    /* Master Console Output */
    printf("Master begins execution\n");
    printf("Master created a shared memory segment named %s \n", nameArg);
    printf("Master created %d child processes execute slave \n", slaveCount - 1);
    printf("Master waits for all child processes to terminate\n");
    struct CLASS myShmPtr;
    myShmPtr.index = 0;
    for (int i = 0; i < 10; i++)
        myShmPtr.response[i] = 0;

    for (int i = 1; i < slaveCount; i++)
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
            char str[10];                                   // char array to hold integer
            snprintf(str, sizeof(str), "%d", i);            // prints integer onto a char array
            execlp("./slave", "slave", str, nameArg, NULL); // executes child process, passing in child number and name
                                                            /* read from the mapped shared memory segment */
        }
        /* parent process */
        else
        {
            wait(NULL); // wait for child process status to return 0
        }
    }
    /* Master Console Output */
    printf("Master received termination signals from all %d child processes\n", slaveCount - 1);
    printf("Updated content of shared memory segment after access by child processes:\n");
    /* open the shared memory segment as if it was a file */
    shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1)
    {
        printf("cons: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }
    /* map the shared memory segment to the address space
    of the process */
    shm_base = mmap(0, SIZE, PROT_READ, MAP_SHARED,
                    shm_fd, 0);
    if (shm_base == MAP_FAILED)
    {
        printf("cons: Map failed: %s\n", strerror(errno));
        /* close and unlink */
        exit(1);
    }
    /* Display Current Message in Shared Memory */
    printf("Master Display: %s\n", shm_base);

    /* remove the mapped shared memory segment from the
    address space of the process */
    if (munmap(shm_base, SIZE) == -1)
    {
        printf("cons: Unmap failed: %s\n",
               strerror(errno));
        exit(1);
    }
    printf("Master removed shared memory segment, and its exiting\n");
    /* close the shared memory segment as if it was a file */
    if (close(shm_fd) == -1)
    {
        printf("cons: Close failed: %s\n", strerror(errno));
        exit(1);
    }
    /* remove the shared memory segment from the file system */
    if (shm_unlink(name) == -1)
    {
        printf("cons: Error removing %s: %s\n", name, strerror(errno));
        exit(1);
    }
    return 0;
}
