#ifndef USER_H
#define USER_H

#include "share.h"

void INThandler(int sig); // for the ctrl^C
void TimeHandler(int sig); // for alarm (z)
void sigDie(int sig);  // precations 
void sigDie2(int sig);
void AddTime(int addNano);
void AttachToMem(); //all the code for the user to attached to memory 
void addToarr(int i);
int randDeath(int checkTime, int startTime, int pcbLocation); //makes checks to see if the user needs to die

int random_number(int min_num, int max_num); // getting randTime
void releaseMem();



#endif
