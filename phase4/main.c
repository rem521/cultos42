// main.c, 159
// OS bootstrap and The Kernel for OS phase 1
// 
// Team Name: cultOS (Members: Jeremiah Cabugao, Ryan Naatz)

#include "include.h"  // given SPEDE stuff
#include "types.h"    // kernle data types
#include "lib.h"      // small handly functions
#include "proc.h"     // all user process code here
#include "isr.h"      // kernel ISR code
#include "entry.h"    // entries to kernel (TimerEntry, etc.)

// kernel data are all here:
int cur_pid;                        // current running PID; if -1, none selected
q_t ready_q, avail_q, sem_q;               // avail PID and those created/ready to run
pcb_t pcb[PROC_MAX];                // Process Control Blocks
char stack[PROC_MAX][STACK_SIZE];   // process runtime stacks
int sys_ticks;
unsigned short *video_p;

sem_t sem[SEM_MAX];
int car_sem;
term_if_t term_if[TERM_MAX];


void InitKernel(void) {             // init and set up kernel!
   int i;
   struct i386_gate *IVT_p;         // IVT's DRAM location

   IVT_p = get_idt_base();   // get IVT location

   fill_gate( &IVT_p[TIMER], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0 ); // fill out IVT for timer
   outportb(PIC_MASK, MASK);                   // mask out PIC for timer

   fill_gate(&IVT_p[SYSCALL], (int)SyscallEntry, get_cs(), ACC_INTR_GATE, 0);
   
   fill_gate(&IVT_p[TERM0], (int)Term0Entry, get_cs(), ACC_INTR_GATE, 0);

   fill_gate(&IVT_p[TERM1], (int)Term1Entry, get_cs(), ACC_INTR_GATE, 0); 

   Bzero((char *)&ready_q, sizeof(q_t));                      // clear 2 queues
   Bzero((char *)&avail_q, sizeof(q_t));
   Bzero((char *)&sem_q, sizeof(q_t));
   
   for(i=0; i<Q_SIZE; i++) {    // add all avail PID's to the queue
      EnQ(i, &avail_q); 

   }

   for(i=0; i<Q_SIZE; i++) {
      EnQ(i, &sem_q);

   }
   
   sys_ticks = 0;
   video_p = HOME_POS;
   cur_pid = -1;

}

void Scheduler(void) {             // choose a cur_pid to run
   if( cur_pid > 0) return; // a user PID is already picked

   if(ready_q.size == 0 && cur_pid == 0) return; // InitProc OK

   if(ready_q.size == 0 && cur_pid == -1) {
      cons_printf( "Kernel panic: no process to run!\n");
      breakpoint();                                  // to GDB we go
   }

   if(cur_pid != -1) {
     // 1. append cur_pid to ready_q; // suspend cur_pid
     	EnQ(cur_pid, &ready_q);
     // 2. change its state
        pcb[cur_pid].state = READY;
   }
  // replace cur_pid with the 1st one in ready_q; // pick a user proc

   cur_pid = DeQ(&ready_q);                          // reset process time
   pcb[cur_pid].time=0;
   pcb[cur_pid].state = RUN;                          // change its state
}

int main(void) {                       // OS bootstraps
  // initialize the kernel-related stuff by calling ...
   InitKernel();

   NewProcISR(InitProc);            // create InitProc

   //call Scheduler() to set cur_pid to the 1st PID;
   Scheduler();

  // call Loader(with its TF_p);         // load proc to run
   Loader(pcb[cur_pid].TF_p);

   return 0; // statement never reached, compiler needs it for syntax
}

void TheKernel(TF_t *TF_p) {           // kernel runs
   char ch;

   pcb[cur_pid].TF_p = TF_p; // save TF addr

//   TimerISR();                     // handle tiemr event

   switch(TF_p->entry) {
      case SLEEP:
        SleepISR();
        break;
      case GETPID:
        GetPidISR();
        break;
      case SETVIDEO:
        SetVideoISR();
        break;
      case WRITE:
        WriteISR();
        break;
      case TIMER:
        TimerISR();
        break;
      case SEMINIT:
        SemInitISR();
        break;
      case SEMWAIT:
        SemWaitISR();
        break;
      case SEMPOST:
        SemPostISR();
        break;
      default:
        cons_printf("Entry issue");
        breakpoint();
   }

   if(cons_kbhit()) {                  // if keyboard pressed
      ch= cons_getchar();    //get the pressed key/character
      if(ch=='b'){                     // 'b' for breakpoint
         breakpoint();                       // go into GDB
      }
      if(ch == 'n' ){                     // 'n' for new process
         NewProcISR(UserProc); // create a UserProc
      }
      if(ch == 'c'){
         NewProcISR(CarProc);
      }
   }
   
   Scheduler(); // which may pick another proc
   Loader(pcb[cur_pid].TF_p); // load proc to run!
}

