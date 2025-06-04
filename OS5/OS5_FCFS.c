#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <windows.h> // For setColor and console attributes

#define N 4 // �{�Ǽƶq
#define MAX_TIME 50 // �̯S�Ϫ��̤j�ɶ��A�i��ݭn�ھڵ{���`����ɶ��վ�

// ������G�O�@�N����C�M�@�ɪ��{�Ǹ�T
pthread_mutex_t queue_mutex;
pthread_mutex_t proc_info_mutex;

// �����ܼơG��N����C���{�ǮɡA�q�� CPU �����
pthread_cond_t process_available;
// �����ܼơG��ɶ����i�ɡA�q����F���������
//pthread_cond_t time_advanced;

// ����ɶ��p�ƾ��A�� proc_info_mutex �O�@
int current_time = 0;

// �X�СG���ܩҦ��{�ǬO�_���w�ѩ�F��������������N����C
int all_processes_submitted = 0;
// �X�СG���ܼ����O�_���פ� (�Ω���u���a���� time_keeper)
volatile int simulation_active = 1;

// �]�w��r�C��
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// �{�Ǹ�Ƶ��c
typedef struct {
    int pid;        // �{�� ID
    char state;     // ���A�]���ϥΡ^
    int arrival;    // ��F�ɶ�
    int burst;      // ����ɶ�
    int priority;   // �u���v�]���ϥΡ^
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

// �����ܼ�
int ready_queue[N];
int q_front = 0, q_rear = 0;
int gantt_chart[MAX_TIME]; // �̯S�Ϭ���
int g_index = 0;// �̯S�ϯ���
int sorted_indices[N];

// �L�X�N����C
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

        // ��N����C���šA�B�Ҧ��{�ǩ|����������A�B���������{�ǮɡACPU ���������
        while (q_front == q_rear && !all_processes_submitted && finished_processes < N) {
            pthread_mutex_unlock(&queue_mutex); // ������ queue_mutex�A�A�O�� idle
            pthread_mutex_lock(&proc_info_mutex);
            if (g_index < MAX_TIME) {
                gantt_chart[g_index++] = -1; // CPU idle
                current_time++;
            }

            pthread_mutex_unlock(&proc_info_mutex);
            Sleep(1); // ���� idle 1 �@��
            pthread_mutex_lock(&queue_mutex);
        }

        // �p�G��C���ťB�Ҧ��{�ǳ��w����
        if (q_front == q_rear && all_processes_submitted) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        // �T�O��C������
        if (q_front == q_rear) {
            pthread_mutex_unlock(&queue_mutex);
            continue;
        }

        int current_proc_idx = ready_queue[q_front++];
        process *p = &procInfo[current_proc_idx];

        printf("�ɨ� %2d ", current_time);
        setColor(6);
        printf("Process %d �}�l���� (�ݭn %d �@��)\n", p->pid, p->burst);
        setColor(7);

        pthread_mutex_unlock(&queue_mutex); // �����������

        // �����{�ǰ���
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

        printf("�ɨ� %2d ", finish_time);
        setColor(12);
        printf("Process %d ��������\n", p->pid);
        setColor(7);
    }
    pthread_exit(NULL);
}

// �{�ǩ�F����������\��
void *process_arrival_simulator(void *arg) {
    // ��l�ƥ��쪺 sorted_indices �}�C�A�îھک�F�ɶ��Ƨ�
    for (int i = 0; i < N; ++i) {
        sorted_indices[i] = i;
    }

    // �ھک�F�ɶ��Ƨ� (�p�G��F�ɶ��ۦP�A�h�� PID �Ƨ�)
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

        //pthread_mutex_lock(&proc_info_mutex); // �O�@ current_time �õ��� time_advanced �T��
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
        setColor(10); // ���
        printf("Process %d �[�J Ready Queue: ", procInfo[proc_idx].pid);
        setColor(7);
        printQueue();
        printf("\n");
        pthread_cond_signal(&process_available); // �q�� CPU ��������s�{�ǥi��
        pthread_mutex_unlock(&queue_mutex);
    }

    // �Ҧ��{�ǳ��w�����N����C
    pthread_mutex_lock(&proc_info_mutex);
    all_processes_submitted = 1;
    pthread_mutex_unlock(&proc_info_mutex);
    pthread_cond_signal(&process_available); // �A���o�e�T���A�H�� CPU ��������b����
    pthread_exit(NULL);
}

/*
// �ɶ����i��������\��
void *time_keeper(void *arg) {
    while (simulation_active) { // �ϥΥ���X�б���פ�
        pthread_mutex_lock(&proc_info_mutex);
        current_time++;
        // �s���T���A�q���Ҧ����ݮɶ����i�������
        pthread_cond_broadcast(&time_advanced);
        pthread_mutex_unlock(&proc_info_mutex);
        Sleep(1); // �C 1 �@����i 1 �Ӯɶ����
    }
    pthread_exit(NULL);
}
*/

int main() {
    // ��l�Ƥ�����M�����ܼ�
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&proc_info_mutex, NULL);
    pthread_cond_init(&process_available, NULL);
    //pthread_cond_init(&time_advanced, NULL);

    // ��l�ƥ̯S�Ϭ� -1 (��� CPU �Ŷ�)
    for (int i = 0; i < MAX_TIME; i++) {
        gantt_chart[i] = -1;
    }

    printf("********** %d �ӵ{�Ǫ� CPU �Ƶ{�����{�� ************\n\n", N);
    printf("========== ���� FCFS �Ƶ{���� (�h�����) ==========\n\n");
    setColor(10); printf("���G�[�J Ready Queue\n");
    setColor(6);  printf("����G�ϥ� CPU\n");
    setColor(12); printf("����G���} CPU\n\n");
    setColor(7);  printf("---------------------------------------------------\n\n");
    printf("-------------------���UFCFS�Ƶ{����----------------\n\n");

    pthread_t cpu_thread, arrival_thread;

    // �Ыذ�����G���Ұʮɶ����i���A�T�O�ɶ��}�l�y��
    //pthread_create(&time_thread, NULL, time_keeper, NULL);
    pthread_create(&arrival_thread, NULL, process_arrival_simulator, NULL);
    pthread_create(&cpu_thread, NULL, FCFS_scheduler, NULL);

    // ���� CPU �M��F�������������
    pthread_join(arrival_thread, NULL);
    pthread_join(cpu_thread, NULL);

    // �� CPU �M��F�������������A�w���a����ɶ����i��
    pthread_mutex_lock(&proc_info_mutex);
    simulation_active = 0; // �]�w�X�гq�� time_keeper ����
    pthread_mutex_unlock(&proc_info_mutex);
    //pthread_cond_broadcast(&time_advanced); // �s���@���H�T�O time_keeper �Q������ˬd�X��
    //pthread_join(time_thread, NULL); // ���� time_keeper ������h�X

    // �ƻs procInfo �}�C�H�K�� PID �Ƨ���ܲέp���G�A���v�T��l�p�ⶶ��
    process temp_procInfo[N];
    for (int i = 0; i < N; i++) {
        temp_procInfo[i] = procInfo[i];
    }
    // PID �Ƨ�
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (temp_procInfo[i].pid > temp_procInfo[j].pid) {
                process temp = temp_procInfo[i];
                temp_procInfo[i] = temp_procInfo[j];
                temp_procInfo[j] = temp;
            }
        }
    }

    // �έp���G
    printf("\n===============   FCFS �έp���G   ================\n");
    float total_wait = 0, total_turnaround = 0;
    printf("PID  |  Waiting Time  |  Turnaround Time\n");
    for (int i = 0; i < N; i++) {
        printf(" P%d  |        %2d      |        %2d\n", temp_procInfo[i].pid, temp_procInfo[i].waiting, temp_procInfo[i].turnaround);
        total_wait += temp_procInfo[i].waiting;
        total_turnaround += temp_procInfo[i].turnaround;
    }

    // ��ܥ̯S��
    printf("\n=============== �̯S�� Gantt Chart ===============\n");
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

    printf("\n�������ݮɶ��G%.2f ms\n", total_wait / N);
    printf("�����P��ɶ��G%.2f ms\n", total_turnaround / N);
    printf("Throughput�G%.4f processes/ms\n", (float)N / current_time); // �ϥγ̲ת� current_time

    // �P��������M�����ܼ�
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&proc_info_mutex);
    pthread_cond_destroy(&process_available);
    //pthread_cond_destroy(&time_advanced);

    return 0;
}
