#include "ec440threads.h"

enum thread_status
{
    TS_EXITED,
    TS_RUNNING,
    TS_READY
};

struct thread_control_block {
    //id
    pthread_t tid;
    //jmp_buf
    jmp_buf position;
    //status
    enum thread_status status;
    //pointer to the stack
    void* stack;
    //pointer to a return val
    void* ret;
};

/* global vars */
//current running thread
pthread_t TID = 0;
//universal signal handler
struct sigaction sa = { NULL };
//Table of TCB
struct thread_control_block* TCB_table[MAX_THREADS] = { NULL };

void schedule(int signal)
{
    if(TCB_table[TID]->status == TS_RUNNING) {
        TCB_table[TID]->status = TS_READY;
    }
    
    //save the state of current running thread
    int succ = 1;
    if(TCB_table[TID]->status != TS_EXITED) {
        succ = setjmp(TCB_table[TID]->position);
    }

    if(TCB_table[TID]->status == TS_EXITED) { succ = 0; }
    
    int cnt = 0;
    if(!succ) {
        int next = TID + 1;
        while(1) {
            if(next == MAX_THREADS - 1) {
                next = 0;
            }
            if(TCB_table[next] != NULL) {
                if(TCB_table[next]->status == TS_EXITED) {
                    next++;
                }   
                if( TCB_table[next] != NULL && TCB_table[next]->status == TS_READY) {
                    break;
                }
                cnt++;
                if(cnt == MAX_THREADS) {
                    exit(0);
                }
            }
            if(TCB_table[next] == NULL) {
                next++;
            }

        }


        TID = next;
        TCB_table[TID]->status = TS_RUNNING;
        longjmp(TCB_table[TID]->position, 1);
    }
}

// to supress compiler error saying these static functions may not be used...
void schedule(int signal) __attribute__((unused));


void scheduler_init()
{
  
    struct thread_control_block* main = malloc( sizeof(struct thread_control_block) );
    TCB_table[0] = main;
    TCB_table[0]->tid = 0;
    main->status = TS_RUNNING;
    TID = 0;
    
    struct itimerval ite;
    ite.it_interval.tv_sec = 0;
    ite.it_interval.tv_usec = QUANTUM;
    ite.it_value = ite.it_interval;
    
    setitimer(ITIMER_REAL, &ite, NULL);
    sa.sa_handler = &schedule;
    sa.sa_flags = SA_NODEFER;
    sigaction(SIGALRM, &sa, NULL);

    assert(TCB_table[0] != NULL);
    assert(TID == 0);
    assert(TCB_table[TID]->status == TS_RUNNING);
}

int pthread_create(
    pthread_t *thread, const pthread_attr_t *attr,
	void *(*start_routine) (void *), void *arg)
{
  // Create the timer and handler for the scheduler. Create thread 0.
    static bool is_first_call = true;
    if (is_first_call) {
        is_first_call = false;
        scheduler_init();
    }
    
    int ii = 1;
    for(ii = 1; TCB_table[ii] != NULL; ii++) {
        if(ii == MAX_THREADS - 1) {
            printf("max treads reached!");
            return -1;
        }
    }

    *thread = ii;
    struct thread_control_block* newthread = malloc(sizeof(struct thread_control_block));
    newthread->stack = malloc(THREAD_STACK_SIZE);
    TCB_table[ii] = newthread;
    
    set_reg(&(newthread->position), JBL_PC, (unsigned long int)start_thunk);
    set_reg(&(newthread->position), JBL_R13, (unsigned long int)arg);
    set_reg(&(newthread->position), JBL_R12, (unsigned long int)start_routine);
    
    //move the prethread_exit to the bottom of the stack
    void *bottom_of_stack = newthread->stack + THREAD_STACK_SIZE;
    void *stack_pointer = bottom_of_stack - sizeof(&start_routine);
    void (*func)(void*) = (void*) &pthread_exit;
    
    //move the new stackpointer to the stack
    memcpy(stack_pointer, &func, sizeof(func));
    set_reg(&(newthread->position), JBL_RSP, (unsigned long int)stack_pointer);
    
    newthread->tid = ii;
    newthread->status = TS_READY;

    return 0;
}

void pthread_exit(void *value_ptr)
{
    if(value_ptr) {
        TCB_table[TID]->ret = value_ptr;
    }
    TCB_table[TID]->status = TS_EXITED;
    assert(TCB_table[TID]->status == TS_EXITED);
    pause();
    while(1);
}

pthread_t pthread_self(void)
{
    return TID;
}

int pthread_join(pthread_t thread, void **retval)
{
    while(TCB_table[(int)thread]->status != TS_EXITED) {
        pause();
    } 
    void* addr = TCB_table[(int)thread]->ret;
    free(TCB_table[(int)thread]->stack);
    free(TCB_table[(int)thread]);
    TCB_table[(int)thread] = NULL;
    if(retval) {
        *retval = addr;
        pthread_exit(NULL);
    }
    /* will never be reached */
    return 1;
}
