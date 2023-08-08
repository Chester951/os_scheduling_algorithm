#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "scheduler.h"

int get_policy(char *policy)
{
    if (strcmp(policy, "FIFO") == 0) return FIFO;
    else if (strcmp(policy, "SJF") == 0) return SJF;
    else if (strcmp(policy, "PSJF") == 0) return PSJF;
    else if (strcmp(policy, "RR") == 0) return RR;
    else return -1;
}

int cmp_end(const void *a, const void *b)
{
    return ((Process *)a)->end - ((Process*)b)->end;
}

int main(int argc, char *argv[])
{
    // Read the .txt file and get policy
    char tmp[8];
    assert(scanf("%s", tmp) == 1);
    int policy = get_policy(tmp);
    assert(policy != -1);

    // Read 2nd line, get number of process
    int num_Procs;
    scanf("%d", &num_Procs);

    // Read P1's name, starting time, finish time, P2's.....
    Process *procs = (Process*) malloc(num_Procs * sizeof(Process));
    for (int i = 0; i < num_Procs; i++)
    {
        scanf("%s%d%d", procs[i].name, &procs[i].ready_time, &procs[i].exec_time);
        procs[i].pid = -1;
        procs[i].start = -1;
        // printf("%s %d %d\n", procs[i].name, procs[i].ready_time, procs[i].exit_time);
    }

    // Start scheduling
    scheduling(policy, num_Procs, procs);

    // Print the each scheduling result and save.
    qsort(procs, num_Procs, sizeof(Process), cmp_end);
    for (int i =0; i < num_Procs; i++)
    {
        printf("%s %d %d\n", procs[i].name, procs[i].start, procs[i].end);
        fprintf(stderr, "%s %d %d\n", procs[i].name, procs[i].start, procs[i].end);
    }
}