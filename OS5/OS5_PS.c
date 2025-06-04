#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <windows.h>

#define N 4 // �{�Ǽƶq
#define MAX_TIME 50 // �̯S�ϳ̤j�ɶ�

pthread_mutex_t queue_mutex;
pthread_mutex_t proc_info_mutex;

pthread_cond_t process_available;
pthread_cond_t time_advanced;

//�����ܼ�
int current_time = 0;
int all_processes_submitted = 0;
volatile int simulation_active = 1;

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// �{�Ǹ�Ƶ��c
typedef struct {
    int pid;        // �{�� ID
    char state;     // ���A�]���ϥΡ^
    int arrival;    // ��F�ɶ�
    int burst;      // ����ɶ�
    int priority;   // �u���v
    int waiting;    // ���ݮɶ�
    int turnaround; // �P��ɶ�
} process;

// ��l�Ƶ{�Ǹ��
process procInfo[] = { /* ��l�ƦU�ӵ{�Ǫ��u�@��T */
    {3, '-', 0, 3, 4, 0, 0}, //�{���ѧO�X,�{�Ƿ�U���A,�{�ǩ�F�[�JReady Queue���ɨ�,�{�ǻݨϥ�CPU���ɶ�(��쬰�@��),�{�Ǫ��u���v,�{�ǵ��ݮɶ�,�{�ǩP��ɶ�
    {2, '-', 0, 6, 2, 0, 0}, //    pid   ,    state   ,           arrival           ,               brust           ,   priority ,   waitimg  ,  turnaround
    {1, '-',13, 8, 1, 0, 0},
    {4, '-', 2, 2, 3, 0, 0}
};

int ready_queue[N];
int q_front = 0, q_rear = 0;
int gantt_chart[MAX_TIME]; //�̯S��
int g_index = 0; //�̯S�ϯ���
int sorted_indices[N]; //�Ƨ�->�Y��F�ɶ��ۦP�h�̷� PID

//�L�XReadyQueue
void printQueue() {
    printf("[");
    for (int i = q_front; i < q_rear; i++) {
        printf(" P%d ", procInfo[ready_queue[i]].pid);
        if (i < q_rear - 1) printf("|");
    }
    printf("]");
}

// ��N����C���u���v�̰��{��(�u���v�ȳ̤p)��m
int find_highest_priority_process() {
    int highest_idx = q_front;
    for (int i = q_front + 1; i < q_rear; i++) {
        int curr_idx = ready_queue[i];
        int highest_pid_idx = ready_queue[highest_idx];
        if (procInfo[curr_idx].priority < procInfo[highest_pid_idx].priority) {
            highest_idx = i;
        }
    }
    return highest_idx;
}

//PS�Ƶ{
void *PS_scheduler(void *arg) {
    int finished_processes = 0;

    while (finished_processes < N) {
        pthread_mutex_lock(&queue_mutex);

        // �Ŧ�C�B�|�����槹���� CPU ����
        while (q_front == q_rear && !all_processes_submitted && finished_processes < N) {
            pthread_mutex_unlock(&queue_mutex);
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME) {
                gantt_chart[g_index++] = -1; // CPU idle ���m
                current_time++;
            }
            pthread_mutex_unlock(&proc_info_mutex);
            Sleep(1);
            pthread_mutex_lock(&queue_mutex);
        }

        if (q_front == q_rear && all_processes_submitted) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        //Queue����
        if (q_front == q_rear) {
            pthread_mutex_unlock(&queue_mutex);
            continue;
        }

        int highest_pos = find_highest_priority_process();
        int current_proc_idx = ready_queue[highest_pos];

        // �����ӵ{��
        for (int i = highest_pos; i < q_rear - 1; i++) {
            ready_queue[i] = ready_queue[i + 1];
        }
        q_rear--;

        process *p = &procInfo[current_proc_idx];// ���o���V�� process ������

        printf("�ɨ� %2d ", current_time);
        setColor(6);
        printf("Process %d �}�l���� (�ݭn %d �@��, �u���v %d)\n", p->pid, p->burst, p->priority);
        setColor(7);

        pthread_mutex_unlock(&queue_mutex);

        for (int i = 0; i < p->burst; i++) {
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME){
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

        printf("�ɨ� %2d ", finish_time);
        setColor(12);
        printf("Process %d ��������\n", p->pid);
        setColor(7);
    }

    pthread_exit(NULL);
}

void *process_arrival_simulator(void *arg) {
    for (int i = 0; i < N; i++) {
        sorted_indices[i] = i;
    }

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
        printf("�ɨ� %2d ", procInfo[proc_idx].arrival);
        setColor(10);
        printf("Process %d �[�J Ready Queue: ", procInfo[proc_idx].pid);
        setColor(7);
        printQueue();
        printf("\n");
        pthread_cond_signal(&process_available);
        pthread_mutex_unlock(&queue_mutex);
    }

    pthread_mutex_lock(&proc_info_mutex);
    all_processes_submitted = 1;
    pthread_mutex_unlock(&proc_info_mutex);
    pthread_cond_signal(&process_available);
    pthread_exit(NULL);
}
/*
void *time_keeper(void *arg) {
    while (simulation_active) {
        pthread_mutex_lock(&proc_info_mutex);
        current_time++;
        pthread_cond_broadcast(&time_advanced);
        pthread_mutex_unlock(&proc_info_mutex);
        Sleep(1);
    }
    pthread_exit(NULL);
}
*/

int main() {
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&proc_info_mutex, NULL);
    pthread_cond_init(&process_available, NULL);

    for (int i = 0; i < MAX_TIME; i++) gantt_chart[i] = -1;

    printf("********** %d �ӵ{�Ǫ� CPU �Ƶ{�����{�� ************\n\n", N);
    printf("========== ���� PS �Ƶ{���� (�h�����) ==========\n\n");
    setColor(10); printf("���G�[�J Ready Queue\n");
    setColor(6);  printf("����G�ϥ� CPU\n");
    setColor(12); printf("����G���} CPU\n\n");
    setColor(7);  printf("-------------------------------------------------\n\n");
    printf("-------------------���UPS�Ƶ{����----------------\n\n");

    pthread_t cpu_thread, arrival_thread;
    pthread_create(&arrival_thread, NULL, process_arrival_simulator, NULL);
    pthread_create(&cpu_thread, NULL, PS_scheduler, NULL);

    pthread_join(arrival_thread, NULL);
    pthread_join(cpu_thread, NULL);

    // �Ƨ���ܲέp���G
    process temp_procInfo[N];
    for (int i = 0; i < N; i++) temp_procInfo[i] = procInfo[i];
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (temp_procInfo[i].pid > temp_procInfo[j].pid) {
                process temp = temp_procInfo[i];
                temp_procInfo[i] = temp_procInfo[j];
                temp_procInfo[j] = temp;
            }
        }
    }
    printf("\n############## �}�l�ɨ� %2d , �����ɨ�%2d ###########\n\n",0,current_time);
    printf("\n================   PS �έp���G   =================\n");
    float total_wait = 0, total_turnaround = 0;
    printf("PID  |  Waiting Time  |  Turnaround Time\n");
    for (int i = 0; i < N; i++) {
        printf(" P%d  |        %2d      |        %2d\n", temp_procInfo[i].pid, temp_procInfo[i].waiting, temp_procInfo[i].turnaround);
        total_wait += temp_procInfo[i].waiting;
        total_turnaround += temp_procInfo[i].turnaround;
    }

    printf("\n=============== �̯S�� Gantt Chart ===============\n");
    int start = 0;
    for (int i = 1; i <= g_index; i++) {
        if (i == g_index || gantt_chart[i] != gantt_chart[i - 1]) {
            if (gantt_chart[i - 1] == -1){
                printf("[%2d - %2d] ", start, i);
                setColor(12);
                printf("CPU IDLE\n");
                setColor(7);
            }
            else
                printf("[%2d - %2d] Process %d\n", start, i, gantt_chart[i - 1]);
            start = i;
        }
    }

    printf("\n�������ݮɶ��G%.2f ms\n", total_wait / N);
    printf("�����P��ɶ��G%.2f ms\n", total_turnaround / N);
    printf("Throughput�G%.4f processes/ms\n", (float)N / current_time);

    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&proc_info_mutex);
    pthread_cond_destroy(&process_available);
    //pthread_cond_destroy(&time_advanced);

    return 0;
}
