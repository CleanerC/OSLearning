#include "ec440threads.h"

enum thread_status
{
    TS_EXITED,
    TS_RUNNING,
    TS_READY,
    TS_BLOCKED
};

enum mutex_err
{
    MU_SUCC,
    MU_ALR_LOCK,
    MU_NOT_LOCK
};

enum barrier_err
{
    DIS_ON_WAIT = -1
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
struct sigaction sa = {0};
//Table of TCB
struct thread_control_block* TCB_table[MAX_THREADS] = { NULL };
//timer attr
struct itimerval old_attr;



void schedule(int signal)
{
    // printf("print\n");
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
                if( (TCB_table[next]->status == TS_EXITED) || (TCB_table[next]->status == TS_BLOCKED) ) {
                    next++;
                }   
                if( (TCB_table[next] != NULL) && (TCB_table[next]->status == TS_READY) ) {
                    break;
                }
                cnt++;
                if(cnt ==  MAX_THREADS) {
                    exit(0);
                }
            } else {
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
    schedule(0);
    exit(0);
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


/*******************************************************/
/* syncronization */
/* helper */
void Pause()
{
    struct itimerval zero_timer = {0};
    setitimer(ITIMER_REAL, &zero_timer, &old_attr);
}
void Resume()
{
    setitimer(ITIMER_REAL, &old_attr, NULL);
}
/* these two function just make sure some part of the function executes "atomically" */

void append(Mutex_Control_Unit* MCU, pthread_t tid) {
    waitList_t *new = malloc(sizeof(waitList_t));
    new->tid = tid;
    new->next = NULL;
    if(!MCU->List_head) {
        MCU->List_head = new;
        MCU->List_tail = new;
    } else {
        MCU->List_tail->next = new;
        MCU->List_tail = new;
    }
}

/* mutex */
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr)
{
    if((void*)(mutex->__align) != NULL) { free( (void*)(mutex->__align) ); }
    mutex->__align = (long int)malloc(sizeof(Mutex_Control_Unit));
    ((Mutex_Control_Unit*)(mutex->__align))->locked = false;
    ((Mutex_Control_Unit*)(mutex->__align))->List_head = NULL;
    ((Mutex_Control_Unit*)(mutex->__align))->List_tail = NULL;
    
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if( (((Mutex_Control_Unit*)(mutex->__align))->locked) || ((Mutex_Control_Unit*)(mutex->__align)) ) { return -1; }
    free(((Mutex_Control_Unit*)(mutex->__align)));
    return 0;
}


int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    Pause();
    Mutex_Control_Unit *MCU = (Mutex_Control_Unit*)(mutex->__align);
    if( MCU->locked ) {     //thread tries to grab a locked mutex
        TCB_table[TID]->status = TS_BLOCKED;
        append( MCU, TID );
        Resume();
        schedule(0);
    } else {        //thread grabing a unlocked mutex
        MCU->locked = true;
        Resume();
    }
    
    return 0;
}


int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    Mutex_Control_Unit *MCU = (Mutex_Control_Unit*)(mutex->__align);
    Pause();
    MCU->locked = false;
    if(MCU->List_head) {
        waitList_t *tofree = MCU->List_head;
        TCB_table[tofree->tid]->status = TS_READY;       //FIFO
        MCU->List_head = tofree->next;
        free(tofree);
    }
    Resume();
    return 0;
}


/* barrier */
int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count)
{
    if((Barrier_Control_Unit*)(barrier->__align)) {
        free(((Barrier_Control_Unit*)(barrier->__align))->tids);
        free((void*)(barrier->__align)); 
    }
    barrier->__align = (long int) malloc(sizeof(Barrier_Control_Unit));
    ((Barrier_Control_Unit*)(barrier->__align))->cnt = 0;
    ((Barrier_Control_Unit*)(barrier->__align))->ret = false;
    ((Barrier_Control_Unit*)(barrier->__align))->bar = count;
    ((Barrier_Control_Unit*)(barrier->__align))->tids = calloc(count, sizeof(pthread_t));
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    Barrier_Control_Unit* BCU = ((Barrier_Control_Unit*)(barrier->__align));
    if(BCU->cnt != 0) { return DIS_ON_WAIT; }
    free(BCU->tids);
    free(BCU);
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{

    Barrier_Control_Unit* BCU = ((Barrier_Control_Unit*)(barrier->__align));
    Pause();
    if(BCU->cnt == 0) { BCU->ret = false; }
    (BCU->tids)[BCU->cnt] = TID;
    BCU->cnt++;
    TCB_table[TID]->status = TS_BLOCKED;
    if (BCU->cnt < BCU->bar) {
        Resume();
        schedule(0);
    }
    //cnt is reached
    if(!BCU->ret) {
        for(int ii = 0; ii < (BCU->bar); ii++) {
            TCB_table[(BCU->tids)[ii]]->status = TS_READY;
        }
        free(BCU->tids);
        BCU->tids = calloc(BCU->bar, sizeof(pthread_t));
        BCU->cnt = 0; 
        BCU->ret = true;
        Resume();
        schedule(0);
        return -1;          //PTHREAD_BARRIER_SERIAL_THREAD
    }
    Resume();
    schedule(0);
    return 0;
}