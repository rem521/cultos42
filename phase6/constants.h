// constants.h, 159

#ifndef __CONSTANTS__
#define __CONSTANTS__

#define VGA_MASK 0x0f00     // foreground bright white, background black
#define TIMER 32            // TIMER constant identifier
#define PIC_MASK 0x21       // Programmable Interrupt Controller I/O
//#define MASK ~0x01          // mask for Programmable Interrupt Controller
#define PIC_CONTROL 0x20    // Programmable Interrupt Controller I/O
#define DONE 0x60

#define LOOP 166666          // slow: 1666666 times calling asm("inb $0x80");
#define TIME_MAX 350         // max timer count, then rotate process
#define PROC_MAX 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define STACK_SIZE 4096      // process stack in bytes
#define BUFF_SIZE 101

#define STDOUT 1
#define READ 3
#define WRITE 4
#define GETPID 20
#define SETVIDEO 100
#define SYSCALL 128
#define SLEEP 162
#define HOME_POS ((unsigned short *)0xb8000)
#define END_POS ((unsigned short *)0xb8000+24*80)

#define SEM_MAX 20
#define SEMINIT 101
#define SEMWAIT 102
#define SEMPOST 103

#define MASK ~0x19
#define TERM_MAX 2
#define TERM0 35
#define TERM1 36
#define TERM0_IO 0x2f8
#define TERM1_IO 0x3e8
#define TERM0_DONE 0x63
#define TERM1_DONE 0x64

#define SIGNAL 48
#define SIGINT 2

#endif
