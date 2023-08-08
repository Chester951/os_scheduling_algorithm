#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sched.h>
#include <signal.h>
#include <limits.h>

#include "scheduler.h"

int cmp_FIFO(const void *a, const void *b)
{
    return ((Process *)a)->ready_time - ((Process *)b)->ready_time;
}

/** swap_queue() Implementation **/
static inline int swap_queue(int* queue, int queue_num)
{
	int tmp[queue_num];
	for(int i = 0; i < queue_num; i++)
    {
		tmp[i] = queue[i];
    }

    //  將原始陣列 queue 的第一個元素移動到最後一個位置，要保持其它進程的相對順序不變
    //  這樣，下一個時間片的執行權就會被交換給排在第二位的就緒進程。
	queue[queue_num - 1] = queue[0];
	for(int i = 1; i < queue_num; i++)
    {
		queue[i - 1] = tmp[i];
    }
}

/** decide_proc() Implementation **/
// 根據傳入的排班演算法 policy，以及目前的系統狀態和進程資訊，來選擇下一個要執行的進程。
static inline int decide_proc(int policy, int num_Proc, Process *procs, int last_id, int *rr, int *rr_queue, int rr_num,int curr_time)
{
    // 設定輸入的進程id為當前進城的id。
    int curr_id = last_id;

    // curr_id == -1: 代表現在沒有進程在執行
    // curr_id != -1 && procs[curr_id].pid == -1: 代表現在有進程在執行且當前進程還沒執行完畢
    switch (policy)
    {
    // FIFO: 如果上一次執行的進程不在了，則找到最接近它後面的第一個就緒進程
    case FIFO:
        if (curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1))
        {
            int i = curr_id + 1;         // 從 curr_id 的下一個位置開始搜索
            curr_id = -1;
            for (; i < num_Proc; i++)    // i 變數寫外面這樣的特性可以在當 for 迴圈執行完時，仍可以保留住 i 變數。
            {
                if (procs[i].pid != -1)  // 一旦找到就緒進程，即 procs[i].pid != -1 成立
                {
                    curr_id = i;
                    break;
                }
            }
        }
        break;

    // SJF: 找到整個陣列中執行時間最短的進程來執行。
    case SJF:
        if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)) // 檢查當前查當前選擇的進程 curr_id 是否有效。
        {
            curr_id = -1;
            for(int i = 0; i < num_Proc; i++) // 遍歷所有行程
            {
                if(procs[i].pid == -1)        // 如果該進程已經結束，則跳過，不考慮他最為下一個進程。
                {
                    continue;
                }
                 // 在整個所有行程陣列中尋找最短的 exec_time。
                if(curr_id == -1 || (curr_id != -1 && procs[i].exec_time < procs[curr_id].exec_time))
                {
                    curr_id = i;
                }
            }
        }
        break;

    // PSJF: 找到整個陣列中剩餘執行時間最短的就緒進程。
    case PSJF:
        int shortest_time = INT_MAX;

        curr_id = -1;
        for (int i = 0; i < num_Proc; i++)  // 遍歷所有行程
        {
            if (procs[i].pid == -1)
            {
                continue;
            }
            // 因為在 scheduling() 中，如果行程有執行1個單位時間，則會更新該行程的 procs[i].exec_time 
            // 所以在這裡行程的剩餘執行時間等於 procs[i].exec_time 
            int remaining_time = procs[i].exec_time;

            // 在整個所有行程陣列中尋找最短的 remaining_time。
            if(curr_id == -1 || (curr_id != -1 && remaining_time < shortest_time))
            {
                curr_id = i;
                shortest_time = remaining_time;
            }
        }
        break;

    case RR:
        if (curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1) || *rr == 0)
        {
            if(rr_num <= 0)
            {
                curr_id = -1;
            }
            else
            {
                if (curr_id != -1 && procs[curr_id].pid != -1 && *rr == 0)
                {
                    swap_queue(rr_queue, rr_num);
                }
                curr_id = rr_queue[0];
            }
            *rr = TQ;
        }
        break;
    }
    // printf("Current time: %d, Decide Process ID: %d\n", curr_time, curr_id+1);
    return curr_id;
}

/** scheduling() Implementation **/
int scheduling(int policy, int num_Procs, Process *procs)
{
    /*========== 1. Scheduling Initialization: sort the process according its starting time. ==========*/
    // 排序後，process 陣列中的進程會按照準備就緒時間的順序排列。
    qsort(procs, num_Procs, sizeof(Process), cmp_FIFO);


    /*========== 2. 開始排班（scheduling） ==========*/
    int last_id = -1;        // last_id 代表上一次選擇的進程 ID
    int curr_time = 0;       // curr_time 代表當前時間
    int rr = TQ;             // rr 代表 Round Robin 演算法的時間片
    int rr_queue[num_Procs]; // rr_queue 用於 Round Robin 演算法的進程佇列
    int rr_num = 0;          // rr_num 是該佇列中的進程數量
    int done_count = 0;      // done_count 是已完成排程的進程數量
    // 這部分程式碼會不斷地進入迴圈，根據進程的準備就緒時間選擇進程來執行，直到所有進程都完成排程，整個排程過程才結束。
    while (1)
    {
        /*========== 2.1 遍歷每個進程，檢查是否有進程準備好要執行 ==========*/
        for (int i = last_id + 1; i < num_Procs; i++)
        {
            // 某個進程的準備就緒時間（procs[i].ready_time）大於當前時間（curr_time），則下次再來
            if (procs[i].ready_time > curr_time)
            {
                break;
            }
            // 如果某個進程的準備就緒時間（procs[i].ready_time）等於當前時間（curr_time），則執行該進程
            else if (procs[i].ready_time == curr_time)
            {
                
                procs[i].pid = 1;
                // 如果使用的是 Round Robin 演算法（policy == RR），則將該進程的 ID 加入到 Round Robin 佇列（rr_queue）中。
                if (policy == RR)
                {
                    rr_queue[rr_num] = i;
                    rr_num++;
                }
            }
        }

        /*=========== 2.2 確定下一個要執行的進程 ==========*/
        int curr_id = decide_proc(policy, num_Procs, procs, last_id, &rr, rr_queue, rr_num, curr_time);

        /*=========== 2.3 內容轉換（context switching） ==========*/
        if (curr_id != last_id)
        {
            if (curr_id != -1 && procs[curr_id].pid != -1 && procs[curr_id].start == -1)
            {
                procs[curr_id].start = curr_time;
            }
        }

        /*=========== 2.4 執行目前選擇的進程一個時間單位 ==========*/
        curr_time += 1;
        if (curr_id != -1)
        {
            procs[curr_id].exec_time--;
            rr--;
        }

        /*=========== 2.5 檢查目前選擇的進程是否已經執行完畢 ==========*/
        if (curr_id != -1 && procs[curr_id].exec_time <= 0)
        {
            procs[curr_id].pid = -1;  // 該進程設定為 -1，代表該進程已經執行完畢了
            procs[curr_id].end = curr_time;
            if (policy == RR)
            {
                swap_queue(rr_queue, rr_num);
                rr_num--;
            }
            done_count += 1;

            // 結束整個排班過程
            if (done_count == num_Procs)
            {
                break;
            }
        }

        /*=========== 2.6 更新 last_id 為目前選擇的進程 ID。 ==========*/
        if (curr_id != -1)
        {
            last_id = curr_id;
        }
    }
}