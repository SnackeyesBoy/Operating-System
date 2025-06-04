#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <windows.h> // For setColor and console attributes

#define N 4 // 程序數量
#define MAX_TIME 50 // 甘特圖的最大時間，可能需要根據程序總執行時間調整

// 互斥鎖：保護就緒佇列和共享的程序資訊
pthread_mutex_t queue_mutex;
pthread_mutex_t proc_info_mutex;

// 條件變數：當就緒佇列有程序時，通知 CPU 執行緒
pthread_cond_t process_available;
// 條件變數：當時間推進時，通知抵達模擬執行緒
//pthread_cond_t time_advanced;

// 全域時間計數器，受 proc_info_mutex 保護
int current_time = 0;

// 旗標：指示所有程序是否都已由抵達模擬執行緒提交到就緒佇列
int all_processes_submitted = 0;
// 旗標：指示模擬是否應終止 (用於更優雅地停止 time_keeper)
volatile int simulation_active = 1;

// 設定文字顏色
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 程序資料結構
typedef struct {
    int pid;        // 程序 ID
    char state;     // 狀態（未使用）
    int arrival;    // 抵達時間
    int burst;      // 執行時間
    int priority;   // 優先權（未使用）
    int waiting;    // 等待時間
    int turnaround; // 周轉時間
} process;

// 初始化程序資料
process procInfo[] = { /* 初始化各個程序的工作資訊 */
    {3, '-', 0, 3, 4, 0, 0}, //程序識別碼,程序當下狀態,程序抵達加入Ready Queue的時刻,程序需使用CPU的時間(單位為毫秒),程序的優先權,程序等待時間,程序周轉時間
    {2, '-', 0, 6, 2, 0, 0}, //    pid   ,    state   ,           arrival           ,               brust           ,   priority ,   waitimg  ,  turnaround
    {1, '-',13, 8, 1, 0, 0},
    {4, '-', 2, 2, 3, 0, 0}
};

// 全域變數
int ready_queue[N];
int q_front = 0, q_rear = 0;
int gantt_chart[MAX_TIME]; // 甘特圖紀錄
int g_index = 0;// 甘特圖索引
int sorted_indices[N];

// 印出就緒佇列
void printQueue() {
    printf("[");
    for (int i = q_front; i < q_rear; i++) {
        printf(" P%d ", procInfo[ready_queue[i]].pid);
        if (i < q_rear - 1) printf("|");
    }
    printf("]");
}

// FCFS
void *FCFS_scheduler(void *arg) {
    int finished_processes = 0;

    while (finished_processes < N) {
        pthread_mutex_lock(&queue_mutex);

        // 當就緒佇列為空，且所有程序尚未全部提交，且有未完成程序時，CPU 執行緒等待
        while (q_front == q_rear && !all_processes_submitted && finished_processes < N) {
            pthread_mutex_unlock(&queue_mutex); // 先釋放 queue_mutex，再記錄 idle
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME) {
                gantt_chart[g_index++] = -1; // CPU idle
                current_time++;
            }

            pthread_mutex_unlock(&proc_info_mutex);
            Sleep(1); // 模擬 idle 1 毫秒
            pthread_mutex_lock(&queue_mutex);
        }

        // 如果佇列為空且所有程序都已提交
        if (q_front == q_rear && all_processes_submitted) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        // 確保佇列不為空
        if (q_front == q_rear) {
            pthread_mutex_unlock(&queue_mutex);
            continue;
        }

        int current_proc_idx = ready_queue[q_front++];
        process *p = &procInfo[current_proc_idx];

        printf("時刻 %2d ", current_time);
        setColor(6);
        printf("Process %d 開始執行 (需要 %d 毫秒)\n", p->pid, p->burst);
        setColor(7);

        pthread_mutex_unlock(&queue_mutex); // 執行時釋放鎖

        // 模擬程序執行
        for (int i = 0; i < p->burst; i++) {
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME) {
                gantt_chart[g_index++] = p->pid;
                current_time++;
            }
            pthread_mutex_unlock(&proc_info_mutex);
            Sleep(1);
        }

        pthread_mutex_lock(&proc_info_mutex);
        int finish_time = current_time;
        p->turnaround = finish_time - p->arrival;
        p->waiting = p->turnaround - p->burst;
        finished_processes++;
        pthread_mutex_unlock(&proc_info_mutex);

        printf("時刻 %2d ", finish_time);
        setColor(12);
        printf("Process %d 結束執行\n", p->pid);
        setColor(7);
    }
    pthread_exit(NULL);
}

// 程序抵達模擬執行緒功能
void *process_arrival_simulator(void *arg) {
    // 初始化全域的 sorted_indices 陣列，並根據抵達時間排序
    for (int i = 0; i < N; ++i) {
        sorted_indices[i] = i;
    }

    // 根據抵達時間排序 (如果抵達時間相同，則按 PID 排序)
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

        //pthread_mutex_lock(&proc_info_mutex); // 保護 current_time 並等待 time_advanced 訊號
        while (1) {
            pthread_mutex_lock(&proc_info_mutex);
            int ready = (procInfo[proc_idx].arrival <= current_time);
            pthread_mutex_unlock(&proc_info_mutex);
            if (ready) break;
            Sleep(1);
        }
        pthread_mutex_unlock(&proc_info_mutex);

        pthread_mutex_lock(&queue_mutex);
        ready_queue[q_rear++] = proc_idx;
        printf("時刻 %2d ", procInfo[proc_idx].arrival);
        setColor(10); // 綠色
        printf("Process %d 加入 Ready Queue: ", procInfo[proc_idx].pid);
        setColor(7);
        printQueue();
        printf("\n");
        pthread_cond_signal(&process_available); // 通知 CPU 執行緒有新程序可用
        pthread_mutex_unlock(&queue_mutex);
    }

    // 所有程序都已提交到就緒佇列
    pthread_mutex_lock(&proc_info_mutex);
    all_processes_submitted = 1;
    pthread_mutex_unlock(&proc_info_mutex);
    pthread_cond_signal(&process_available); // 再次發送訊號，以防 CPU 執行緒仍在等待
    pthread_exit(NULL);
}

/*
// 時間推進器執行緒功能
void *time_keeper(void *arg) {
    while (simulation_active) { // 使用全域旗標控制終止
        pthread_mutex_lock(&proc_info_mutex);
        current_time++;
        // 廣播訊號，通知所有等待時間推進的執行緒
        pthread_cond_broadcast(&time_advanced);
        pthread_mutex_unlock(&proc_info_mutex);
        Sleep(1); // 每 1 毫秒推進 1 個時間單位
    }
    pthread_exit(NULL);
}
*/

int main() {
    // 初始化互斥鎖和條件變數
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&proc_info_mutex, NULL);
    pthread_cond_init(&process_available, NULL);
    //pthread_cond_init(&time_advanced, NULL);

    // 初始化甘特圖為 -1 (表示 CPU 空閒)
    for (int i = 0; i < MAX_TIME; i++) {
        gantt_chart[i] = -1;
    }

    printf("********** %d 個程序的 CPU 排程模擬程式 ************\n\n", N);
    printf("========== 執行 FCFS 排程機制 (多執行緒) ==========\n\n");
    setColor(10); printf("綠色：加入 Ready Queue\n");
    setColor(6);  printf("黃色：使用 CPU\n");
    setColor(12); printf("紅色：離開 CPU\n\n");
    setColor(7);  printf("---------------------------------------------------\n\n");
    printf("-------------------底下FCFS排程機制----------------\n\n");

    pthread_t cpu_thread, arrival_thread;

    // 創建執行緒：先啟動時間推進器，確保時間開始流動
    //pthread_create(&time_thread, NULL, time_keeper, NULL);
    pthread_create(&arrival_thread, NULL, process_arrival_simulator, NULL);
    pthread_create(&cpu_thread, NULL, FCFS_scheduler, NULL);

    // 等待 CPU 和抵達模擬執行緒完成
    pthread_join(arrival_thread, NULL);
    pthread_join(cpu_thread, NULL);

    // 當 CPU 和抵達執行緒都完成後，安全地停止時間推進器
    pthread_mutex_lock(&proc_info_mutex);
    simulation_active = 0; // 設定旗標通知 time_keeper 停止
    pthread_mutex_unlock(&proc_info_mutex);
    //pthread_cond_broadcast(&time_advanced); // 廣播一次以確保 time_keeper 被喚醒並檢查旗標
    //pthread_join(time_thread, NULL); // 等待 time_keeper 執行緒退出

    // 複製 procInfo 陣列以便按 PID 排序顯示統計結果，不影響原始計算順序
    process temp_procInfo[N];
    for (int i = 0; i < N; i++) {
        temp_procInfo[i] = procInfo[i];
    }
    // PID 排序
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (temp_procInfo[i].pid > temp_procInfo[j].pid) {
                process temp = temp_procInfo[i];
                temp_procInfo[i] = temp_procInfo[j];
                temp_procInfo[j] = temp;
            }
        }
    }

    // 統計結果
    printf("\n===============   FCFS 統計結果   ================\n");
    float total_wait = 0, total_turnaround = 0;
    printf("PID  |  Waiting Time  |  Turnaround Time\n");
    for (int i = 0; i < N; i++) {
        printf(" P%d  |        %2d      |        %2d\n", temp_procInfo[i].pid, temp_procInfo[i].waiting, temp_procInfo[i].turnaround);
        total_wait += temp_procInfo[i].waiting;
        total_turnaround += temp_procInfo[i].turnaround;
    }

    // 顯示甘特圖
    printf("\n=============== 甘特圖 Gantt Chart ===============\n");
    int start = 0;
    for (int i = 1; i <= g_index; i++) {
        //printf("%d\n",gantt_chart[i - 1]);
        if (i == g_index || gantt_chart[i] != gantt_chart[i - 1]) {
            if (gantt_chart[i - 1] == -1){
                printf("[%2d - %2d] ", start, i);
                setColor(12);
                printf("CPU IDEL\n");
                setColor(7);
            }
            else
                printf("[%2d - %2d] Process %d\n", start, i, gantt_chart[i - 1]);
            start = i;
        }
    }

    printf("\n平均等待時間：%.2f ms\n", total_wait / N);
    printf("平均周轉時間：%.2f ms\n", total_turnaround / N);
    printf("Throughput：%.4f processes/ms\n", (float)N / current_time); // 使用最終的 current_time

    // 銷毀互斥鎖和條件變數
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&proc_info_mutex);
    pthread_cond_destroy(&process_available);
    //pthread_cond_destroy(&time_advanced);

    return 0;
}
