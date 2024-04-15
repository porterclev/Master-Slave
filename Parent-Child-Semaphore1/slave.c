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

int main(int argc, char const *argv[])
{
    const int childNumber = atoi(argv[1]); /* child number from commandline */
    const char *shmName = argv[2];         /* shared memory name from commandline */
    const char *semName = argv[3];         /* semaphore name from commandline */
    const int SIZE = sizeof(struct CLASS);

    printf("Slave begins execution\n");
    printf("I am child number %d, received shared memory name %s and %s\n", childNumber, shmName, semName);
    // [ x, /shm_name & /sem_name are from exec() ]
    int shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666); // opens shared memory
    if (shm_fd == -1)                                       // checks shared memory opened
    {
        printf("master: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }
    ftruncate(shm_fd, SIZE);
    sem_t *mutex_sem = sem_open(semName, O_CREAT, 0660, 1); // opens semaphore
    if (mutex_sem == SEM_FAILED)                            // makes sure semaphore seg is opened
    {
        printf("master: sem_open failed: %s\n", strerror(errno));
        exit(1);
    }
    /* lock mutex_sem for critical section */
    if (sem_wait(mutex_sem) == -1) // checks that semaphore is usable
    {
        printf("child %d: sem_wait failed: %s/n", childNumber, strerror(errno));
        exit(1);
    }
    struct CLASS *shm_ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // maps struct to shared memory
    if (shm_ptr == MAP_FAILED)
    {
        printf("slave: Map failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("Child number %d: What is my lucky number? ", childNumber);
    int luckyNum;
    scanf("%d", &luckyNum);
    shm_ptr->response[shm_ptr->index] = childNumber;
    shm_ptr->response[shm_ptr->index + 1] = luckyNum;
    //[ Child x prompts user to input a lucky number ][ User inputs a number in response to the above prompt ]
    printf("I have written my child number to slot %d and my lucky number to slot %d, and updated index to %d.\n", shm_ptr->index, (shm_ptr->index) + 1, (shm_ptr->index) + 2);
    shm_ptr->index += 2;
    // [y is the value in index at time of access ]
    printf("Child %d closed access to shared memory and terminates.\n", childNumber);

    /* done with semaphore, close it & free up resources allocated to it */
    if (sem_post(mutex_sem) == -1) // checks if semaphore is closed
    {
        printf("slave: sem_post failed: %s\n", strerror(errno));
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
    return 0;
}
