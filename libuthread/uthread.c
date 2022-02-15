#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h" //queue added

queue_t current_q; //running thread
struct uthread_tcb *current_thread;


int initial_id = 0;

enum States{
	active,
	ready,
	blocked,
	zombie,
};

struct uthread_tcb {

	uthread_ctx_t *contxt; //thread context(set of registers)
	void *top_of_stack; //points to the top of the stack
	enum States state; //current state
	int id;

};

struct uthread_tcb *uthread_current(void) //points to current thread
{
	return current_thread;
}


void uthread_yield(void)
{
    /*
     enqueue current thread if its not blocked, not zombie
     */
    if(current_thread->state != blocked && current_thread->state != zombie)
    {
        queue_enqueue(current_q, current_thread); //enqueue current thread to the last position
        current_thread->state = ready;
    }
    
    //allocate new active thread
    struct uthread_tcb *active_thread;
    struct uthread_tcb *old_thread;
    old_thread = current_thread;
    
    preempt_disable();
    //dequeue into active thread
        if(queue_dequeue(current_q, (void**)&active_thread) == -1)
    {
        return; //there's nothing in queue to dequeue, finished running
    }
    preempt_enable();
    active_thread->state = active; //change the new thread state to active
    
    /*
     * i
     set current_thread if the active thread isn't a blocked thread
     otherwise current_thread remain pointing to the blocked thread
     */
    // if(current_thread->state != blocked)//
    //{
    current_thread = active_thread;
    //}
    preempt_disable();
    //context switch from old thread to active thread
    uthread_ctx_switch(old_thread->contxt, active_thread->contxt); //save current, switch to new thread
    preempt_enable();
}
void uthread_exit(void)
{
    /*
     set a temperary current thread to current thread
     */
	struct uthread_tcb *temp_current = current_thread;
   	current_thread->state=zombie;
    uthread_yield();//switch from current to next
    
    /*
     release current thread using temp,
     keep current thread for further switch
     */
	free(temp_current->contxt);
	uthread_ctx_destroy_stack(temp_current->top_of_stack);
    queue_dequeue(current_q, (void**)&temp_current); //remove zombie thread from queue
}

int uthread_create(uthread_func_t func, void *arg)
{
    
        /*
         allocate new thread
         */
        struct uthread_tcb *tcb = (struct uthread_tcb*)malloc(sizeof(struct uthread_tcb)); //a new tcb
        tcb->contxt = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
        if(tcb == NULL || tcb->contxt == NULL)
        {
                return -1;
        }
        tcb->top_of_stack = uthread_ctx_alloc_stack(); //assign a stack for tcb
        tcb->state = ready; //enqueued thread ready
        initial_id++;
        tcb->id = initial_id; //start from 1 (1st thread)

        /*
         assign current_thread points to the created thread
         */
        int init_failed =  uthread_ctx_init(tcb->contxt, tcb->top_of_stack,
                             func, arg);
        if(init_failed== -1)
        { //if successful, return 0, otherwise an error code for the error will be returned
                return -1;
        }
        
        /*
         enqueue created thread
         */
        queue_enqueue(current_q, tcb);
        return 0;
}

int uthread_start(uthread_func_t func, void *arg) //the first one
{
        /*
         allocate main thread
         build queue
         */
        preempt_start();
        preempt_disable();
        struct uthread_tcb *temp = (struct uthread_tcb*)malloc(sizeof(struct uthread_tcb));
        temp->contxt = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
        temp->top_of_stack = uthread_ctx_alloc_stack(); //assign a stack for tcb
        current_q = queue_create();//build queue
        preempt_enable();
        /*
         create threads && check if anything failed
         */
        if(current_q == NULL || uthread_create(func, arg)||temp == NULL || temp->contxt == NULL)
        {
                return -1;
        }
    
        /*
         Switch from main thread to init thread
         */
        queue_dequeue(current_q, (void**)&current_thread);
        current_thread->state = active;
        uthread_ctx_switch(temp->contxt, current_thread->contxt);
        
        /*
         check if there's thread left in queue
         */
        while(queue_length(current_q) != 0)
        {
            printf("entered uthread_yield()");
                uthread_yield();
        }
    
        /*
         successful
         */
        return 0;
}

void uthread_block(void)
{
    //change current thread's state to block and yield to the next thread
	current_thread->state = blocked;
	uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
    //unblock the blocked thread and enqueue to the queue
	if(uthread->state == blocked)
	{
		uthread->state = ready;
	}
    preempt_disable();
	queue_enqueue(current_q, uthread); //insert activated thread to the front
    preempt_enable();
}
