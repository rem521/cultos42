// proc.h, 159

// prototype those in proc.c here

#ifndef __PROC__
#define __PROC__

void InitProc();

void UserProc();

void CarProc();

void TermProc();

void Ouch();

void Wrapper(func_p_t);

void ChildCode(void);

void ChldHandler();

#endif

