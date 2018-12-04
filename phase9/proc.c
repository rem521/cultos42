// proc.c, 159
// all processes are coded here
// processes do not access kernel data, must use syscalls

#include "constants.h" // include only these 2 files
#include "types.h"
#include "syscalls.h"
#include "data.h" // or add a pointer to car_sem (*car_sem)
#include "lib.h"

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

void Ouch(){
  int device;
  int my_pid;
  my_pid=GetPid();
  if(my_pid%2==0)
    device = TERM0;
  if(my_pid%2==1)
    device = TERM1;


  Write(device, "Ouch, dont touch that!\n\r");
}

void Wrapper(func_p_t handler_p){
  asm("pushal");
  handler_p();
  asm("popal");
  asm("movl %%ebp, %%esp; popl %%ebp; ret $4" ::);
}

void ChildCode(){
  int my_pid, device;
  int ppid;
  char str[3];
  my_pid = GetPid();
  ppid = GetPpid();
  str[0] = my_pid / 10 + '0' ;
  str[1] = my_pid % 10 + '0';
  str[2] = '\0';
  if(ppid%2==0)
     device = TERM0;
  if(ppid%2==1)
     device = TERM1;
  Write(device, "\r\n");
  Write(device,"I'm child PID");
  Write(device, str);
  Write(device, "\n\r");
  Sleep(my_pid);
  Exit(my_pid * 5);
}

void ChldHandler(){
  int my_pid, device, ec, cpid;
  char str[3], cstr[3], ecstr[3];
  my_pid = GetPid();
  str[0] = my_pid / 10 + '0' ;
  str[1] = my_pid % 10 + '0';
  str[2] = '\0';

  if(my_pid%2==0)
    device = TERM0;
  if(my_pid%2==1)
    device = TERM1;
  cpid=Wait(&ec);
  cstr[0] = cpid / 10 + '0' ;
  cstr[1] = cpid % 10 + '0';
  cstr[2] = '\0';
  ecstr[0] = ec / 10 + '0' ;
  ecstr[1] = ec % 10 + '0';
  ecstr[2] = '\0';
  //Write calls from Wait
  Write(device, "my_pid ");
  Write(device, str);
  Write(device, ", cpid ");
  Write(device, cstr);
  Write(device, ", ec ");
  Write(device, ecstr);
  Write(device, "\n\r");
  Signal(SIGCHLD, 0);
}

void TermProc(){ 
   int my_pid, child, device, fg, ec;
   char str[3], cstr[3], ecstr[3];
   char buff[BUFF_SIZE];

  // get my PID and make a string from it (null-delimited)
   my_pid = GetPid();
   str[0] = my_pid / 10 + '0' ;
   str[1] = my_pid % 10 + '0';
   str[2] = '\0';

   if(my_pid%2==0)
     device = TERM0;
   if(my_pid%2==1)
     device = TERM1;

   Signal(SIGINT, Ouch);

   while(1){
        Write(device, str);
        Write(device, " enter > ");
        Read(device, buff);
        Write(device, "\r\nentered: ");
        Write(device, buff);
        Write(device, "\r\n");
        if(StrCmp(buff, "fork")) fg=1;
        else if(StrCmp(buff, "fork&")) fg=0;
        else continue;
        if(fg==0) Signal(SIGCHLD, ChldHandler);
        child=Fork();
        switch(child){
          case -1:
            Write(device, "OS failed to fork!\n\r");
            break;
          case 0:
            ChildCode();
            break; 
          default:
            Sleep(my_pid * 2);
            if(fg==1){
              child=Wait(&ec);
              cstr[0] = child / 10 + '0' ;
              cstr[1] = child % 10 + '0';
              cstr[2] = '\0';
              ecstr[0] = ec / 10 + '0' ;
              ecstr[1] = ec % 10 + '0';
              ecstr[2] = '\0';
              //Write calls from Wait
              Write(device, "my_pid ");
              Write(device, str);
              Write(device, ", cpid ");
              Write(device, cstr);
              Write(device, ", ec ");
              Write(device, ecstr);
              Write(device, "\n\r");
            }
         }
   }     
}





