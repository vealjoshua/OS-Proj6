#include "user.h"

PCB *pcb; //making instance of rcb
systemClock *OssClock;
REQ msg; //message reuqest 
int pcbID = 0;
int clockID = 0;
int msgID = 0;


int main(int argc, char *argv[])
{
	signal(SIGALRM, TimeHandler); // getting is alarm is activated
	signal(SIGQUIT,INThandler); //Handels SIGQUIT
	signal(SIGINT, INThandler);  // Catching crtl^c
	signal(SIGUSR1, sigDie); //Checking if This process needs to die
	signal(SIGUSR2, sigDie2);
	
	int pcbLocation = atoi(argv[0]); // making key for access 
	pcbID = atoi(argv[1]);
	clockID = atoi(argv[2]);
	msgID   = atoi(argv[3]);
	
	AttachToMem();
	
	// printf("User %d | Pid %d | Time  %d.%010d \n", pcbLocation, pcb[pcbLocation].userPid, OssClock->sec,OssClock->nanoSec);
	
	int memRefCounter = 0; //counter for the number of memory refrences
	int deathCheck = random_number(1,100) + 500;
	
	msg.sender = pcbLocation;
	msg.userPid = pcb[pcbLocation].userPid;
	int table = 0;
	int check = 0;
	while(1)
	{
		msg.pgNum = random_number(0,31); // asking for a random number 
		// if (msg.pgNum == check)
			
		
		if (random_number(1,100) < 50) //read chance
			msg.rWchance = 1; // 1 is read chance
		else               //write chance
			msg.rWchance = 2; //2 is write chance 
		 
		// printf("Read Write chance %d\n", msg.rWchance);
		memRefCounter++; //couting up the memRefCounter when the signal is sent to oss using semaphore
		/*sending A message after picking which page and if read or write */
		msg.msgType = 1; //User is requesting to use a page
		msg.type = 1;
		if((msgsnd(msgID, &msg, sizeof(REQ), 0)) == -1)
		{
			perror("ERROR -- user msgsnd req 1");
			releaseMem();
			exit(1);
		}
		else
		{
			pcb[pcbLocation].critSectionAccess = -1;
			while (pcb[pcbLocation].critSectionAccess == -1); //wait for the OSS to decide for where the user should go
			
			if(pcb[pcbLocation].critSectionAccess == 1) // The user is will start to wait on the OSS
			{
				// printf("User Waiting %d \n", pcbLocation);
				while (pcb[pcbLocation].critSectionAccess == 1) // while the user is waiting
				{
					if(pcb[pcbLocation].critSectionAccess == 2) //The user has been given the page from the Oss to go into its critical section
					{
						/*Critical Section -> this is where the user will "use the page" and Add time to the clock and give control back to the OSS*/
						pcb[pcbLocation].totalMemAccess++; 
						AddTime(random_number(50000,150000)); //User addes time to the clock
						pcb[pcbLocation].critSectionAccess = 0 ; //setting the access back to default and now the Oss has access back to his critcal section 
						break;
					}
				}
			}
			else if(pcb[pcbLocation].critSectionAccess == 2)
			{
				AddTime(random_number(50000,150000)); //User addes time to the clock
				pcb[pcbLocation].totalMemAccess++; 
				pcb[pcbLocation].critSectionAccess = 0 ; //setting the access back to default and now the Oss has access back to his critcal section 
			}
			/* Now the user will wait here untill their page has been retrieved and they can go into the critical section */
		}
	
		// After 1000 Memoray References to the frame table the user has a 80 / 20 chance to die 
		if (memRefCounter > deathCheck) //User has dones 1000 memory refrences not it's picking a random chance to die 
		{
			// printf("memRefCounter %d\n", memRefCounter);
			if (random_number(1,100) < 80)
			{
				printf("User Exiting : %d\n", pcbLocation);
				msg.msgType = 2; //User telling the OSS its time to die
				msg.type = 1;
				if((msgsnd(msgID, &msg, sizeof(REQ), 0)) == -1)
				{
					perror("ERROR -- user msgsnd req 1");
					releaseMem();
					exit(1);
				}
				else
				{
					pcb[pcbLocation].critSectionAccess = -1;
					while (pcb[pcbLocation].critSectionAccess == -1) {}; //wait for the OSS to clean everthing before the user extis
					
					if(pcb[pcbLocation].critSectionAccess == 2) //The user can now exit
					{
						releaseMem();
						exit(1);
					}
				}
			}
			else
			{
				printf("User %d : %d Decided not to Exit\n",pcbLocation, pcb[pcbLocation].userPid);
				deathCheck = random_number(900,1100) + memRefCounter;
			}
		}
	}
	
	pcb[pcbLocation].valid = 0;
	releaseMem();
	exit(1);
	return 0;
}
void INThandler(int sig)
{ 
  signal(sig, SIG_IGN); // ignoring any signal passed to the INThandler
  fprintf(stderr, "\nCtrl^C Called, Process Exiting\n");
  releaseMem();
  kill(getpid(), SIGKILL);
}
void TimeHandler(int sig)
{
  releaseMem();
  //shmctl(shmid, IPC_RMID, NULL); //mark shared memory for deletion
  fprintf(stderr, "\nOut of Time, Process %d Exiting\n", getpid());
  kill(getpid(), SIGKILL);
  //exit(0);
}

void sigDie(int sig)
{ 
	// printf("Exiting pid: %d\n", getpid());
	releaseMem();
	exit(0);
}
void sigDie2(int sig)
{
	printf("Yes!\n");
}
void AttachToMem() //this function just attacheds all the id's to their memory location
{
	/* Attaching to the shared memory for OssClock The OSS CLOCK */
	OssClock = (systemClock*)shmat(clockID, NULL, 0);
	if(OssClock->nanoSec == (int)-1) // Now we attach the segment to our data space.
	{
		releaseMem();
		perror("Shmat error in Main OssClock");
		exit(1);
	}
	
	/* Attaching to the shared memory for the PCB*/
	pcb = (PCB*)shmat(pcbID, NULL, 0);
	if((void*)pcb == (void*)-1)
	{
		perror("shmat child pcb");
		releaseMem();
		exit(1);
	}
		
}
int random_number(int min_num, int max_num)
{
	if (max_num > 10000)
	{
		  int randNum = min_num;
		  int result =0,low_num=0,hi_num=0;
		  if(min_num < max_num)
		  {
			low_num=min_num;
			hi_num=max_num+1;
		  }
		  else
		  {
			low_num=max_num+1;
			hi_num=min_num;
		  }
		  srand(time(NULL) + (getpid()*2*random_number(1,1000)));
		  result = (rand()%(hi_num-low_num))+low_num;
		  return result;
	}
	
	int randNum = min_num;
	int result =0,low_num=0,hi_num=0;
	if(min_num < max_num)
	{
		low_num=min_num;
		hi_num=max_num+1;
	}
	else
	{
		low_num=max_num+1;
		hi_num=min_num;
	}
	// srand(time(NULL) - (getpid()));
	result = ((fast_rand())%(hi_num-low_num))+low_num;
	return result;
}
void AddTime(int addNano)
{
	OssClock->nanoSec +=addNano; //adding schedular time for removing the process and changing to next proceses
	if(OssClock->nanoSec >= NANOSECOND)
	{
		OssClock->sec +=1;
		if(OssClock->nanoSec > NANOSECOND)
		{
			OssClock->nanoSec = OssClock->nanoSec - NANOSECOND;
		}
	}
}
void releaseMem()
{
	if((shmctl(pcbID , IPC_RMID, NULL)) == -1) //detach from shared memory
	{
		perror("Error in shmdt in Parent");
	}
	if((shmctl(clockID, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
	{ 
		printf("Error in shmclt"); 
	}
	/*if((shmctl(rcbID, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
	{ 
		printf("Error in shmclt child rcbID"); 
	}
		
	if ((msgctl(dieMsgID, IPC_RMID, NULL) == -1))
	{
		perror("Erorr in msgctl ");
	}	*/
}
