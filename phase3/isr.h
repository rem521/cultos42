// isr.h, 159

#ifndef __ISR__
#define __ISR__

// prototype those in isr.c here

void NewProcISR( func_p_t p);

void TimerISR(void);

void GetPidISR(void);

void SleepISR(void);

void SetVideoISR(void);

void WriteISR(void);


#endif
