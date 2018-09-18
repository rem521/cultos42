// types.h, 159

#ifndef __TYPES__
#define __TYPES__

#include "constants.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {AVAIL, READY, RUN} state_t;

typedef struct {
   unsigned int reg[8];
   unsigned int eip;
   unsigned int cs;
   unsigned int efl;
} TF_t;

typedef struct {
	TF_t *TF_p;
	int time;
	int life;
	state_t state;
} pcb_t;                     

typedef struct {
	int q[Q_SIZE];
	int head;
	int tail;
	int size; 
} q_t;

#endif
