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
   
   for(i=0; i<PROC_MAX; i++){
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
   int term;

   device = pcb[cur_pid].TF_p->ebx;
   str =(char *) pcb[cur_pid].TF_p->ecx;
   
   if( str == '\0'){
     return;
   }
   
   if(device == TERM0 || device == TERM1){
     if( device == TERM0){
       term= 0;
     }
     if(device == TERM1){
       term= 1;
     }
     outportb(term_if[term].io, *str);
     str++;
     term_if[term].tx_p = str;
     //Block current proc
     EnQ(cur_pid, &term_if[term].tx_wait_q);
     pcb[cur_pid].state=WAIT;
     cur_pid= -1;
   }

   if(device == STDOUT) {
      for( i=0; str[i]!= '\0' ; i++){
        
        if(video_p == END_POS)
          video_p = HOME_POS;
        
        if( (video_p-HOME_POS) % 80 == 0 ){
          for(j=0; j<80; j++){          
            *video_p= ' ' + VGA_MASK;
            video_p++;
          }
          video_p= video_p - 80; 
        }
        
        if( str[i] != 0x0A){
          *video_p= str[i] + VGA_MASK;
          video_p++;
        }
        
       else{
          video_p+=80 - (video_p-HOME_POS)%80;
        }  
      }
   }
}

void ReadISR(){
   int device;
   char *str;
   int term;
   
   device = pcb[cur_pid].TF_p->ebx;
   str =(char *) pcb[cur_pid].TF_p->ecx;
   
   if( device == TERM0 ){
     term= 0;
   }
   if( device == TERM1){
     term= 1;
   }
   term_if[term].rx_p = str;
   EnQ(cur_pid, &term_if[term].rx_wait_q);
   pcb[cur_pid].state=WAIT;
   cur_pid= -1;

}

void SemInitISR(){
  int id;
  id= DeQ(&sem_q);
  if(id == -1){
     cons_printf("No more semaphores :( ");
     breakpoint();
  }
  Bzero((char *)&sem[id].wait_q, Q_SIZE);
  sem[id].passes=pcb[cur_pid].TF_p->ebx;
  pcb[cur_pid].TF_p->ecx= id;
  *(HOME_POS + 21 * 80) = sem[id].passes + '0' + VGA_MASK;  
}

void SemWaitISR(){
  if(sem[car_sem].passes>0){
    sem[car_sem].passes--;
    *(HOME_POS + 21 * 80) = sem[car_sem].passes + '0' + VGA_MASK;
    return;
  }
  EnQ(cur_pid, &sem[car_sem].wait_q);
  pcb[cur_pid].state=WAIT;
  cur_pid=-1;
  *(HOME_POS + 21 * 80) = sem[car_sem].passes + '0' + VGA_MASK;
  return;
}

void SemPostISR(){
  int pid;
  if(sem[car_sem].wait_q.size == 0){
    sem[car_sem].passes++;
    *(HOME_POS + 21 * 80) = sem[car_sem].passes + '0' + VGA_MASK;
    return;
  }
  pid=DeQ(&sem[car_sem].wait_q);
  pcb[pid].state= READY;
  EnQ(pid , &ready_q);
  *(HOME_POS + 21 * 80) = sem[car_sem].passes + '0' + VGA_MASK;
  return;


}


void TermISR(int index){
   int inport;
   inport = inportb(term_if[index].io + IIR);
   if( inport == IIR_TXRDY){
      TermTxISR(index);
   }
   if( inport == IIR_RXRDY){
      TermRxISR(index);
   }
   outportb(PIC_CONTROL, term_if[index].done); 
   return;
}

void TermTxISR(int index){
  int pid;
  if(term_if[index].tx_wait_q.size == 0) return;
  if(*term_if[index].tx_p == '\0'){
    pid=DeQ(&term_if[index].tx_wait_q);
    pcb[pid].state= READY;
    EnQ(pid, &ready_q);
  }
  else{
    outportb(term_if[index].io, *term_if[index].tx_p);
    term_if[index].tx_p++;
  }
}

void TermRxISR(int index){
  int pid;
  char inport;
  inport = (char)inportb(term_if[index].io + IIR);
  if(inport == '\n' || inport '\r'){
    outportb(term_if[index].io, inport);
    if(term_if[index].rx_wait_q.size != 0){
      term_if[index].rx_p++;
    }
    return;
  }
  

}




