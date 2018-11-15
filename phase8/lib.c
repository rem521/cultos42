// lib.c, 159

#include "include.h"
#include "types.h"
#include "data.h"

// clear DRAM data block, zero-fill it
void Bzero(char *p, int bytes) {
	int i;
	for(i=0; i<bytes; i++)
	    *p++ = 0;
}

int QisEmpty(q_t *p) { // return 1 if empty, else 0
	if(p->size == 0) return 1;
	else return 0;
}

int QisFull(q_t *p) { // return 1 if full, else 0
	if(p->size == Q_SIZE) return 1;
	else return 0;
}

int InQ(q_t *p, int id){
  int i= p->head;
  while(1){
    i=i%Q_SIZE;
    if(p->q[i]==id) return 1;
    if(i==p->tail) return 0;
    i++;
  }
}
// dequeue, 1st integer in queue
// if queue empty, return -1
int DeQ(q_t *p) { // return -1 if q[] is empty
	int pid;
	

	if(QisEmpty(p)) {
		return -1;
	}

	else {
		pid = p->q[p->head];
		p->size--;
		p->head++;
		p->head = p->head % Q_SIZE;
		return pid;
	}
}

// if not full, enqueue integer to tail slot in queue
void EnQ(int pid, q_t *p) {
	
	if(QisFull(p)) {
		cons_printf("Kernel panic: queue is full, cannot EnQ!\n");
		breakpoint();
    return;
	}

	else {
		p->size++;
    p->q[p->tail] = pid;
		p->tail++;
		p->tail = p->tail % Q_SIZE;
		//p->q[p->tail] = pid;
	}
}

void DelQ(q_t *p, int id){
  int i;
  int tmp;
  for(i=0; i<=p->size; i++){
    tmp=DeQ(p);
    if(tmp==id) continue;
    EnQ(tmp,p);
  }
}

int StrCmp(char *s1, char *s2){
  int i=0;
  while(1){
    if(s1[i]!=s2[i]) return 0;
    if(s1[i]=='\0' && s2[i]=='\0') return 1;
    i++;
  }
}

void MemCpy(char *dst, char *src, int size){
  int i;
  for(i=0; i<size; i++){
    *dst++ = *src++;
  }
}




