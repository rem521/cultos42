// types.h, 159

#ifndef __TYPES__
#define __TYPES__

#include "constants.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {AVAIL, READY, RUN, SLEEPY, WAIT} state_t;

typedef struct{
   unsigned int edi, esi, ebp, esp,
                ebx, edx, ecx, eax,
                entry,
                eip, cs, efl;
} TF_t;



typedef struct {
	TF_t *TF_p;
	int time;
	int life;
  int wake_time;
	state_t state;
} pcb_t;                     

typedef struct {
	int q[Q_SIZE];
	int head;
	int tail;
	int size; 
} q_t;

typedef struct{
	int passes;
	q_t wait_q;
}sem_t;


#endif
