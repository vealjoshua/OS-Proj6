#ifndef OSS_H
#define OSS_H
#include "share.h"

#define TotalMem 256 // 0 - 255 = 256

/* Global Variables for Oss.c */
int z = 20; //The Seconds time limit on the children running 
int x = 8; //Number of slaves
int v = 1; //this is set to zero in the begning 
int userArr[18]; //array to hold all the pid's 
FILE *filePtr; //file ptr to write to file global so I don't have to pass it

PCB *pcb; //Process control Block
int pcbID = 0;
key_t PCBkey = 1994;

pgTable table[TotalMem];


/*declaring for system clock */
systemClock *OssClock; //To handel time
int clockID = 0; // Id for 
key_t Clockkey = 1991;

int qSize = 0;
REQ msg; //msg to pass back and forth between users and OSS
int msgID = 0; //Message passing ID
key_t MSGkey = 1992;



/*Static Variables */
static int count = 0;
static int waitList = 0; //keeps track of who's in the waitList
static int processMade = 0; //counts up when ever a new process is made
static int processFinished = 0; //Counting How many process I finished 
int printLineCount = 0; //keeps track of how many lines in the .out file limit 2000

/*Function Prototypes for Oss.c */


/*Command Options */
void comOptions (int argc, char **argv , int *x, int *z, int *v, char **filename); //Command options ofr -t and -s , -l
void displayHelpMesg(); //displays the help message
void validate(int *x,int temp,char y); //check for mistakes x(number of process) and z(alarm time vlaues)
void test(int x, int z,char*file);  // this is just to print out put

void shareCreat(); //when called created a shared memory all the structs that need to be shared
void createResource(); //Creats the resources for the users to request 
void AddTime(int addNano); //Adds time for the Clock
/* Argument Handlers for alarm and crtl C*/
void INThandler(int sig);  // handeling Ctrl^C signal
void on_alarm(int signal);  // handeling the alarm(z) signal
void releaseMem(); //one function to delet the shared memory and message queue's

/* Making and Forking the Process */
void makeNewProcess(char * pcbArrLocationPass,char *pcbIDpass,char *clockIDpass, char *msgIDpass);
int findValidPCB(); //find's if there is a valid pcb
void setUpPCB(int i, pid_t childPid); //initlizes a PCB

/* Frame table functions */
int checkInFrame(REQ *msg);
void removeAllPages(int i, pid_t pid); //makes all the pages free is the user dies
int findFrame(); //find a frame that is empty 0 or recramable and return the frame number
void AddToTable(REQ msg);
int checkFree(); //checks the amounts of free pages left
void daemon1();//this find the oldest and makes them reclaimable of if already reclaimable then free
int findOldestPage(int size, int arr[]);//find the oldest page in the frame ignoring previously found and returning the frame location
void removePage(int i, pid_t pid); 

/* Queue Functions */
void addToQueue(queue **head, REQ *msg); //adds to the queu
void printQueue(queue **head);  //prints the queue
void remNode(queue **head); //removes the head

/*printing to file */
void printToFile(int choice, int index);

int getRandFork();
int random_number(int min_num, int max_num);

#endif