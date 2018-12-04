#ifndef __SYSCALLS__
#define __SYSCALLS__

void Sleep(int);

int GetPid(void);

void SetVideo(int, int);

void Write(int, char *); 

void Read(int, char *);

int SemInit(int);

void SemWait(int);

void SemPost(int);

void Signal(int , func_p_t);

int GetPpid(void);

int Fork(void);

void Exit(int ec);

int Wait(int *);

#endif
