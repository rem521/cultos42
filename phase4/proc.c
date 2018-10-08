// proc.c, 159
// all processes are coded here
// processes do not access kernel data, must use syscalls

#include "constants.h" // include only these 2 files
#include "syscalls.h"
#include "data.h" // or add a pointer to car_sem (*car_sem)

void InitProc(void) {
   int i;

   car_sem = SemInit(3);

   while(1) {
      SetVideo(1,1);         // pos video
      Write(STDOUT, "." );
      for(i=0; i<LOOP/2; i++)asm("inb $0x80"); // busy CPU

      SetVideo(1, 1);         // pos video
      Write(STDOUT, " ");
      for(i=0; i<LOOP/2; i++)asm("inb $0x80"); // busy CPU
   }
}

void UserProc(void) {
   int my_pid;
   char str[3];

  // get my PID and make a string from it (null-delimited)
   my_pid = GetPid();
   str[0] = my_pid / 10 + '0' ;
   str[1] = my_pid % 10 + '0';
   str[2] = '\0';

  // set video cursor to beginning of my row
   SetVideo(my_pid+1, 1);

  // write out that extra long msg to test line wrapped and erasure
   Write(STDOUT, "Here I am, a brand new spanking process! How I love the aromatic fragrance of summer roses permeating in the cool fresh morning air... Ah...");
   
  // sleep for 2 seconds
   Sleep(2);

   while(1) {
     // call service to set video cursor to beginning of my row
     SetVideo(my_pid+1, 1);

     // call service to write out my PID str
     Write(STDOUT,(char *) &str);

     // call service to sleep for 2 seconds
     Sleep(2);

     //call service to set video cursor to beginning of my row
     SetVideo(my_pid+1, 1);

     //call service to erase my PID str (with "--")
     Write(STDOUT, "--");    

     //call service to sleep for 2 seconds
     Sleep(2);
     
   }
}

void CarProc(void) {

   int my_pid;
   char str[3];

  // get my PID and make a string from it (null-delimited)
   my_pid = GetPid();
   str[0] = my_pid / 10 + '0' ;
   str[1] = my_pid % 10 + '0';
   str[2] = '\0';

  // set video cursor to beginning of my row
   SetVideo(my_pid+1, 1);
   Write(STDOUT, str);
	
   while(1) {
      
      SetVideo(my_pid+1, 10);

      Write(STDOUT, "I'm off...           ");
      Sleep(2);

      SemWait(car_sem);

      SetVideo(my_pid+1, 10);
      Write(STDOUT, "I'm on the bridge!");
      Sleep(2);
      SemPost(car_sem);
   }
}





