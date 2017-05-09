//Schuyler Davis
//Timothy Gibson
//CS3750
//Prog02
/* In this program the server puts dishes full of food on one end of a table and diners pass the dishes down to the other end of the table where they are collected by the busser.

The job of each diner is to perform a loop:
become ready for the next dish,
acquire the next dish on the trivet to the left,
if the dish contains food eat some of the food from it,
when available, place the dish on the trivet to the right
if the dish contains the check then stop else go to step 1.
The job of the server is to serve dishes by placing them one after another onto trivet 0 and then exit.

The job of the busser is to remove dishes from the last trivet one after another until removing the check.

The threads implementing the server, busser, and diners have to coordinate. Their goals are to
avoid as much busy waiting as possible
achieve as much parallel processing as possible
avoid the possibility that a person could acquire the same dish twice
avoid the possiblity that a person could "serve from an empty dish"
avoid the possibility that a person could place one dish on top of another.
make sure everyone gets finished eating and the check gets paid.
make sure that only two threads are involved in regulating access to each trivet: the thread that puts dishes onto the trivet and the thread that takes them off.
Your grade depends on how well you achieve these goals. */
#include <iostream>
#include <sched.h>
#include <time.h>
#include <pthread.h>
#include <string>
#include "sem.h"

using namespace std ;

/* ######################################## */
/*             Global Variables             */
/* ######################################## */
//const int numTrivets = 6 ;
const int numTrivets = 3 ;
const int numDiners = numTrivets - 1 ;
const int maxDishNames = 13 ;
//const int numDishNames = 13 ;
const int numDishNames = 5 ;

int trivet[numTrivets] ;

string dishName[maxDishNames];

      /* Here declare the array(s) of semaphores you will
	 need to synchronize threads, plus whatever other
         variables you want. */

         sim_semaphore emptyTrivets[numTrivets];
         sim_semaphore fullTrivets[numTrivets];

         sim_semaphore server;

         sim_semaphore finished;


      /* child_t are global variables to represent the
	 dynamically-created threads. */

pthread_t child_t[numTrivets] ;

/* ######################################## */
/*      "Special"   Global Variables        */
/* ######################################## */

/* Code in sem.cpp "expects" the two variables below to be here.
   This particular program does not use "checking." */

         /* "Checking" is just a flag that you set to 1, if you want lots of
	    debugging messages, and set to 0 otherwise.  The semaphore code in
	    sem.cpp imports "checking".  Therefore the semaphore operations
	    will write lots of messages if you set checking=1.  */

int checking ;

      /* In some programs, we use the "stdoutLock" variable declared below to
	 get intelligible printouts from multiple concurrent threads that write
	 to the standard output.  (There has to be something to prevent the
	 output of the threads from interleaving unintelligibly on the standard
	 output, and we can't use semaphores if the semaphore code is writing
	 messages too.)

         To print a message to standard output, a thread first locks standard
	 output, then writes, then unlocks standard output.  See files sem.cpp
	 or conc.cpp for examples of code that write messages in this manner.

         WARNING:  DON'T change how the locking of standard output is done
	 until you've thought a WHOLE lot about the consequences.  In
	 particular, using semaphores to do the job of stdoutLock can cause
	 "infinite recursion" under certain circumstances.  The reason is that
	 the semaphore code itself imports "stdoutLock" and writes messages
	 when the "checking" variable is set to 1. */

pthread_mutex_t stdoutLock ;

struct threadIdType
{
   int id ;
};

/* ################################################## */
/*                         init                       */
/* ################################################## */
void init()
{
  int index ;

  srandom(time((time_t *) 0)); /* INITIALIZE RANDOM NUMBER GENERATOR */

  checking = 0 ;

       /* Initialize the "special lock" that is used only to get
	  exclusive access to the screen. */

  if ( 0!=pthread_mutex_init(&stdoutLock, NULL) )
  {  cout << "MUTEX INITIALIZATION FAILURE!" << endl;
     exit(-1) ;}

    /* Initialize the trivets to indicate that each contains "no
       dish." */
for (index=0; index<numTrivets; index++) trivet[index]=0;

    /* Here initialize the array(s) of semaphores, and
       whatever other variables you use.  */

//initialize emptyTrivets to 1
//initialize fullTrivets to 0

  for (int i = 0; i < numTrivets; i++ )
  {
    fullTrivets[i] = create_sim_sem(0);
    emptyTrivets[i] = create_sim_sem(1);
  }

  //set the finished sem to 0
  finished = create_sim_sem(0);

  //set the server semaphore to 0
  server = create_sim_sem(0);

 /* Give some mnemonic names to the dishes.  The first name is
    used for an empty trivet.  The last name denotes the check
    (bill) for the meal.  This is coded so no changes are needed
    here as long as the value of "numDishNames" is between 2 and
    13. */

  dishName[0]="no dish";
  dishName[1]="vegetable soup" ;
  dishName[2]="bread and butter" ;
  dishName[3]="beets and chickpeas" ;
  dishName[4]="hardboiled eggs" ;
  dishName[5]="calf tongue" ;
  dishName[6]="baked potato" ;
  dishName[7]="string beans" ;
  dishName[8]="rack of lamb" ;
  dishName[9]="salad" ;
  dishName[10]="coffee" ;
  dishName[11]="flan" ;
  dishName[numDishNames-1]="check" ;

}

/* ################################################## */
/*                    DelayAsMuchAs                   */
/* ################################################## */
void delayAsMuchAs (int limit)
{
  int time, step;
  time=(int)random()%limit;
  for (step=0;step<time;step++) sched_yield() ;
}

/* ################################################## */
/*                       Server                       */
/* ################################################## */
/*

     The mother thread spawns a child thread that executes this
     function.  This function carries out the job of the server
     at the restaurant.

*/
void * Server(void * ignore)
{

  int i, j, delayLimit=100 ;

  for (i=1; i<numDishNames; i++)
  {

        /* I delay a random time before I "feel like" placing
	   another dish on the table.*/

    delayAsMuchAs(delayLimit);

      /* When the trivet is available, I place the dish on the
         trivet to my right. */

       /* Here do a synchronization task.  One thing you need to
	  do is be sure that you are not going to place a dish on
	  a trivet that already has a dish on it.  *DO NOT* just
	  busy-wait until you see that the trivet is empty. */

    wait_sem(emptyTrivets[0]);

    trivet[0]=i; // put dish #i onto trivet #0.
    pthread_mutex_lock(&stdoutLock) ;
    cout << "Server places " << dishName[trivet[0]]
         << " on trivet #0." << endl ;
    pthread_mutex_unlock(&stdoutLock);

       /* Here you may want to a synchronization task --
	  something that "opens the door" for diner #0 to get
	  access to the new dish. */

    signal_sem(fullTrivets[0]);


  }
  pthread_exit ((void *)0) ;
}

/* ################################################## */
/*                         Diner                      */
/* ################################################## */
/*

     The mother thread spawns child threads that execute this
     function.  This function carries out the job of one of the
     diners at the restaurant.

*/

void * Diner(void * postnPtr)
{

       /* Type cast the parameter to recover "position" -- which
	  tells me the position at which I am seated at the
	  table. */
  int position = ((threadIdType *) (postnPtr))->id ;
  int i, j, delayLimit=100 ;

  for (i=1; i<numDishNames; i++)
  {
        /* I delay a random time before I "feel like" picking up the next
	   dish.*/

    delayAsMuchAs(delayLimit);

      /* When available, I pick up the next new dish on my left. */

       /* Here do a synchronization task.  One thing you need to
	  do is be sure that there is a new dish on the trivet to
	  your left now, and that the person on your left has
	  "let go" of it. */

wait_sem(fullTrivets[position]);


      /* I declare what I am doing */
    pthread_mutex_lock(&stdoutLock) ;
    cout << "Diner number "<< position ;
    if (i<numDishNames-1) cout << " enjoys ";
    else if (position<numDiners-1) cout << " examines " ;
         else cout << " examines and pays " ;

    cout << dishName[trivet[position]] << endl ;
    pthread_mutex_unlock(&stdoutLock);

	/* I delay a random time to simulate the time it takes for me to
	   serve myself some of what is on the dish -- or look at the
	   check. */

    delayAsMuchAs(delayLimit);

        /* When available, I place the dish on the trivet to my right. */

       /* Here do a synchronization task.  One thing you need to
	  do is be sure that the trivet on your right does not
	  have a dish on it now.*/


signal_sem(fullTrivets[position+1]);

    pthread_mutex_lock(&stdoutLock) ;
    cout << "Diner number "<< position << " moves "
         << dishName[trivet[position]] << " from trivet #"
         << position << " to trivet #" << position+1 << endl;
    pthread_mutex_unlock(&stdoutLock);
       /* transfer the dish on my left to trivet on my right */
    trivet[position+1]=trivet[position] ;
      /* mark trivet on my left as empty */
    trivet[position]=0;

       /* Here do a synchronization task. You have transferred a
	  dish from your left to your right.  The person on your
	  left will need to find out that the trivet on your left
	  is now empty.  The person on your right will need to
	  find out that the trivet on your right now has a new
	  dish on it.  */



  }
  pthread_exit ((void *)0) ;
}

/* ################################################## */
/*                       Busser                       */
/* ################################################## */
/*

     The mother thread spawns children and then executes this
     function.  This is convenient because this function should
     be the last to exit.  This function carries out the job of
     the busser at the restaurant.

*/
void * Busser (void * ignore)
{

  int i, j, delayLimit=100 ;

  for (i=1; i<numDishNames; i++)
  {

        /* I delay a random time before I "feel like" bussing another
	   dish.*/

    delayAsMuchAs(delayLimit);

      /* When another dish is on the trivet to my right I remove it. */

       /* Here do a synchronization task.  One thing you need to
	  do is be sure that there is a new dish on the trivet to
	  your left now, and that the person on your left has
	  "let go" of it. */



    pthread_mutex_lock(&stdoutLock) ;
    cout << "Busser removes "
         << dishName[trivet[numTrivets-1]] << " from trivet #"
	 << numTrivets-1<< "." << endl ;
    pthread_mutex_unlock(&stdoutLock);
    trivet[numTrivets-1]=0; // remove the dish.

       /* Here do a synchronization task. The person on your left
	  will need to find out that the trivet on your left is
	  now empty.  */



  }
  return (void *) 0 ;
}

/* ################################################## */
/*                         Main                       */
/* ################################################## */
int main()
{
  init();

  cout << endl << endl;
  cout << "Welcome to the restaurant!" << endl ;
  cout << numDiners << " will be dining." << endl ;
  cout << "The meal will consist of " << numDishNames-2
       << " dishes." << endl;
  cout << "Bon appetit!" << endl ;
  cout << endl << endl;

  int i;

       /* This is a pointer to a struct (class) that contains an int
          field - it is a convenient data type to use as the parameter
          to the child function.  */
   threadIdType * idPtr ;

  for (i=0; i<numDiners; i++)
  {

      idPtr = new threadIdType ; /* allocate memory for struct */
      idPtr->id = i ;  /* records current index as the child's ID */

     if (0!=pthread_create(&child_t[i], NULL, Diner, (void *) idPtr))
        {cout << "THREAD CREATION FAILURE!" << endl; exit(-1) ;}

     if (0!=pthread_detach(child_t[i]))
        {cout << "THREAD DETACHMENT FAILURE!" << endl ; exit(-1) ;}
  }

     if (0!=pthread_create(&child_t[numDiners], NULL, Server, (void *) 0))
        {cout << "THREAD CREATION FAILURE!" << endl; exit(-1) ;}

     if (0!=pthread_detach(child_t[numDiners]))
        {cout << "THREAD DETACHMENT FAILURE!" << endl ; exit(-1) ;}

  Busser((void *)0) ;

  cout << endl << endl;
  cout << "Thank you for coming!" << endl ;
  cout << endl << endl;

  return 0 ;

}
