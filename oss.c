#include "oss.h"



int main(int argc, char *argv[])
{
	signal(SIGALRM, on_alarm); // getting is alarm is activated
	signal(SIGQUIT,INThandler); //Handels SIGQUIT
	signal(SIGINT, INThandler);  // Catching crtl^c
	
	
	char *filename = "test.out";  // file name for to be written to

	/* Checking for arguments */
	if (argc < 2) //Cheking if the user is running the program with arguments 
	{
		printf("Running Program without any Commands, default options will be apply.\n");
	}
	else //calling commmand options if user inputs commands 
	{
		comOptions(argc,argv,&x,&z, &v, &filename); //sending commands to getopt
	}
	test(x,z,filename);
	
	/*filePtr global to make it sure easy for me */
	filePtr = fopen (filename, "w"); //using W to override the previous file
    if (filePtr == 0)
    {
      perror("Could not open file ");
      exit(0);
    }
	
	/* Creating Shared Memory in fuction shareCreate() */
	shareCreat(); //Fuctions that Creats all the shared memory
	
	// Getting Char's to pass to to the user
	char* pcbArrLocationPass = malloc(sizeof(int));
	char* clockIDpass = malloc(sizeof(int));
	char* pcbIDpass = malloc(sizeof(int));
	char* msgIDpass = malloc(sizeof(int));
	sprintf(clockIDpass, "%d", clockID);
	sprintf(pcbIDpass, "%d", pcbID);
	sprintf(msgIDpass, "%d", msgID);
	
	fprintf (filePtr,"\n--------------------------Running Program-------------------------\n");
	printf("\n--------------------------Running Program-------------------------\n");
	
	int forkAtRand =random_number(OssClock->nanoSec,OssClock->nanoSec+500000);

	queue *waitQueue;
	waitQueue = NULL;
	int test = 0;
	alarm(z);
	while(1)
	{
		if(OssClock->sec >= 100)
		{
			printf("\nProgram Has Finished Correctly\n");
			break;
		}
		
		AddTime(100); //adding time to the clock
		
		if(forkAtRand <= OssClock->nanoSec)
		{
			if (count < x) //this cannot exceed more than 18 process
			{	
				makeNewProcess(pcbArrLocationPass,pcbIDpass,clockIDpass,msgIDpass); //getting Random Second to fork next time
				count++; //couting up number of process currently running 
				printf("Number of Process Running: %d | Total Number of Process Created: %d\n", count, processMade);
				forkAtRand =  random_number(OssClock->nanoSec,OssClock->nanoSec+500000);
			}
			else
			{
				forkAtRand =  random_number(OssClock->nanoSec,OssClock->nanoSec+500000);
			}
		}
		msg.msgType = -1; //this just reset the msgType so I don't go through the if statements if there is no message 
		msg.type = 1;
		
		while((msgrcv(msgID , &msg , sizeof(REQ) , 0 , IPC_NOWAIT) >= 0))
		{
			if (msg.msgType == 1) //User is asking for read / write
			{
				int check = checkInFrame(&msg); //Checking if the Page is in the frame table
				if (check == -1) // page fault | Page is not in the frame 
				{
					pcb[msg.sender].critSectionAccess = 1; // User will enter critical section
					pcb[msg.sender].totalPageFaults++; //adding up all the page faults the user will have
					if(v)
						printToFile(4,0);
					/*adding Time the msg request was put in the Queue */
					msg.accessTime.nanoSec = OssClock->nanoSec;
					msg.accessTime.sec = OssClock->sec;
					/*putting msg into the wait Queue */
					addToQueue(&waitQueue, &msg);
					printQueue(&waitQueue);
				}
				else if (check == 0) //Page is in the frame table So the process gets to run 
				{
					if(v)
						printToFile(6,0); 
					AddTime(1000); //adding 10 nanosec for the OSS to deal with the page request
					pcb[msg.sender].critSectionAccess = 2; // giving the user his chance to access their critical section and add to the criical section
					while (pcb[msg.sender].critSectionAccess == 2)
					{
						if(pcb[msg.sender].critSectionAccess == 0) // user is finished with this critical section
							break; //this prevents the same user who's page found the keep sending message back to back
					}
					// break; //this prevents the same user who's page found the keep sending message back to back
				}
			}
			else if (msg.msgType == 2) // if the is ready to exit
			{
				pcb[msg.sender].valid = 0;
				count--;
				processFinished++;
				fprintf(filePtr, "\n-------------------------------------\n");
				fprintf(filePtr,"USER Exit %d Pid %d \n", msg.sender,msg.userPid);
				fprintf(filePtr,"\t Total Num of Page Faults %d \n", pcb[msg.sender].totalPageFaults);
				int p = pcb[msg.sender].totalMemAccess / OssClock->sec;
				fprintf(filePtr,"\t Memory Access Per second %d \n", p);
				fprintf(filePtr,"Page Allocation \n");
				fprintf(filePtr, "Pages : FrameLocation -> ");
				int i = 0;
				for (i; i < 32; i++)
				{
					if (pcb[msg.sender].pgAcess[i] == -1)
						fprintf(filePtr,"  %d: . |", i);
					else
						fprintf(filePtr,"  %d : %d |", i,pcb[msg.sender].pgAcess[i]);
				}
				fprintf(filePtr,"\n");

				fprintf(filePtr, "\n-------------------------------------\n");
				removeAllPages(msg.sender, msg.userPid);	
				pcb[msg.sender].critSectionAccess = 2;
			}
		}
		
		/* checking if it has been 15 milliseconds since the time the process was added to the queue */
		if(waitQueue) //checking if there is a queue
		{
			int check = waitQueue->msg.accessTime.nanoSec + 15000000; //checking if 15ms has passwed
			if (check > NANOSECOND) //precatuion of the request is made close to a full second
				check = check - NANOSECOND;
			if((check <= OssClock->nanoSec))
			{
				AddToTable(waitQueue->msg); //adding to the table
				pcb[waitQueue->msg.sender].critSectionAccess = 2; // giving the user his chance to access their critical section and add to the criical section
				while (pcb[waitQueue->msg.sender].critSectionAccess == 2)
				{
					if(pcb[waitQueue->msg.sender].critSectionAccess == 0) // user is finished with this critical section
						break;
				}
				remNode(&waitQueue);
				AddTime(15000000); //adding 15 milliseconds
			}
			else if(qSize == x) // if all the process are waiting to read their page 
				AddTime(1000000);
		}
		if( OssClock->sec % 2 == 0 && test == OssClock->sec)
		{
			test = OssClock->sec + 2;
			if(v == 0)
				printToFile(2,0);
			int i = 0;
			int j = 0;
			for (j; j < count ; j++)
			{
				fprintf(filePtr, "\n %d = FrameLocation : Page-> ", pcb[j].userPid);
				for (i; i < 32; i++)
				{
					if (pcb[j].pgAcess[i] == -1)
						fprintf(filePtr,"  . : %d |", i);
					else
						fprintf(filePtr,"  %d : %d |", pcb[j].pgAcess[i], i);
				}
				i = 0;
				fprintf(filePtr,"\n");
			}
			fprintf(filePtr,"\n ");
		}
		
		/* 25.6 is the 10 / 256 so 26  */
		if(checkFree() < 26) //if the free frames fall below 10%
		{
			daemon1(); //making the reclaimabel of the oldest pages and freeing any page that is alread reclaimabel
		}
	}
	
	releaseMem();
	int i = 0;
	for (i=0; i<x;i++)
	{
		kill(pcb[i].userPid, SIGTERM); // killing em child by children
		wait(NULL);
	}
	return 0;
}
void comOptions (int argc, char **argv , int *x, int *z, int *v, char **filename)
{ 
	int c = 0; //This is for the switch statement
	int temp = 0;
	static struct option long_options[] = 
	{ 
		{"help", no_argument, 0, 'h'},
		{ 0,     0          , 0,  0 } 
	};
	int long_index = 0;
	while((c = getopt_long_only(argc, argv, "hs:t:v:l:", long_options, &long_index)) != -1)
	{
		switch (c)
		{
			case 'h':  // -h
				displayHelpMesg();
			break;
	  
			case 's':
				temp = *x;
				*x = atoi(optarg);
				if (*x > 18)
				{
					printf("Inputed: %d is to big. (Limit 18). Reverting back to default 5.\n", *x);
					*x = temp;
				}
				validate(x,temp,'x');
			break;

			case 't':
				temp = *z;
				*z = atoi(optarg);
				validate(z,temp,'z');
			break;
			
			case 'v':
				*v = atoi(optarg);
				if (*v > 1 || *v < 0 )
				{
					printf("Verbous input %d incorret exiting process\n", *v);
					exit(1);
				}
				break;

			case 'l':
				if (optopt == 'n')
				{
					printf("Please enter a valid filename.");
					return;
				}
				*filename = optarg;
			break;
      
			case '?':
				if (optopt == 'l')
				{
					printf("Command -l requires filename. Ex: -lfilename.txt | -l filename.txt.\n");
					exit(0);
				}
				else if (optopt == 's')
				{
					printf("Commands -s requires int value. Ex: -s213| -s 2132\n");
					exit(0);
				}
				else if (optopt == 'i')
				{
					printf("Command -y requires int value. Ex: -i213| -i 2132\n");
					exit(0);
				}
				else if (optopt == 't')
				{
					printf("Command -z requires int value. Ex: -t13| -t 2132\n");
					exit(0);	
				}
				else
				{
					printf("You have used an invalid command, please use -h or -help for command options, closing program.\n"); 
					exit(0);
				}
				return;
	  
			default :
				if (optopt == 'l')
				{
					printf ("Please enter filename after -l \n");
					exit(0);
				}
				else if (optopt == 'n')
				{ 
					printf ("Please enter integer x after -n \n");
					exit(1);
				}
				printf("Running Program without Commands.\n");
			break;
		}
	}
  
}
void displayHelpMesg()
{
	printf ("---------------------------------------------------------------------------------------------------------\n");
	printf ("Please run oss | ./oss or oss -command arguemtns | ./oss -command arguments .\n");
	printf ("----------Arguments---------------------------------------------\n");
	printf (" -h or -help  : shows steps on how to use the program \n");
	printf (" -s x         : x is the maximum number of slave processes spawned (default 18) \n");
	printf (" -l filename  : change the log file name \n");
	printf (" -v verboes   : changes what is printed to the logfile (default 1)\n");
	printf (" -t z         : parameter z is the time in seconds when the master will terminate itself (default 20) \n"); 
	printf ("---------------------------------------------------------------------------------------------------------\n");
	printf ("\nClosing Program.............\n\n");
	exit(0);
}
void validate(int *x,int temp,char y)
{
  char *print;
  char *print2;
  if (y == 'z')
  {
    print = "z";
    print2 = "-t";	  
  }
  else if (y == 'x')
  {
    print = "x";
    print2 = "-s";	  
  }
  
  
  if (*x == 0)
  {
    printf("Intput invalid for %s changing %s back or default.\n",print2,print);
    *x = temp;
  }
  else if (*x < 0)
  {
    printf("Intput invalid for %s changing %s back or default.\n",print2,print);
    *x = temp; 
  }
}
void test (int x,int z, char *file)
{
	printf ("--------------------------------\n");
	printf ("Number of Slaves (x): %d\n", x);
	printf ("Time limit       (z): %d\n", z);
	printf ("Verbose          (v): %d\n", v);
	printf ("Filename            : %s\n", file);
	printf ("--------------------------------\n\n");
	printf("Running Program Now.\n");
	return;
}
void INThandler(int sig)
{
	signal(sig, SIG_IGN);
	printf("\nCtrl^C Called. Closing All Process.\n");
	int i =0;
	for (i=0; i<x;i++)
	{
		kill(pcb[i].userPid, SIGTERM); // killing em child by children
		wait(NULL);
	}
	releaseMem();
	fflush(stdout);
	exit(0);
}
void on_alarm(int signal)
{
	printf("Out of time killing all slave processes.\n", z);
	printf("Time: %d.%d\n", OssClock->sec,OssClock->nanoSec);
	int i = 0;
	releaseMem();
	for (i=0; i<x;i++)
	{
		kill(userArr[i], SIGTERM); // killing em child by children
		wait(NULL);
	}
	releaseMem();
	// signal(signal, SIG_IGN);
    exit(0);
}
void shareCreat()
{
	/* Creating a shared memeory for OssClock The OSS CLOCK */
	if((clockID = shmget(Clockkey, sizeof(OssClock), IPC_CREAT | 0666)) < 0)  // creating the shared memory 
    {
		releaseMem();
		perror("shmget in parent OssClock");
		exit(1);
	}
	OssClock = (systemClock*)shmat(clockID, NULL, 0);
	if(OssClock->nanoSec == (int)-1) // Now we attach the segment to our data space.
	{
		releaseMem();
		perror("Shmat error in Main OssClock");
		exit(1);
	}
	
		/* Creating a shared memory for the PCB struct */
	if((pcbID = shmget(PCBkey, sizeof(pcb[x]), IPC_CREAT | 0666)) == -1)
	{
		perror("shmget parent pcb");
		releaseMem();
		exit(1);
	}
	pcb = (PCB*)shmat(pcbID, NULL, 0);
	if((void*)pcb == (void*)-1)
	{
		perror("shmat parent pcb");
		releaseMem();
		exit(1);
	}
	
	/* Creating memory for Meges */
	if((msgID = msgget(MSGkey, IPC_CREAT | 0666)) < 0)
	{
		perror("shamt Parent msg(REQ) ");
		releaseMem();
		exit(0);
	}
	
	//Setting all pcb's in shared memory to 0
	int counter = 0;
	for(counter; counter < x; counter++)
	{
		pcb[counter].valid = 0; //premaking all the pcb's 		
	}
	
	/*Setting up the Inital Page table */
	int i = 0;
	for(i; i < TotalMem; i++)
	{
		table[i].frame = i;
		table[i].p = 0;
		table[i].userPid = 0;
		table[i].accessTime.nanoSec = 0;
		table[i].accessTime.sec = 0;
		table[i].dirty = 0;
		table[i].state = 0;
		table[i].pcb = -1;
	}
	// printToFile(2,0);
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

void makeNewProcess(char * pcbArrLocationPass,char *pcbIDpass,char *clockIDpass, char *msgIDpass)
{
	int i = 0;
	if ((i=findValidPCB()) > -1) //Finding a palce in the pcb array
	{	
		pid_t childPid; 
		childPid = fork();
		
		if (childPid < 0)
		{
			perror("Fork Parent Error ");
			kill(getpid(), SIGTERM);
			releaseMem();
			exit(1); 
		}
		else if (childPid == 0)
		{
			sprintf(pcbArrLocationPass, "%d" , i);
			execl("./user",pcbArrLocationPass, pcbIDpass,clockIDpass,msgIDpass,NULL); //Making the process
				perror("Child failed to execl");
		}
		setUpPCB(i, childPid); //setting up everything aobut PCB here
		processMade++; //Counting up the number of process made
		if(v) {printToFile(1, i);}
	}
}
int findValidPCB()
{
	int i = 0;
	for (i; i < x; i++)
	{
		if(pcb[i].valid == 0) //if its no longer being used I can place another here
		{
			if(pcb[i].userPid > 0) //This is a check, If I failed to kill a pcb
			{
		 		// printf("Please don't Happen\n");
				kill(pcb[i].userPid,SIGUSR1);
				wait(NULL);
			}
			// printf("I'm return pcb that's free %d\n", pcb[i].userPid);	
			return i;
		}
	}
	return -1;
}

void setUpPCB(int i, pid_t childPid)
{
	pcb[i].userPid = childPid;
	pcb[i].valid = 1; //setting the user to be valid so it can't be replaced
	pcb[i].critSectionAccess = 0;  //user starts with no wait
	pcb[i].totalMemAccess = 0;
	pcb[i].totalPageFaults = 0;
 	// pcb[i].pgNum = -1;
	int p = 0;
	while (p != 32)  // resetting the Initial array for each pcb
	{
		pcb[i].pgAcess[p] = -1;
		// printf("Pcb %d pgAcess[%d] %d\n", i, p , pcb[i].pgAcess[p]);
		p++;
		
	}
}
int random_number(int min_num, int max_num)
{
	if (max_num > 10000 || min_num >= max_num)
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
	else
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
		// srand(time(NULL) - (getpid()));
		result = (fast_rand()%(hi_num-low_num))+low_num;
		return result;
	} 
}
int checkInFrame(REQ *msg)
{
	/* First check if that array location of the psb was accesed before if not then there will be a page fault	*/
	if (pcb[msg->sender].pgAcess[msg->pgNum] == -1) // page has not been accesed before 
		return -1;  // page fault this is the first time the page is accesed
	else if (table[pcb[msg->sender].pgAcess[msg->pgNum]].p == msg->pgNum && table[pcb[msg->sender].pgAcess[msg->pgNum]].userPid == msg->userPid)
	{ //Checking if the Frames and the pid is the same
		// printf("Frame Location %d \n",pcb[msg->sender].pgAcess[msg->pgNum]); //Test Message

		table[pcb[msg->sender].pgAcess[msg->pgNum]].state = 1; //updating Sate to 1 because the user has accesed the page in the frame
				
		table[pcb[msg->sender].pgAcess[msg->pgNum]].accessTime.nanoSec = OssClock->nanoSec; //changing session time
		table[pcb[msg->sender].pgAcess[msg->pgNum]].accessTime.sec = OssClock->sec;
			
		if(msg->rWchance == 2)
			table[pcb[msg->sender].pgAcess[msg->pgNum]].dirty = 1; // setting dirty to 1 
		/*fprintf (filePtr, "User Accesing -----> "); 
		printToFile(7,0); */
			
		return 0; //User gets to access the page 
	}
	else // page number has been changed so another user is access to
	{
		if(v)
		{
			printToFile(5,0);
			printToFile(7,pcb[msg->sender].pgAcess[msg->pgNum]);
			fprintf(filePtr,"\n");	
		}
		return -1;
	}
}
int findFrame()
{
	//table state 0 = empty | 2 = reclaimabel
	int i = 0;
	for(i; i < TotalMem; i++)
	{
		if(table[i].state == 0 || table[i].state  == 2) 
		{
			if(table[i].state  == 2) //if reclaimabel then first writing to file
			{
				if(table[i].dirty == 1)
				{
					if(v)
					{
						fprintf(filePtr,"@Replacing Frame %d dirty %d Writing to File before Over Writing\n", i, table[i].dirty);
						fprintf(filePtr,"\tFor User %d\n", pcb[table[i].pcb].userPid);
						printToFile(7,i);
					}
				}
				else
				{
					if(v)
					{
						fprintf(filePtr,"@Replacing Frame %d dirty %d \n", i, table[i].dirty);
						fprintf(filePtr,"\tFor User %d\n", pcb[table[i].pcb].userPid);
						printToFile(7,i);
					}
				}
				pcb[table[i].pcb].pgAcess[i] = -1;
				return i;
			}
			else
				return i;
		}
	}
}
void AddToTable(REQ msg)
{
	int ff = findFrame(); // frae fraem found
	// printf("Free Frame %d | ", ff);
	// printf("Adding to User %d", msg.sender);
	table[ff].p = msg.pgNum;
	table[ff].userPid = msg.userPid;
	table[ff].accessTime.nanoSec = OssClock->nanoSec;
	table[ff].accessTime.sec = OssClock->sec;
	table[ff].state = 1;
	table[ff].dirty = 0;
	
	if(msg.rWchance == 2)
		table[ff].dirty = 1;
	
	table[ff].pcb = msg.sender;
	pcb[msg.sender].pgAcess[msg.pgNum] = ff;
	if(v)
	{
		fprintf(filePtr, "Page Added to Table ->>> \t ");
		printToFile(7,ff);
	}
	
}
int checkFree()
{
	int add = 0; // counter 

	int i = 0;
	for (i; i< TotalMem; i++)
	{
		if (table[i].state == 0) // Counting all the free frames
			add++;	
	}
	return add;
}
void daemon1()
{
	int oldCheck = table[0].accessTime.nanoSec; //startin initial search with first frame
	int secCheck = table[0].accessTime.sec; // taking record of the time for the fist frame
	int frameLocation = 0; //storing the location of the frame in which the oldest frame is found
	int totalPages = 0; //getting total pages in the frame tables
	int i = 0;
	for (i; i< TotalMem; i++) //going through the frame table
	{
		if (table[i].userPid > 0) // if there is a user then the pid is larger than 0 (default is 0)
			totalPages++; //couting up the total number of pages
		/* Checking if the next frame nanoSec are smaller (older) and the next frames Seconds are smaller (older)  */
		/* Ignoring any page that is already set to free */
		if (oldCheck > table[i].accessTime.nanoSec && secCheck >= table[i].accessTime.sec && table[i].state != 0)
		{	
			frameLocation = i; //get the location of the frame
			secCheck = table[i].accessTime.sec;
			oldCheck = table[i].accessTime.nanoSec;
		}
	}
	// now I have the oldest time and its location
	// and I have the total number of pages in the frame table
	int change = totalPages * 0.05;  //getting 5% of the total pages  I wanted this to be an Int so it I can ignore the decimal
	int locationArr[change]; //this array is the size of the number 5% of the total pages and it holds the location of the oldest pages in the table
	locationArr[0]= frameLocation; // first one is set 
	i = 1; //starting at one because the oldest has already been found and recorded 
	for (i; i < change ; i++)
	{
		locationArr[i] = findOldestPage(i,locationArr); // I pass in the array to find the oldest that is not already recorded 
		if(table[locationArr[i]].state == 1) //if the page was used 
		{
			table[locationArr[i]].state = 2; // marking the frame reclaimabel
			if(table[locationArr[i]].dirty == 1)
			{
				if(v)
				{
					fprintf(filePtr, "##Frame %d Being Marked Reclaimable And Being Written To File : ", i);
					printToFile(7,locationArr[i]);
					fprintf(filePtr,"\n");
				}
			}
				
		}
		else //if the page in the array is already reclaimabel
		{
			table[locationArr[i]].state = 0; // marking the frame for free
			if(table[locationArr[i]].dirty == 1)
			{
				if(v)
				{
					fprintf(filePtr, "##Frame %d Being Marked FREE And Being Written To File : ", i);
					printToFile(7,locationArr[i]);
					fprintf(filePtr,"\n");
				}
			}
		}
	}
	

}
int findOldestPage(int size, int arr[])
{
	int oldCheck = table[0].accessTime.nanoSec; //startin initial search with first frame
	int secCheck = table[0].accessTime.sec; 
	int frameLocation = 0;
	int check = 1; //this is 1 if the oldest found is 
	int i = 0; //count to go through the frame table
	int i2 = 0; //count to go through the array's holding the location of the oldest 
	for (i; i< TotalMem; i++)
	{
		/* Checking if the next frame nanoSec are smaller (older) and the next frames Seconds are smaller (older)  */
		/* Ignoring any page that is already set to free */
		if (oldCheck > table[i].accessTime.nanoSec && secCheck >= table[i].accessTime.sec && table[i].state != 0)
		{	
			for(i2;i2<size;i2++)
			{
				if(table[i].frame == arr[i2])
				{
					// fprintf(filePtr, "Checking fram %d  | Recorded %d",table[i].frame,arr[i2] );
					check = 0;
					break;
				}
			}
			// fprintf(filePtr,"\n");
			i2 = 0; //reseting i2
			if (check == 1)
			{
				frameLocation = i;
				secCheck = table[i].accessTime.sec;
				oldCheck = table[i].accessTime.nanoSec;
			}
			check = 1; // reseting check
		}
	}
	return frameLocation;
}
void removeAllPages(int i, pid_t pid)
{
	int c = 0;
	for (c; c < 32 ; c++)
	{
		if (pcb[i].pgAcess[c] != -1)
		{
			table[pcb[i].pgAcess[c]].state = 0; // freeing up the state
			if(v)
			{
				if(table[pcb[i].pgAcess[c]].dirty == 1)
				{
					fprintf(filePtr,"User %d %d Exiting Writing Page %d To File\n", i , pid,table[pcb[i].pgAcess[c]].p);
				}
			}
		}
		
	}
}
void addToQueue(queue **head, REQ *msg)
{
	queue *newNode = malloc(sizeof(queue));
	newNode->userNum = msg->sender;
	newNode->userPid = msg->userPid;
	newNode->msg = *msg;
	newNode->next = NULL;
	// printf("Adding %d\n", msg->sender);

	if(!*head)
		*head = newNode;
	else
	{
		queue *nodePtr = malloc(sizeof(queue)); // I create a node pointer to search the queue
		nodePtr = *head; //Start the search at the head
		while(nodePtr->next) //NodePtr keeps going untill It hits the end where the next is NULL
			nodePtr = nodePtr->next; //NodePtr moving to the next block
		nodePtr->next = newNode; //Settings the 
		free(nodePtr);
	}
	qSize++; // adding up the qSize 
}
void remNode(queue **head)
{
	if(qSize == 0) //if the head is null
		return;
	
	queue* nodePtr = malloc(sizeof(queue));
	nodePtr = *head; //pointing to the head
	
	if((*head)->next) // if there is a next node to set it too
		*head = (*head)->next; //set the new head
	else
		*head = NULL;
	
	free(nodePtr); //remove the old node
	qSize--;
}
void printQueue(queue **head)
{
	if(qSize == 0)
		return;
	queue *ptr = malloc(sizeof(queue));
	ptr = *head;
	
	printf("Current Wait List | ");
	while(ptr)
	{
		printf(" %d |", ptr->msg.sender);
		ptr = ptr->next;
	}
	printf("\n");
}
void releaseMem()
{
	printToFile(99,0);
	fclose(filePtr);
	if((shmctl(pcbID, IPC_RMID, NULL)) == -1) //detach from shared memory
		perror("Error in shmctl Parent pcbID ");
		
	if((shmctl(clockID, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
		printf("Error in shmctl Parent clockID ");

	
	if((msgctl(msgID, IPC_RMID, NULL)) == -1)//mark shared memory for deletion
		printf("Error in shmclt Parent msgID ");
}
void printToFile(int choice, int index)
{
	/*Choice 99 is like the End statistics so I need to print that */
	if(printLineCount >= 1000 && choice != 99) //going from 0 to 2000 Lines
		return;
	else
		printLineCount++;
	
	
	int c = choice;
	switch (c)
	{
		case 0:
			fprintf(filePtr,"\tComputerTime: %d.%010d | ", OssClock->sec,OssClock->nanoSec);
			fprintf(filePtr,"Child Death Pid: %d | valid: %d \n",pcb[index].userPid,pcb[index].valid);
			break;
		case 1:
			/*fprintf(filePtr,"Master Created ");
			fprintf(filePtr,"Child: %d.%010d | ", OssClock->sec,OssClock->nanoSec);
			fprintf(filePtr,"Process #%d Pid: %d |", index,pcb[index].userPid);
			fprintf(filePtr," Am I valid: %d \n", pcb[index].valid);*/
			
			printf("Master Created ");
			printf("Child: %d.%010d | ", OssClock->sec,OssClock->nanoSec);
			printf("Process #%d Pid: %d |", index,pcb[index].userPid);
			printf(" Am I valid: %d \n", pcb[index].valid);
			break;
		case 2:
			fprintf (filePtr, "\t\t\tPage Table \n");
			fprintf (filePtr, "Frame = Frame Number | p = pageNumber | usPid = userPid | Time = accessTime"); 
			fprintf (filePtr, " | dirty = 0 read / 1 write to file | state = 0 free / 1 filled / 2 reclaimabel \n\n");
			fprintf(filePtr, "Frame  \t p  \t usPid   Time  \t\t Dirty \t State \n");
			int i = 0;
			for(i; i < TotalMem; i++)
			{
				if(table[i].userPid > 0) //this helps cut down empty frames
				{
					fprintf (filePtr, "%d \t %d \t %d \t %d.%010d \t %d \t %d\n",table[i].frame, table[i].p, table[i].userPid,table[i].accessTime.sec, 
						table[i].accessTime.nanoSec, table[i].dirty, table[i].state);
				}
			}
			
			break;
		case 3:
			fprintf(filePtr,"\t\tUser %d Pid %d is requesting page %d ", msg.sender, pcb[msg.sender].userPid,msg.pgNum);
			if(msg.rWchance == 1) // if the user would like to read the page
				fprintf(filePtr,"to Read \n");
			else if (msg.rWchance == 2)
				fprintf(filePtr, "to Write \n");

			break;
		case 4:
			fprintf(filePtr,"\t\t\t Page Fault (Page not in Frame) for User %d Pid %d for Page Reuqest %d\n", msg.sender, pcb[msg.sender].userPid,msg.pgNum);
			break;
		case 5:
			fprintf(filePtr,"\t\t\t Page Fault (Space Replaced) for User %d Pid %d for Page Reuqest %d\n", msg.sender, pcb[msg.sender].userPid,msg.pgNum);
			break;
		case 6:
			fprintf(filePtr," Page Found And Used for User %d Pid %d for Page Reuqest %d\n", msg.sender, pcb[msg.sender].userPid, pcb[msg.sender],msg.pgNum);
			break;
		case 7:
			fprintf (filePtr, "%d \t %d \t %d \t %d.%010d \t %d \t %d\n",table[index].frame, table[index].p, table[index].userPid,table[index].accessTime.sec, 
				table[index].accessTime.nanoSec, table[index].dirty, table[index].state);
			break;
		case 8:
			
			break;
		case 99:
			fprintf(filePtr, "\n------Statistics------------\n");
			printf("\n------Statistics------------\n");
			
			fprintf(filePtr, "Program End time------------------:%d.%010d\n", OssClock->sec, OssClock->nanoSec);
			printf("Program End time------------------:%d.%010d\n", OssClock->sec, OssClock->nanoSec);
			
			fprintf(filePtr, "\nTotal Process Made----------------:%d\n",processMade);
			printf("\nTotal Process Made----------------:%d\n",processMade);
			
			fprintf(filePtr, "\nTotal Process Finished Execution--:%d\n", processFinished);
			printf("\nTotal Process Finished Execution--:%d\n", processFinished);
			break;
	}
}
