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

/// @brief sets a named semaphore to wait (locks)
/// @param display_sem
void namedSemWait(sem_t *display_sem)
{
    if (sem_wait(display_sem) == -1)
    {
        printf("master: sem_wait(display_sem) failed: %s/n", strerror(errno));
        exit(1);
    }
}

/// @brief sets a named semaphore to post (opens)
/// @param display_sem
void namedSemPost(sem_t *display_sem)
{
    if (sem_post(display_sem) == -1)
    {
        printf("master: sem_post(display_sem) failed: %s\n", strerror(errno));
        exit(1);
    }
}

/// @brief unnamed semaphore wait (locks)
/// @param shm_ptr
void unnamedSemWait(struct CLASS *shm_ptr)
{
    /* lock mutex_sem to initialize shared count */
    if (sem_wait(&(shm_ptr->mutex_sem)) == -1)
    {
        printf("observer: sem_wait(count_ptr->mutex_sem) failed: %s/n", strerror(errno));
        exit(1);
    }
}

/// @brief unnamed semaphore post (opens)
/// @param shm_ptr
void unnamedSemPost(struct CLASS *shm_ptr)
{
    /* unlock mutex_sem */
    if (sem_post(&(shm_ptr->mutex_sem)) == -1)
    {
        printf("observer: sem_post(count_ptr->mutex_sem) failed: %s\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    const char *shmName = argv[2];         /* shared memory name from commandline */
    const char *semName = argv[3];         /* semaphore name from commandline */
    const int SIZE = sizeof(struct CLASS); /* Shared Memory Struct */
    int shm_fd, i;                         // shared memory file descriptor

    /* create a named semaphore for displaying output */
    sem_t *display_sem = sem_open(semName, O_CREAT, 0660, 1);
    if (display_sem == SEM_FAILED)
    {
        printf("observer: sem_open failed: %s\n", strerror(errno));
        exit(1);
    }

    /* creates shared memory seg */
    shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
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

    /* initialize mutex_sem in shared memory to 1, and nanoset it to shared*/
    if (sem_init(&(shm_ptr->mutex_sem), 0, 1) == -1)
    {
        printf("observer: sem_init(count_ptr->mutex_sem) failed: %s/n", strerror(errno));
        exit(1);
    }

    /* acquire access to monitor for output */
    namedSemWait(display_sem);

    printf("Master begins execution\n");
    printf("Master created a shared memory segment named %s\n", shmName);
    printf("Master initializes index in the shared structure to zero\n");
    printf("Master created a semaphore named %s\n", semName);
    printf("Master created n child processes to execute slave\n");
    printf("Master waits for all child processes to terminate\n");

    /* done with output. unlock monitor */
    namedSemPost(display_sem);

    /*  Initialize shared count */
    shm_ptr->index = 0; // initializes response index to 0

    /*
        Fork Process to Children
    */
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

    /* acquire access to monitor for output */
    namedSemWait(display_sem);

    printf("Master recieved termination signal from all %d child processes\n", childCount - 1);
    printf("Updated content of shared memory segment after access by child processes\n");

    /* reading final memory segment */
    for (int i = 0; i < 10; i += 2)
    {
        printf("child number: %d, lucky number: %d\n", shm_ptr->response[i], shm_ptr->response[i + 1]); // gets shared memory response array
    }

    printf("Master removed the semaphore\n");

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

    /* done with output. unlock monitor */
    namedSemPost(display_sem);

    /* done with semaphore, close it & free up resources
   allocated to it */
    if (sem_close(display_sem) == -1) // makes sure semaphore segment closes
    {
        printf("master: sem_close failed: %s\n", strerror(errno));
        exit(1);
    }
    /* done with mutex_sem, destroy it & free up resources
   allocated to it */
    if (sem_destroy(&(count_ptr->mutex_sem)) == -1)
    {
        printf("observer: sem_destroy(count_ptr->mutex_sem) failed: %s\n", strerror(errno));
        exit(1);
    }
    /* request to remove the named semaphore, but action won't
       take place until all references to it have ended */
    if (sem_unlink(semName) == -1)
    {
        printf("master: sem_unlink failed: %s\n", strerror(errno));
        exit(1);
    }
    return 0;
}
