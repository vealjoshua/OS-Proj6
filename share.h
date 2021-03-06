#ifndef SHARE_H
#define SHARE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include <error.h>
#include <assert.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <semaphore.h>


#define  NANOSECOND    1000000000
#define  macro 18

typedef struct Clock
{
	unsigned int nanoSec;
	unsigned int sec;
}systemClock;

typedef struct PageTable
{
	int frame; //frame number
	int p; //page number
	pid_t userPid;
	systemClock accessTime; // the time the frame was accesed and used 
	int dirty; //if the page in the frame was used to write to file
	int state; // 0 if he page in the frame is empty | 1 if the page in the frame was used | 2 if the page in the frame is old and can be reclaimable 
	int pcb;
}pgTable;


typedef struct msgStruct
{
	int type;
	int msgType; // 0 is death 1 is user would like access to a page
	int sender; //the pcbLocation that is sending the message 
	pid_t userPid;
	systemClock accessTime;
	int pgNum;  // pgNum the user would like to access
	int rWchance; //0 is read | 1 is write 
}REQ;

typedef struct ProcessControlBlock
{
	pid_t userPid;
	int valid;
	int critSectionAccess; // 0 is default 1 is waiting 2 is accessing critical section 
	int pgAcess[32];
	int totalMemAccess; //recording all the memory access by the user
	int totalPageFaults; //recurding all the page faults the user got
}PCB;
static unsigned int g_seed;

// Used to seed the generator.           
inline void fast_srand(int seed)
{
    g_seed = seed;
}

typedef struct queue
{
	REQ msg; //stores the msg struct in the queue
	int userNum; //stores the user pcbLocation 
	pid_t userPid;
	struct queue* next;
}queue;

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
inline int fast_rand(void) 
{
    g_seed = ((getpid()*rand()%1000000000000)*g_seed+(time(NULL)*rand()%1000000000000));
    return (g_seed>>16)&0x7FFF;
}

#endif
