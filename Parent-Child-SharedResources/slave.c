#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include "myShm.h"

int main(int argc, char const *argv[])
{
    struct CLASS myShmPtr;
    int childNumber = atoi(argv[1]); // converts child number into an integer
    char nameArg[100];
    strcpy(nameArg, argv[2]); // copies name of shared memory
    char n[50] = "/";
    strcat(n, nameArg);    // appends shared memory name to a path of the current directory
    const char *name = n;  /* file name */
    const int SIZE = 4096; /* file size */

    /* Slave Console Output */
    printf("Slave begins execution\n");
    printf("I am child number %d, received shared memory name %s \n", childNumber, nameArg);
    int shm_fd;     /* file descriptor, from shm_open()*/
    char *shm_base; /* base address, from mmap() */
    char *ptr;      /* shm_base is fixed, ptr is movable*/
    /* create the shared memory segment */
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        printf("prod : Shared memory failed : %s\n", strerror(errno));
        exit(1);
    }
    /* configure the size of shared memory segment */
    ftruncate(shm_fd, SIZE);
    /* map shared memory segment in the address space of the process */
    shm_base = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,
                    0);
    if (shm_base == MAP_FAILED)
    {
        printf("prod : Map failed : %s\n", strerror(errno));
        /* close and shm_unlink */
        exit(1);
    }

    /*
     * Now write to the shared memory region.
     * Note we must increment value of ptr after each write.
     */
    ptr = shm_base;                   // initialize ptr to the base of shared memory
    myShmPtr.index = childNumber - 1; // sets index position to the child number
    int index = myShmPtr.index;       // change index of response array
    char response[1];
    for (int i = 0; i < 10; i++)
    {
        sprintf(response, "%d", shm_base[i]);       // converts shared memory character element into integer
        myShmPtr.response[i] = atoi(response) - 48; // stores integer into an array
    }
    myShmPtr.response[index] = childNumber;
    printf("I have written my child number to slot %d and updated index to %d. \n", myShmPtr.index, myShmPtr.index + 1);
    char responseString[300];
    for (int i = 0; i < index + 1; i++)
    {
        sprintf(responseString, "%d", myShmPtr.response[i]); // converts the response element into a string
        sprintf(ptr, "%s", responseString);                  // writes the string to the shared memory
        ptr += strlen(responseString);                       // increments the pointer memory to the next slot
    }
    /* remove the mapped memory segment from the address space of the
    process */
    if (munmap(shm_base, SIZE) == -1)
    {
        printf("prod: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }
    /* close shared memory segment as if it was a file */
    if (close(shm_fd) == -1)
    {
        printf("prod: Close failed: %s\n",
               strerror(errno));
        exit(1);
    }
    printf("Child %d closed access to shared memory and terminates.\n", childNumber);
    return 0;
}
