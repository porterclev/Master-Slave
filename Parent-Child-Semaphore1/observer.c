/* observer.c
   compile with gcc, link with -lrt -lpthread
   run with ./observer /myShm /mySem
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <errno.h>
#include "myShm.h"

int main(int argc, char *argv[])
{
    const char *shmName = argv[2];         /* shared memory name from commandline */
    const char *semName = argv[3];         /* semaphore name from commandline */
    const int SIZE = sizeof(struct CLASS); /* Shared Memory Struct */
    printf("Master begins execution\n");
    printf("Master created a shared memory segment named %s\n", shmName);

    int shm_fd, i;

    shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666); // opens shared memory
    if (shm_fd == -1)
    {
        printf("master: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    /* configure size of shared memory segment */
    ftruncate(shm_fd, SIZE);

    /* map shared memory segment in the address space of the process */
    struct CLASS *shm_ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // maps struct to shared memory
    if (shm_ptr == MAP_FAILED)                                                            // checks if shared mem seg is linked
    {
        printf("master: Map failed: %s\n", strerror(errno));
        exit(1);
    }

    /*  Initialize shared count */
    printf("Master initializes index in the shared structure to zero\n");
    shm_ptr->index = 0; // initializes response index to 0

    /* create a named semaphore for mutual exclusion */
    printf("Master created a semaphore named %s\n", semName);
    sem_t *mutex_sem = sem_open(semName, O_CREAT, 0660, 1); // opens shared semaphore
    if (mutex_sem == SEM_FAILED)                            // makes sure semaphore seg is opened
    {
        printf("master: sem_open failed: %s\n", strerror(errno));
        exit(1);
    }
    /*
        Fork Process to Children
    */
    printf("Master created n child processes to execute slave\n");
    int childCount = atoi(argv[1]) + 1; // gets child count from command line
    for (i = 1; i < childCount; i++)
    {
        pid_t pid;
        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "fork failed");
            return 1;
        }
        else if (pid == 0)
        {
            char str[10];                                            // string buffer
            snprintf(str, sizeof(str), "%d", i);                     // buffer is set to child number
            execlp("./slave", "slave", str, shmName, semName, NULL); // calls slave process passing child number, shared memory seg name, shared semaphore name
        }
        else
        {
            wait(NULL); // waits for child process to complete
        }
    }

    /* critical section to access & increment count in shared memory */
    for (int i = 0; i < 10; i += 2)
    {
        printf("child number: %d, lucky number: %d\n", shm_ptr->response[i], shm_ptr->response[i + 1]); // gets shared memory response array
    }

    /* done with semaphore, close it & free up resources
       allocated to it */
    printf("Master removed the semaphore\n");
    if (sem_close(mutex_sem) == -1) // makes sure semaphore segment closes
    {
        printf("master: sem_close failed: %s\n", strerror(errno));
        exit(1);
    }

    /* request to remove the named semaphore, but action won't
       take place until all references to it have ended */
    if (sem_unlink(semName) == -1)
    {
        printf("master: sem_unlink failed: %s\n", strerror(errno));
        exit(1);
    }

    /* remove mapped memory segment from the address space */
    if (munmap(shm_ptr, SIZE) == -1) // checks if shared mem link closed
    {
        printf("master: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }

    /* close shared memory segment */
    if (close(shm_fd) == -1) // checks if shared mem seg is closed
    {
        printf("master: Close failed: %s\n", strerror(errno));
        exit(1);
    }

    /* remove shared memory segment from the file system */
    if (shm_unlink(shmName) == -1)
    {
        printf("master: Error removing %s: %s\n", shmName, strerror(errno));
        exit(1);
    }
    printf("Master closed access to shared memory, removed shared memory segment, and is exiting\n");

    return 0;
}
