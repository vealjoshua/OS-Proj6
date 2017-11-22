Joshua Veal-Briscoe
Project 4

/** You can clear all shared memory by using ./memClear *//

-----commands-----
-h or -help  : shows steps on how to use the program 
-s x         : x is the maximum number of slave processes spawned (default 8) 
-l filename  : change the log file name (default test.out)
-t z         : parameter z is the time in seconds when the master will terminate itself (default 20) 
-v verboes   : changes what is printed to the logfile (default 1)

-------------------Weird Things------------------------------
Since the user chance to terminate was asked to be so high 1000 +/- 100 many times no user terminate by the time program ends

Other than that programs works alright.
I do print the entire Frame table when the Verbous is set to 0 | I DO ON PRINT EMPTY FRAMES
I Print the page location for each user regardless of Verbous Setting

--------------------Frame Table Guide Lines-------------------------------------------------  
Frame = Frame Number | p = pageNumber | usPid = userPid | Time = accessTime | dirty = 0 read / 1 write to file | state = 0 free / 1 filled / 2 reclaimabel 
--------------------Page Location Guide Lines -------------------------------------------
User PID FrameLocation : Page |  if the page has never is accessed before then . is showen

of to finish Compilers ta ta 
