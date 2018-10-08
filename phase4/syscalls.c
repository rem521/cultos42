// syscalls.c
// calls to OS services

#include "constants.h"

void Sleep(int sec) {       // # of seconds to sleep
   asm("movl %0, %%eax;     // service #162 (SLEEP)
        movl %1, %%ebx;     // sleep period in ticks
        int $128"
       :
       : "g" (SLEEP), "g" (sec)
       : "eax", "ebx"       // used registers
       );
}

int GetPid(void) {
   int pid;

   asm("movl %1, %%eax;     // service #20 (GETPID)
        int $128;           // interrupt!
        movl %%ebx, %0"     // after, copy eax to variable 'pid'
       : "=g" (pid)         // output
       : "g" (GETPID)       // input
       : "eax", "ebx"       // used registers
    );

   return pid;
}

void SetVideo(int row, int col) {
    asm("movl %0, %%eax;
         movl %1, %%ebx;
         movl %2, %%ecx;
         int $128"
        : 
        : "g" (SETVIDEO), "g" (row), "g" (col)
        : "eax", "ebx", "ecx");
         
}

void Write(int device, char *str) {
    asm("movl %0, %%eax;
         movl %1, %%ebx;
         movl %2, %%ecx;
         int $128"
         :
         : "g" (WRITE), "g"(device), "g"(str)
         :"eax", "ebx", "ecx");

}

int SemInit(int passes){
    int id;
    asm("movl %1, %%eax;
         movl %2, %%ebx;
         int $128
         movl %%ecx, %0;" 
         : "=g" (id)
         : "g" (SEMINIT), "g" (passes)
         : "eax", "ebx", "ecx");

    return id;

}

void SemWait(int id){
     asm("movl %0, %%eax;
	  movl %1, %%ebx;
	  int $128;"
	  :
	  : "g" (SEMWAIT), "g" (id)
	  : "eax", "ebx");
}


void SemPost(int id){
     asm("movl %0, %%eax;
	  movl %1, %%ebx;
	  int $128;"
	  :
	  : "g" (SEMPOST), "g" (id)
	  : "eax", "ebx");
}



