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
        printf("slave: sem_wait(display_sem) failed: %s/n", strerror(errno));
        exit(1);
    }
}

/// @brief sets a named semaphore to post (opens)
/// @param display_sem
void namedSemPost(sem_t *display_sem)
{
    if (sem_post(display_sem) == -1)
    {
        printf("slave: sem_post(display_sem) failed: %s\n", strerror(errno));
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
        printf("slave: sem_wait(count_ptr->mutex_sem) failed: %s/n", strerror(errno));
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
        printf("slave: sem_post(count_ptr->mutex_sem) failed: %s\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    const int childNumber = atoi(argv[1]); /* child number from commandline */
    const char *shmName = argv[2];         /* shared memory name from commandline */
    const char *semName = argv[3];         /* semaphore name from commandline */
    const int SIZE = sizeof(struct CLASS);

    int shm_fd = shm_open(shmName, O_RDWR, 0666); // opens shared memory
    if (shm_fd == -1)                             // checks shared memory opened
    {
        printf("slave: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    /* create a named semaphore for displaying output */
    sem_t *display_sem = sem_open(semName, O_RDWR, 0660, 1);
    if (display_sem == SEM_FAILED)
    {
        printf("slave: sem_open failed: %s\n", strerror(errno));
        exit(1);
    }

    namedSemWait(display_sem);
    printf("Slave begins execution\n");
    printf("I am child number %d, received shared memory name %s and %s\n", childNumber, shmName, semName);
    namedSemPost(display_sem);

    struct CLASS *shm_ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // maps struct to shared memory
    if (shm_ptr == MAP_FAILED)
    {
        printf("slave: Map failed: %s\n", strerror(errno));
        exit(1);
    }

    /* acquire access to monitor for output */
    namedSemWait(display_sem);
    printf("Child number %d: What is my lucky number? ", childNumber);

    /* done with output. unlock monitor */
    namedSemPost(display_sem);

    /* lock unnamed semaphore */
    unnamedSemWait(shm_ptr);

    int luckyNum;
    scanf("%d", &luckyNum);
    shm_ptr->response[shm_ptr->index] = childNumber;
    shm_ptr->response[shm_ptr->index + 1] = luckyNum;

    /* unlock unnamed semaphore */
    unnamedSemPost(shm_ptr);

    /* acquire access to monitor for output */
    namedSemWait(display_sem);

    printf("I have written my child number to slot %d and my lucky number to slot %d, and updated index to %d.\n", shm_ptr->index, (shm_ptr->index) + 1, (shm_ptr->index) + 2);
    printf("Child %d closed access to shared memory and terminates.\n", childNumber);

    /* done with output. unlock monitor */
    namedSemPost(display_sem);

    /* lock unnamed semaphore */
    unnamedSemWait(shm_ptr);

    shm_ptr->index += 2;
    /* unlock unnamed semaphore */
    unnamedSemPost(shm_ptr);

    /* remove mapped memory segment from the address space */
    if (munmap(shm_ptr, SIZE) == -1) // checks if shared mem link closed
    {
        printf("slave: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }

    /* close shared memory segment */
    if (close(shm_fd) == -1) // checks if shared mem seg is closed
    {
        printf("slave: Close failed: %s\n", strerror(errno));
        exit(1);
    }
    return 0;
}
