// lib.h, 159

// prototype those in lib.c here

#ifndef __LIB__
#define __LIB__

void Bzero( char *p, int bytes );

int QisEmpty( q_t *p );

int QisFull( q_t *p );

int InQ( q_t *p, int);

void DelQ(q_t *, int);

int DeQ( q_t *p );

void EnQ( int to_add, q_t *p );

int StrCmp(char *, char *);

void MemCpy( char *, char *, int);

void ReclaimPages(int);

int Alloc(int, int, int []);


#endif

