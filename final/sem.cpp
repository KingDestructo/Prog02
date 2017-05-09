/*
     (Latest Revision: October 23, 2011)
 
*/
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include "sem.h"

using namespace std ;

/*
 These directives make the two variables "checking" and
 "stdoutLock" visible to the functions in this file.  The
 variables "checking" and "stdoutLock" are supposed to be
 declared in the main program file.
 */

extern int checking ;
extern pthread_mutex_t stdoutLock ;

/* ################################################## */
/*                    create_sim_sem                  */
/* ################################################## */

/* Function to allocate a semaphore and
 initialize it. */

sim_semaphore create_sim_sem(int  init_val)
{
    sim_semaphore ptr ;
    ptr = new sim_sem_data ;
    if   (ptr == NULL)
                cout << "ERROR: Out of space for semaphore." << endl ;
    else
    {    if ( 0!=pthread_mutex_init(&(ptr->lock), NULL) )
        {  cout << "MUTEX INITIALIZATION FAILURE!" << endl;
                exit(-1) ;}
                
                if ( 0!=pthread_cond_init(&(ptr->cond), NULL) )
                {  cout << "CONDITION INITIALIZATION FAILURE!" << endl ;
            exit(-1) ;}
                
                (ptr->value)     =  init_val ;
                (ptr->Qfront)    =  NULL ;
                (ptr->Qrear)     =  NULL ;
    }
        
    return(ptr) ;
}

/* ################################################## */
/*                    empty_sem_Q                     */
/* ################################################## */

/* Test to check if the semaphore queue is
 empty. */

int empty_sem_Q ( sim_sem_data sem_d)
{
        return (sem_d.Qfront == NULL) ;
}

/* ################################################## */
/*                    print_sem_Q                     */
/* ################################################## */


/* For printing the id_num's of the processes
 waiting in the semaphore queue -- for debugging
 & checking.
 
 -- WARNING! Don't add code in this procedure to
 lock the stdoutLock.  Callers of print_sem_Q
 will lock printing before making the call.
 Thus, it is an error for this procedure to
 try to gain the lock.
 */

/* Software maintenance note: 2011/10/23 - had to make a
 change so that id_num becomes a pointer to int type, due to
 compiler now not allowing return type of pthread_self to be
 cast to an int.  Below we cast temp->id_num to long for
 printing, so that it doesn't come out as an octal value.  I
 hope the long data type has enough bits so the numbers are
 unique. */

void print_sem_Q (sim_sem_data sem_d)
{
        sim_PCB  *temp ;
        temp = sem_d.Qfront ;
        cout << "Printing Q .... " << endl ;
        if (temp == NULL) cout << "Q is empty." << endl ;
        while (temp != NULL)
        {
                cout << "Thread #" << (long) temp->id_num << endl ;
                temp = temp->next ;
        }
}

/* ################################################## */
/*                      enq_sem_Q                     */
/* ################################################## */

/* For placing a process on the semaphore
 queue. */

void enq_sem_Q (sim_semaphore sem, sim_PCB * proc)
{    /* Place the unique id of the thread into
          a field of the sim_PCB. */
        
        /* Software maintenance note: 2011/10/23 This was a cast to
         int in previous versions of the code, but that stopped
         working with version 10.6 of the Mac OS.  So I changed
         the data type of id_num to int*, which seems to give a
         workable alternative. */
        
        (proc->id_num)   = (int*) pthread_self() ;
        (proc->next)     = NULL ;
        (proc->in_Q)     = 1 ;
        if   (empty_sem_Q(*sem)) sem->Qfront = proc ;
        else sem->Qrear->next = proc ;
        sem->Qrear = proc ;
}

/* ################################################## */
/*                    serve_sem_Q                     */
/* ################################################## */

/* For removing a process from the
 semaphore queue. */

void serve_sem_Q(sim_semaphore sem)
{
        sim_PCB  *temp ;
        
        if (checking)
        { pthread_mutex_lock (&stdoutLock) ;
        
                /* Software maintenance note: 2011/10/23.  I was casting the
                 'sem' to 'int' before, but that stopped working with
                 version 10.6 of the Mac OS.  So I cast to 'int*' and then
                 cast to 'long' for printing.  The compiler allows this
                 and the numbers come out in decimal.  Without the cast to
                 'long' I was getting octal output. */
                
                
                cout << "State of Q in semaphore #"<< (long) (int*) sem
                << " prior to Q serve: " << endl ;
                print_sem_Q (*sem) ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        temp              =    sem->Qfront ;
        sem->Qfront        =   sem->Qfront->next ;
        
        /* Provide a means by which an enqueued
         thread can tell if it has been dequeued. */
        
        (temp->in_Q)   =   0 ;
        if (sem->Qfront == NULL)  sem->Qrear = NULL ;
        
        if (checking)
        { pthread_mutex_lock(&stdoutLock) ;
                cout << "State of Q in semaphore #"<< (long) (int*) sem
                << " after Q serve: " << endl ;
                print_sem_Q (*sem) ;
                pthread_mutex_unlock(&stdoutLock) ; }
}

/* ################################################## */
/*                    wait_sem                        */
/* ################################################## */

/* The wait operation on the simulated
 semaphore. */

void wait_sem (sim_semaphore sem )
{
        pthread_mutex_lock( &(sem->lock) ) ;
        sem->value = sem->value - 1 ;
        
        if (checking)
        { pthread_mutex_lock(&stdoutLock) ;
                cout << "Thread #" << (long) pthread_self()
                << " begins a wait and"
                << " decrements the value of semaphore #"
                << (long) (int*) sem << " to: " << sem->value << endl ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        if (sem->value < 0)
        /* START (sem->value < 0) */
        {
                sim_PCB * proc = new sim_PCB ;
                if    (proc == NULL)
                        cout << "ERROR: Out of space for sim_PCB." << endl ;
                else
                /* START (proc != NULL) */
                {
                        if (checking)
                        { pthread_mutex_lock(&stdoutLock) ;
                                cout << "Thread #" << (long) pthread_self()
                                << " queues up on semaphore #" << (long) (int*) sem
                                << "." << endl ;
                                pthread_mutex_unlock(&stdoutLock) ;  }
                        
                        /* Put a sim_PCB for this thread on the
                         queue.*/
                        
                        enq_sem_Q(sem, proc) ;
                        
                        /* The thread "dozes" until it awakens
                         and finds that it has been taken off
                         the queue. */
                        
                        do
                        {
                                if (checking)
                                {  pthread_mutex_lock(&stdoutLock) ;
                                        cout << "Thread #" << (long) pthread_self()
                                        << " now BLOCKS on semaphore #" << (long) (int*) sem
                                        << " by doing a pthread_cond_wait." << endl ;
                                        pthread_mutex_unlock(&stdoutLock) ; }
                                
                                if ( 0!= pthread_cond_wait (&(sem->cond),&(sem->lock) ) )
                                {  cout << "COND BROADCAST FAILURE!" << endl ;
                                        exit(-1) ;}
                                
                                if (checking)
                                {  pthread_mutex_lock(&stdoutLock) ;
                                        cout << "Thread #" << (long) pthread_self()
                                        << " on semaphore #" << (long) (int*) sem
                                        << " wakes up, and checks to" << endl
                                        << "\t" << "see if a signal_sem OP has "
                                        << "taken it off the queue." << endl
                                        << "\t" << "It IS" ;
                                        if (!proc->in_Q)  cout << " NOT" ;
                                        cout << " in the queue." << endl ;
                                        pthread_mutex_unlock(&stdoutLock) ; }
                        } /* END OF DO-PART */
                        while (proc->in_Q) ;
                        delete proc ;
                }  /* END  else (proc != NULL) */
        } /* END   if (sem->value < 0) */
        
        if (checking)
        {  pthread_mutex_lock(&stdoutLock) ;
        cout << "Thread #" << (long) pthread_self()
        << " completes a wait on semaphore #"
        << (long) (int*) sem << "." << endl ;
        pthread_mutex_unlock(&stdoutLock) ; }
        
        pthread_mutex_unlock( &(sem->lock) ) ;
}


/* ################################################## */
/*                    signal_sem                      */
/* ################################################## */

/* The signal operation on the simulated
 semaphore. */

void signal_sem (sim_semaphore sem)
{
        pthread_mutex_lock(&(sem->lock)) ;
        sem->value = sem->value + 1 ;
        
        if (checking)
        { pthread_mutex_lock(&stdoutLock) ;
                cout << "Thread #" << (long) pthread_self()
                << " begins a signal and"
                << " increments the value of semaphore #"
                << (long) (int*) sem << " to: " << sem->value << endl ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        if (sem->value <= 0)
        {
                if (checking)
                {  pthread_mutex_lock(&stdoutLock) ;
                        cout << "Thread #" << (long) pthread_self()
                        << " removes a PCB from the queue of"
                        << " semaphore #" << (long) (int*) sem << "." << endl ;
                        pthread_mutex_unlock(&stdoutLock) ; }
                
                serve_sem_Q(sem) ;  /* Remove next sim_PCB from queue. */
                
                /* Tell each dozing thread to check
                 to see if it is the one removed. */
                
                if (checking)
                {  pthread_mutex_lock(&stdoutLock) ;
                        cout << "Thread #" << (long) pthread_self()
                        << " broadcasts to the thread(s) waiting on semaphore #"
                        << (long) (int*) sem << "." << endl ;
                        pthread_mutex_unlock(&stdoutLock) ;  }
                
                if ( 0!=pthread_cond_broadcast(&(sem->cond)) )
                {  cout << "COND BROADCAST FAILURE!" << endl ;
                        exit(-1) ;}
        }
        
        if (checking)
        {  pthread_mutex_lock(&stdoutLock) ;
                cout << "Thread #" << (long) pthread_self()
                << " completes a signal on semaphore #"
                << (long) (int*) sem << "." << endl ;
                pthread_mutex_unlock(&stdoutLock) ; }
        
        
        pthread_mutex_unlock( &(sem->lock) ) ;
}
