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
   cur_pid = -1;
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
   
   if( *str == '\0'){
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
  Bzero((char *)&sem[id].wait_q, sizeof(sem_t));
  sem[id].passes=pcb[cur_pid].TF_p->ebx;
  pcb[cur_pid].TF_p->ecx= id;
  *(HOME_POS + 21 * 80) = sem[id].passes + '0' + VGA_MASK;  
}

void SemWaitISR(){
  int semID;
  semID = pcb[cur_pid].TF_p->ebx;
  if(sem[semID].passes>0){
    sem[semID].passes--;
    *(HOME_POS + 21 * 80) = sem[semID].passes + '0' + VGA_MASK;
    return;
  }
  EnQ(cur_pid, &sem[semID].wait_q);
  pcb[cur_pid].state=WAIT;
  cur_pid=-1;
  *(HOME_POS + 21 * 80) = sem[semID].passes + '0' + VGA_MASK;
  return;
}

void SemPostISR(){  
  int pid;
  int semID;
  semID = pcb[cur_pid].TF_p->ebx;

  if(QisEmpty(&sem[semID].wait_q)){
    sem[semID].passes++;
    *(HOME_POS + 21 * 80) = sem[semID].passes + '0' + VGA_MASK;
    return;
  }
  pid=DeQ(&sem[semID].wait_q);
  pcb[pid].state= READY;
  EnQ(pid , &ready_q);
  *(HOME_POS + 21 * 80) = sem[semID].passes + '0' + VGA_MASK;
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
  if(QisEmpty(&term_if[index].tx_wait_q)) return;
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
  inport = (char)inportb(term_if[index].io);
  if(inport == 0x3){
    if(QisEmpty(&term_if[index].rx_wait_q)) return;
    pid=DeQ(&term_if[index].rx_wait_q);
    pcb[pid].state= READY;
    EnQ(pid, &ready_q);
    *term_if[index].rx_p= '\0';
    //check if has handler
    if(pcb[pid].sigint_handler_p != NULL){
      WrapperISR(pid, pcb[pid].sigint_handler_p);
    }
    return;

  }
  if(inport != '\n' && inport != '\r'){
    outportb(term_if[index].io, inport);
    if(!(QisEmpty(&term_if[index].rx_wait_q))){
     //append
     *term_if[index].rx_p = inport;
     term_if[index].rx_p++;
    }
    return;
  }
  if(!(QisEmpty(&term_if[index].rx_wait_q))){
    //delimit
    *term_if[index].rx_p = '\0';
    pid=DeQ(&term_if[index].rx_wait_q);
    pcb[pid].state= READY;
    EnQ(pid, &ready_q);
  }
}

void WrapperISR(int pid, func_p_t handler_p){
  TF_t tmp;
  unsigned int *handle, *eip;
  eip = &pcb[pid].TF_p->cs;
  handle = &pcb[pid].TF_p->efl;
  tmp = *pcb[pid].TF_p;
  (int)pcb[pid].TF_p -= 8;
  *pcb[pid].TF_p= tmp;
  
  *handle = (unsigned int) handler_p;
  *eip = (unsigned int) tmp.eip;
     
  pcb[pid].TF_p->eip = (unsigned) Wrapper;  

}

void SignalISR(){
   int signalNum;
   func_p_t func;
   signalNum = pcb[cur_pid].TF_p->ebx;
   func =(func_p_t) pcb[cur_pid].TF_p->ecx;
   if(signalNum==SIGCHLD)
     pcb[cur_pid].sigchld_handler_p=func;
   if(signalNum==SIGINT)
     pcb[cur_pid].sigint_handler_p= func; 
}

void GetPpidISR(){
  pcb[cur_pid].TF_p->ebx = pcb[cur_pid].ppid; 
}

void ForkISR(){
  int child;
  int distance;
  int *p; 
  child=DeQ(&avail_q);
  pcb[cur_pid].TF_p->ebx = child;
  if(child == -1){
    cons_printf("Kernel Panic: no more process!");
    return;
  }
  
  pcb[child]=pcb[cur_pid];
  
  pcb[child].state=READY;
  EnQ(child, &ready_q);
  pcb[child].ppid=cur_pid;

  //move stack stuff
  MemCpy((char *)stack[child], (char *)stack[cur_pid], STACK_SIZE);  
  distance=(unsigned)&stack[child]-(unsigned)&stack[cur_pid];
  (int)pcb[child].TF_p += distance;
  (int)pcb[child].TF_p->ebx=0;
  (int)pcb[child].TF_p->esp += distance;
  (int)pcb[child].TF_p->ebp += distance;
  (int)pcb[child].TF_p->esi += distance;
  (int)pcb[child].TF_p->edi += distance;
  
  p=(int *)pcb[child].TF_p->ebp;  
  while(*p!=0){
    *p =  *p + distance;
    p =(int *) *p;
  }
}

void ExitISR(){
  int ppid, ec, *ec_p;
  ec = pcb[cur_pid].TF_p->ebx;
  ppid= pcb[cur_pid].ppid;
  if(!InQ(&wait_q,ppid)){
    pcb[cur_pid].state=ZOMBIE;
    cur_pid=-1;
    if(pcb[ppid].sigchld_handler_p != NULL)
      WrapperISR(ppid, pcb[ppid].sigchld_handler_p);
    return;
  }
  DelQ(&wait_q, ppid);
  EnQ(ppid, &ready_q);
  pcb[ppid].state=READY;
  
  ec_p=(int *)pcb[ppid].TF_p->ebx;
  *ec_p=ec;
  pcb[ppid].TF_p->ecx= cur_pid;

  EnQ(cur_pid, &avail_q);
  pcb[cur_pid].state=AVAIL;
  cur_pid=-1;
}

void WaitISR(){
  int cpid, *ec_p;
  for(cpid=0; cpid<=PROC_MAX; cpid++){
    if(pcb[cpid].state==ZOMBIE && pcb[cpid].ppid==cur_pid) break;
  }
  if(cpid>=PROC_MAX){
    EnQ(cur_pid, &wait_q);
    pcb[cur_pid].state=WAIT;
    cur_pid=-1;
    return;
  }
  
  ec_p=(int *)pcb[cur_pid].TF_p->ebx;
  *ec_p=pcb[cpid].TF_p->ebx;
  pcb[cur_pid].TF_p->ecx=cpid;
  
  EnQ(cpid, &avail_q);
  pcb[cpid].state=AVAIL;
}




