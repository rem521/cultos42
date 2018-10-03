// isr.c, 159

#include "include.h"
#include "types.h"
#include "data.h"
#include "isr.h"
#include "lib.h"
#include "proc.h"

// to create a process: alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_q
void NewProcISR(func_p_t p) {  // arg: where process code starts
   int pid;

   if(QisEmpty(&avail_q)) {    // may occur if too many been created
      cons_printf("Kernel panic: no more process!\n");
                           // cannot continue, alternative: breakpoint();
      breakpoint();
   }

   pid = DeQ(&avail_q);                  // alloc PID (1st is 0)
   Bzero((char *)&pcb[pid],sizeof(pcb_t));    // clear PCB  
   Bzero((char *)&stack[pid], STACK_SIZE);    // clear stack
   pcb[pid].state = READY;           // change process state
   
   EnQ(pid, &ready_q);                                      // queue it

// point TF_p to stack & fill it out
   pcb[pid].TF_p = (TF_t *)&stack[pid][STACK_SIZE];             
   pcb[pid].TF_p--;
   pcb[pid].TF_p->efl = EF_DEFAULT_VALUE|EF_INTR; // enables intr
   pcb[pid].TF_p->cs = get_cs();                  // duplicate from CPU
   pcb[pid].TF_p->eip = (int)p;                          // set to code
}

// count run time and switch if hitting time limit
void TimerISR(void) {
   int i;

   outportb(PIC_CONTROL, DONE);    // notify PIC getting done

   pcb[cur_pid].time++;                  // count up time
   pcb[cur_pid].life++;                  // count up life

   if(pcb[cur_pid].time == TIME_MAX) {       // if runs long enough    
      EnQ(cur_pid, &ready_q);                // move it back to ready_q
      pcb[cur_pid].state = READY;           // change its state
      cur_pid = -1;                           // now no running proc
   
   }
   sys_ticks++;
   
   for(i=0; i<20; i++){
    if(pcb[i].state == SLEEPY && pcb[i].wake_time <= sys_ticks){
        EnQ(i, &ready_q);
        pcb[i].state = READY;
     }
   }

}

void GetPidISR(void){
   pcb[cur_pid].TF_p->ebx = cur_pid;
}

void SleepISR(void){
   int sleepSecond;
   sleepSecond = pcb[cur_pid].TF_p->ebx;

   pcb[cur_pid].wake_time = sys_ticks + sleepSecond * 100;
   pcb[cur_pid].state = SLEEPY;
   cur_pid = -1; //might need to be zero!!
}

void SetVideoISR(void){
   int row, col;

   row = pcb[cur_pid].TF_p->ebx;
   col = pcb[cur_pid].TF_p->ecx;

   video_p = HOME_POS + (row-1) * 80 + (col-1);
}


void WriteISR(void){
   int device;
   char *str;
   int i,j;

   device = pcb[cur_pid].TF_p->ebx;
   str =(char *) pcb[cur_pid].TF_p->ecx;
   if(device == STDOUT) {
      for( i=0; str[i]!= '\0' ; i++){
        
        if(video_p == END_POS)
          video_p = HOME_POS;
        
        if( (video_p-HOME_POS) % 80 == 0 ){
          for(j=0; j<80; j++){          
            *video_p= ' ';
            video_p++;
          }
          video_p= video_p - 80; 
        }
        
        if( str[i] != 0x0A){
          //cons_printf(" %c", str[i]);
          *video_p= str[i] + VGA_MASK;
          video_p++;
        }
        
       else{
          video_p+=80 - (video_p-HOME_POS)%80;
        }  
      }
   }



}

void SemInitISR(){
  int id;
  id= DeQ(&sem_q);
  Bzero((char *)&sem[id].wait_q, Q_SIZE);
  sem[id].passes=pcb[cur_pid].TF_p->ebx;
  pcb[cur_pid].TF_p->ecx= id;
}

void SemWaitISR(){
  if(sem[car_sem].passes>0){
    sem[car_sem].passes--;
    return;
  }
  EnQ(cur_pid, &sem[car_sem].wait_q);
  pcb[cur_pid].state=WAIT;
  cur_pid=-1;
}

void SemPostISR(){
  int pid;
  if(sem[car_sem].wait_q.size == 0){
    sem[car_sem].passes++;
    return;
  }
  pid=DeQ(&sem[car_sem].wait_q);
  pcb[pid].state= READY;
  EnQ(pid , &ready_q);
  return;


}








