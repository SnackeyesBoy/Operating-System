#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <windows.h> // for SetConsoleTextAttribute

#define N 4
#define MAX_TIME 50
#define TIME_QUANTUM 2 // RR 時間片

pthread_mutex_t queue_mutex;
pthread_mutex_t proc_info_mutex;
pthread_cond_t process_available;

int current_time = 0;
int all_processes_submitted = 0;

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 程序資料結構
typedef struct {
    int pid;        // 程序 ID
    char state;     // 狀態（未使用）
    int arrival;    // 抵達時間
    int burst;      // 執行時間
    int priority;   // 優先權
    int waiting;    // 等待時間
    int turnaround; // 周轉時間
    int remaining_time; //剩餘時間
} process;

// 初始化程序資料
process procInfo[] = { /* 初始化各個程序的工作資訊 */
    {3, '-', 0, 3, 4, 0, 0}, //程序識別碼,程序當下狀態,程序抵達加入Ready Queue的時刻,程序需使用CPU的時間(單位為毫秒),程序的優先權,程序等待時間,程序周轉時間
    {2, '-', 0, 6, 2, 0, 0}, //    pid   ,    state   ,           arrival           ,               brust           ,   priority ,   waitimg  ,  turnaround
    {1, '-',13, 8, 1, 0, 0},
    {4, '-', 2, 2, 3, 0, 0}
};

int ready_queue[N * 10];
int q_front = 0, q_rear = 0;
int gantt_chart[MAX_TIME];
int g_index = 0;
int sorted_indices[N];

void printQueue() {
    printf("[");
    for (int i = q_front; i < q_rear; i++) {
        printf(" P%d ", procInfo[ready_queue[i]].pid);
        if (i < q_rear - 1) printf("|");
    }
    printf("]");
}

void *RR_scheduler(void *arg) {
    int finished_processes = 0;

    while (finished_processes < N) {
        pthread_mutex_lock(&queue_mutex);

        if (q_front == q_rear && all_processes_submitted) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        if (q_front == q_rear) {
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME)
                gantt_chart[g_index++] = -1; // 表示 CPU IDLE
            current_time++;
            pthread_mutex_unlock(&proc_info_mutex);
            pthread_mutex_unlock(&queue_mutex);
            Sleep(1);
            continue;
        }

        int current_proc_idx = ready_queue[q_front++];
        process *p = &procInfo[current_proc_idx];

        int exec_time = (p->remaining_time < TIME_QUANTUM) ? p->remaining_time : TIME_QUANTUM;

        pthread_mutex_lock(&proc_info_mutex);
        printf("時刻 %2d ", current_time);
        setColor(6);
        printf("Process %d 執行 %d 毫秒 (剩餘 %2d 毫秒)\n", p->pid, exec_time, p->remaining_time);
        setColor(7);
        pthread_mutex_unlock(&proc_info_mutex);
        pthread_mutex_unlock(&queue_mutex);

        for (int i = 0; i < exec_time; i++) {
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME)
                gantt_chart[g_index++] = p->pid;
            current_time++;
            pthread_mutex_unlock(&proc_info_mutex);
            Sleep(1);
        }

        pthread_mutex_lock(&proc_info_mutex);
        p->remaining_time -= exec_time;
        if (p->remaining_time <= 0) {
            p->turnaround = current_time - p->arrival;
            p->waiting = p->turnaround - p->burst;
            finished_processes++;
            printf("時刻 %2d ", current_time);
            setColor(12);
            printf("Process %d 結束執行\n", p->pid);
            setColor(7);
        } else {
            pthread_mutex_lock(&queue_mutex);
            ready_queue[q_rear++] = current_proc_idx;
            pthread_mutex_unlock(&queue_mutex);
        }
        pthread_mutex_unlock(&proc_info_mutex);
    }

    pthread_exit(NULL);
}

void *process_arrival_simulator(void *arg) {
    for (int i = 0; i < N; ++i) sorted_indices[i] = i;

    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (procInfo[sorted_indices[i]].arrival > procInfo[sorted_indices[j]].arrival ||
                (procInfo[sorted_indices[i]].arrival == procInfo[sorted_indices[j]].arrival &&
                 procInfo[sorted_indices[i]].pid > procInfo[sorted_indices[j]].pid)) {
                int temp = sorted_indices[i];
                sorted_indices[i] = sorted_indices[j];
                sorted_indices[j] = temp;
            }
        }
    }

    for (int i = 0; i < N; i++) {
        int proc_idx = sorted_indices[i];

        while (procInfo[proc_idx].arrival > current_time) {
            Sleep(1);
        }

        pthread_mutex_lock(&queue_mutex);
        ready_queue[q_rear++] = proc_idx;
        pthread_mutex_lock(&proc_info_mutex);
        printf("時刻 %2d ", current_time);
        setColor(10);
        printf("Process %d 加入 Ready Queue: ", procInfo[proc_idx].pid);
        setColor(7);
        printQueue();
        printf("\n");
        pthread_mutex_unlock(&proc_info_mutex);
        pthread_mutex_unlock(&queue_mutex);
    }

    pthread_mutex_lock(&proc_info_mutex);
    all_processes_submitted = 1;
    pthread_mutex_unlock(&proc_info_mutex);
    pthread_exit(NULL);
}

int main() {
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&proc_info_mutex, NULL);
    pthread_cond_init(&process_available, NULL);

    for (int i = 0; i < MAX_TIME; i++) gantt_chart[i] = -1;
    for (int i = 0; i < N; i++) {
        procInfo[i].remaining_time = procInfo[i].burst;
    }
    printf("********** %d 個程序的 CPU 排程模擬程式 ************\n\n", N);
    printf("================== 執行 RR 排程機制  ==============\n\n", TIME_QUANTUM);
    setColor(10); printf("綠色：加入 Ready Queue\n");
    setColor(6);  printf("黃色：使用 CPU\n");
    setColor(12); printf("紅色：離開 CPU\n\n");
    setColor(7);  printf("---------------------------------------------------\n\n");
    printf("-------------------底下RR排程機制------------------\n\n");

    pthread_t cpu_thread, arrival_thread;
    pthread_create(&arrival_thread, NULL, process_arrival_simulator, NULL);
    pthread_create(&cpu_thread, NULL, RR_scheduler, NULL);

    pthread_join(arrival_thread, NULL);
    pthread_join(cpu_thread, NULL);

    process temp_procInfo[N];
    for (int i = 0; i < N; i++) temp_procInfo[i] = procInfo[i];
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (temp_procInfo[i].pid > temp_procInfo[j].pid) {
                process tmp = temp_procInfo[i];
                temp_procInfo[i] = temp_procInfo[j];
                temp_procInfo[j] = tmp;
            }
        }
    }

    printf("\n===============   RR 統計結果   ================\n");
    float total_wait = 0, total_turnaround = 0;
    printf("PID  |  Waiting Time  |  Turnaround Time\n");
    for (int i = 0; i < N; i++) {
        printf(" P%d  |        %2d      |        %2d\n",
               temp_procInfo[i].pid,
               temp_procInfo[i].waiting,
               temp_procInfo[i].turnaround);
        total_wait += temp_procInfo[i].waiting;
        total_turnaround += temp_procInfo[i].turnaround;
    }

    printf("\n=============== 甘特圖 Gantt Chart ===============\n");
    int start = 0;
    for (int i = 1; i <= g_index; i++) {
        if (i == g_index || gantt_chart[i] != gantt_chart[i - 1]) {
            if (gantt_chart[i - 1] == -1){
                printf("[%2d - %2d] ", start, i);
                setColor(12);
                printf("CPU IDLE\n");
                setColor(7);
            }
            else{
                printf("[%2d - %2d] Process %d\n", start, i, gantt_chart[i - 1]);
            }
            start = i;
        }
    }

    printf("\n平均等待時間：%.2f ms\n", total_wait / N);
    printf("平均周轉時間：%.2f ms\n", total_turnaround / N);
    printf("Throughput：%.4f processes/ms\n", (float)N / current_time);

    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&proc_info_mutex);
    pthread_cond_destroy(&process_available);

    return 0;
}
