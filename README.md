# User-level-Thread-Scheduler-Implementation

Schedule the execution of threads in RR, FIFO, Pre-emptive SJF and Multilevel Queue fashion.
Provide an interrupt-based scheduler and be pre-emptive to prevent thread starvation.
Provide a thread semaphore API to control the access to common resources by multiple threads.


## Phase 1: queue API
The goal of Phase 1 is to build a FIFO queue that could store thread(from Phase 2) and put thread in use if we need.

### Step 0:
Setup two struct named `node` and `queue`. `node` is to use store datas and `next` pointer. `queue` is to locate the queue''s start and end point. Also, `queue` could also help us keep tracking the size of queue.

### Step 1:
Create a queue. `queue_create` is to create an empty queue and return its address. Set both the start and end to NULL for initial and size to 0. 

### Step 2:
Apply actions in queue. \
The goal of `queue_destory` is remove the queue. Since we set our queue by using memory address, we need to deallocate the memory address. We can simply use `free()` function to achieve our goal. 
* Since we have a pointer keep tracking the size of queue, I made a Boolean function call `ifQEmpty` to check if the queue is empty just for making out codes easy to read. 
* `queue_enqueue` is use to put thread in queue. We set up two condition here. If queue is empty, we put the `start` pointer to the new node, and make the size plus 1. If queue is not empty, then we just connect the `end` pointer's `next` pointer to the new node. And the new node's `next` pointer will point to __NULL__. 
* `queue_dequeue` is use to take out the first(the oldest) node in our queue. What we do here is remove and replace the first node by make the second of the queue be the first of the queue. Use __queue->start = queue->start->next__ to set the second node become the first one, and reduce the size of the queue by one.
* `queue_delete` is a bit complicated. First we need to create a new queue `findRep` and make it equal to our old queue(point at the start of the old queue). Then we iterate through the queue. If the node we want to delete is the first one, we just apply `queue_dequeue` on it. If we didn't find it after we go through the whole queue, return -1. If we find the node in the middle of the queue, first we need to make a temperate queue for store our datas. Set this temperate queue point at the next node of `findRep`. Then we do the same thing as we did in dequeue, pass the next node. Then we check if this is the last node of the queue. If it is, `queue`'s end will `findRep`. If nothing happened, we just keep go through the queue. In the end free the `findRep`.
* `queue_iterate` is use to go through and callback each node in the queue. First we check if the `queue` or `func` is NULL. Then we create two queue named `Qcp` and `React`. We make `Qcp` equal to the original queue, and `React` to be a empty queue so we can easy assign value on it. We build a loop. At the beginning of the loop, we use `func` to callback `queue`'s data. Then we put `React` point at next node. Basically we go though all queue and apply `func` on it.\
* `queue_length` is use to keep track of numbers of nodes. We have `sizeOfNode` in our struct, so we just return it.
### Step3:
We first use the `queue_tester.c` to test our code is all pass. Then we use the code in `Project_2.html` under Phase 1 Hints. I changed the start of last two lines of code to `TEST_ASSERT`, and pass these tests.


## Phase 2: uthread API
### The goal:
The goal for this Phase is to build a thread library environment that works for:
####  Step 0 : 
set up the global variables and the queue.
####  Step 1: 
`uthread_create` and `uthread_start` function: building each thread and start the processing.
####  Step 2:
`uthread_yield`: does 2 things.
1. taking charge of the determination of if we should return the current blocked thread to `sem.c`  OR
2. enqueue the current unfinished thread to our `current_q` and dequeue and process the next thread in `current_q`.
#### Step 3:
`uthread_block` and `uthread_unblock`:
1. `uthread_block`: Changes the thread's state and yield to the next thread 
2. `uthread_unblock`: Changes the thread's state and put back into `current_q` to

These are for our semaphore to call.
        
#### Step 4: 
`uthread_exit`: exit the current thread.

### Details about step 0 to step 4
### Step 0:
* `Current_q` is the queue I build for putting in each thread to wait.\
* `Current_thread` is the thread pointer I use to track the current active thread or the thread that needs to be modified. And, `current_thread` pointer will also take the responsibility of keep tracking the thread that needs to be blocked in out semaphore later.\
We have an `enum` to translate the states from English to numeric values: active = 0, ready = 1, blocked = 2 and zombie = 3.\
* The `thread_tcb` structure contains: its context, its stack, its state and its id.

### Step 1:
`thread_create` and `uthread_start` function
1. `uthread_create`: is taking the responsibility to create the thread. I allocated a thread first, then initialize it by using `uthread_ctx_init`, then enqueue it into our `current_q`. return -1 if failed
2. `uthread_start`: thread temp is allocated to generate our main thread without any interaction with the out `current_q` . After that, `uthread_create` function is called indicates that our 1st initial thread is initialized and enqueued into our `current_q`.\
Then I dequeued the initial thread, changing its state to be active and the context now can be switched from main to the initial thread. As long as our `current_q` isn't empty, we will go and run our next thread by calling -> `uthread_yield`

### Step 2:
`uthread_yield`: The first thread in our current_q is the NEXT THREAD(pointed by active_thread) needs to be processed next. But we first need to check if the CURRENT THREAD(pointed by `current_thread`) is a blocked or zombie thread. Aparrently, both the blocked and zombie thread won't be enqueued into the current_q but blocked thread will be enqueued into our `waiting_q()` in `sem.c`. Then we dequeue the NEXT THREAD from the `current_queue`(pointer by active_thread), and switch the context from the current_thread to the active_thread.

### Step 3:
`uthread_block` and `uthread_unblock`:
1. `uthread_block`: Changes the thread's state to be blocked and calling the `uthread_yield` function to do the determination of whether dequeue it into the `current_q`, or letting the `current_thread` pointer points to the blocked thread so the current thread now can be returned by `uthread_current()` function and can be used by sem.c to put the blocked threads into sem.c's `waiting_q`(contains blocked threads).
2. `uthread_unlock`: makes sure the thread is blocked and unblocked it(changing the state to be ready), then enqueue the unblocked ready thread into our current_q and waiting for its turn to be processed.

### Step 4:
`uthread_exit`: \
changes the thread that needs to be exit's state to be zombie, and process the next available thread by calling `uthread_yield` function. Then the context of the zombie thread is freed, the stack of the zombie thread is destroyed by calling the `uthread_ctx_destroy_stack()` function from the professor, and finally the zombie thread is dequeued from the `current_q`



## Phase 3: semaphore API
What the semaphore API does is to track the number of available resources(a concept of like the number of threads that the user wants a computer can handle). So it can manage the threads' to process systematically. The functions are for the user to use in which thread they want to process and how many threads they want to process at the same time.

### Part 0:
The semaphore structure will contain the number resources are there, and a waiting_q contains all the blocked(waiting) threads.

### Part 2:
`sem_create`: \
It creates the semaphore by allocating a place for it, and let the user to determine the number of resources. Then create the `waiting(blocking)_q`.

### Part 3:
`sem_destroy`:\
It destroys the queue and free the semaphore.
### Part 4:
`sem_down`:\
As long as there's a semaphore, determine if there is a resource for the `current thread`(the current thread is pulled from uthread.c by calling `uthread_current()`). If not, enqueue the thread into the waiting queue and calling `uthread_block` to block it.(recall, `uthread_block` will changing its state to be block and yield to the next thread without enqueueing the blocked thread into `current_q` in uthread.c). But if there is an available resource, the resource's count is down by 1(meaning 1 resource is taken).
### Part 5:
`sem_up`:\
As long as there's a semaphore, we first record the first thread in our `waiting_q`(contains all blocked threads) using `first_thread` pointer by dequeueing the thread from `waiting_q`. If the dequeue is successful(there are threads being blocked and waiting to be processed) then we can call `uthread_unblock()` to unblocked it and enqueue it into our `current_q` in uthread.c(setting it to wait for process). But if there's no threads are blocked, we can released a resource and let the coming thread to take it.

## Phase 4: Preemption
The goal of this part is to help us understand how to control signal so we keep tracking the the thread(or sem) and decide when to stop running progress. This part we use many function from GNU menu.

### Step 1:
Set four global variable to keep tracking of the signal. `tmer` is from `itimerval`. `itimerval` is use for setting an alarm. It include `it_interval` and `it_value`. These two function we could use to keep tacking time interrupts and alarm. `Act` we create here is use for `AlarmHandle` because there is a function in `sigaction` could keep tracking an function pointer. And we use to track the `uthread_yield`. Finally, `SigForPr` is use for set a signal set and blocking our signal if we need. `Convert` is just for convert HZ to millisecond. 1 sec == 1000 ms, and we want it 100 times so it will be this number.

### Step 2:
* `preempt_disable` is simply block the signal. So we use build in function  
* `sigprocmask(SIG_BLOCK, &SigForPr, NULL)` to block the signal. The __NULL__ at the end because we just need to block the signal instead of switching to another.
* `preempt_enable` is simply unblock the signal.
* `AlarmHandle` is for wait the signal to come. If the signal comes, it will call `uthread_yield`.
* `preempt_stop` is for "save the previous signal action and restore the previous timer configuration'(from proj2 handout). So we put this function at the start of `preempt_start`. 
* In `preempt_start`, we first reset the `sigaction` structure to zero because this is our global variable. `sigempyset` is a function that use to reset the signal set to 0. Then we use `sigaddset` to add the set in the signal set. After these is done, yield for the new action.\
* `preempt_stop` is for start the preemption. We use `tmer` as our time signal set. We use `it_interval` to set our time slot from beginning to finish, and `it_value`to count the time pass. Then let `setitimer` function to handle the action.
