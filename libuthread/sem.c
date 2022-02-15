#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
    size_t count;
    queue_t waiting_q;
};


sem_t sem_create(size_t count)
{
    sem_t my_sem;
    my_sem = malloc(sizeof(sem_t)); //a semaphore object created
    my_sem->count = count;
    my_sem->waiting_q = queue_create(); //a waiting queue is here
    return my_sem; //even when my_sem is NULL, return NULL as in the requirement
}

int sem_destroy(sem_t sem)
{
    queue_destroy(sem->waiting_q);
    free(sem);
    return 0;
}

int sem_down(sem_t sem)
{
    if(sem == NULL)
    {
        return -1;
    }
    while(sem->count == 0) //Taking an unavailable semaphore will cause the caller thread to be blocked until the semaphore becomes available.
    {
	queue_enqueue(sem->waiting_q, uthread_current()); //enqueue the current blocked queue
        uthread_block(); //blocked queue enqueued in waiting_q, and yield to the next one.
        return 0;
    }
    sem->count--;
    return 0;
}


int sem_up(sem_t sem)
{
    if(sem == NULL)
    {
        return -1;
    }
    struct uthread_tcb *first_thread;
    if(queue_dequeue(sem->waiting_q, (void**)&first_thread) == 0) //if waiting_q is not empty
    {
        uthread_unblock(first_thread); //causes the first thread (i.e. the oldest) in the waiting list to be unblocked.
        return 0;
    }
    sem->count++;//if queue isn't empty, release a resource
    return 0;
}
