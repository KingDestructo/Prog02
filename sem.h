/*
 (Latest Revision: October 23, 2011)
 
 */

/*
 Files sem.h and sem.cpp implement a kind of counting
 semaphore that duplicates exactly the semantics of the
 counting semaphores of the Silberschatz-Galvin operating
 systems text book.  Also, signal operations on these
 semaphores free blocked threads in FIFO order.
 */

/* Simulated Process Control Block to represent
 processes waiting on a semaphore. */

typedef int* id_num_type ;
/* Software maintenance note: 2011/10/23 - had to make a
 change so that id_num becomes a pointer to int type, due to
 compiler now not allowing return type of pthread_self to be
 cast to an int */

typedef struct sim_PCB
{
    id_num_type       id_num ;  /* Unique id of thread. */
    int               in_Q ;     /* mark if in Q */
    struct sim_PCB *next ; /* For linked list of sim_PCB's in
                                                        sim_semaphore. */
} sim_PCB ;

/* Data space for simulated semaphore object. */

typedef struct sim_sem_data
{
        
    /* mutex_t lock ; */ /* Built-in cthread devices */
    /* condition_t cond ; */ /* for synchronization.  */
    pthread_mutex_t   lock ;    /* Built-in pthread devices */
    pthread_cond_t    cond ;    /* for synchronization.     */
    int value ; /* Semaphore "counter" or "value". */
    sim_PCB           *Qfront;
    sim_PCB           *Qrear ;
} sim_sem_data ;

/* Simulated seamphore object */

typedef sim_sem_data *sim_semaphore ;


/* Function that creates and returns a new semaphore.  The
 counter of the semaphore is initialized to the value of the
 parameter init_val. */

sim_semaphore create_sim_sem(int  init_val) ;


/* The wait operation on the simulated semaphore. */

void wait_sem (sim_semaphore sem ) ;


/* The signal operation on the simulated semaphore. */

void signal_sem (sim_semaphore sem) ;
