#include <sys/types.h>

#define FIFO 0
#define SJF  1
#define PSJF 2
#define RR   4

#define P_CPU 0
#define C_CPU 1

#define TQ 500


typedef struct Process
{
    char name[32];
    int ready_time;
    int exec_time;
    pid_t pid;
    int start;
    int end;
} Process;



/* Function Declaration */
int scheduling(int policy, int num_Procs, Process *process);
