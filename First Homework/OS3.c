//B11036014 �|�q���|B �L�ε�
#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// �]�w��r�C��
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// ���� calc.exe
void execute_calc(int wait) {
    char *cmd[] = {"calc", NULL};
    spawnv(wait ? _P_WAIT : _P_NOWAIT, "calc.exe", cmd);
    setColor(9);
    printf("(Mode:%s)\n", wait ? "Foreground" : "Background");//���e�ᴺ
    setColor(2);
    printf("Status> Calculator open successful!\n\n");
    setColor(7);
}

// ���� notepad.exe
void execute_notepad(int wait, const char *file) {
    char *cmd[] = {"notepad", (char *)file, NULL};
    spawnv(wait ? _P_WAIT : _P_NOWAIT, "notepad.exe", cmd);
    setColor(9);
    printf("(Mode:%s)\n", wait ? "Foreground" : "Background");//���e�ᴺ
    setColor(2);
    printf("Status> Notepad open successful!\n\n");
    setColor(7);
}

// ���o�D���W��
void get_hostname(char *hostname, int size) {
    FILE *fp = popen("hostname", "r");
    if (fp) {
        fgets(hostname, size, fp);
        hostname[strcspn(hostname, "\n")] = 0; // ��������
        pclose(fp);
    } else {
        setColor(3);
        strcpy(hostname, "UNKNOWN");
    }
}

int main() {
    char cmd[100];//���O��J
    char hostname[128];//�D���W��
    char ampm[3];//�W�ȤU��

    setColor(2 + 16 * 21);
    printf("********************************************************\n");
    printf("*                 It's Snackeyes Shell!!!              *\n");
    printf("*                     Version: 2025.03                 *\n");
    printf("********************************************************\n");
    setColor(7);
    printf("Command Hint: Enter 'hint' to see all commands.\n");
    printf("========================================================\n\n");

    get_hostname(hostname, sizeof(hostname));//�I�shostname

    while (1) {
        setColor(6);
        //���hostname + time
        time_t now = time(NULL);
        struct tm *localTime = localtime(&now);
        int hour = localTime->tm_hour;
        strcpy(ampm, (hour >= 12) ? "PM" : "AM");
        if (hour == 0) hour = 12;
        else if (hour > 12) hour -= 12;

        printf("%s@%02d:%02d:%02d %s\n-> ", hostname, hour, localTime->tm_min, localTime->tm_sec, ampm);
        setColor(7);

        fgets(cmd, sizeof(cmd), stdin);//��J���O
        cmd[strcspn(cmd, "\n")] = 0; // ���������

        //calc.exe
        if (strcmp(cmd, "calc") == 0)
        {
            execute_calc(1);
        }
        else if (strcmp(cmd, "calc &") == 0)
        {
            execute_calc(0);
        }
        //notepad.exe
        else if (strncmp(cmd, "notepad", 7) == 0) {
            char *file = cmd + 7;
            while (*file == ' ') file++; // �����e�ɪť�
            int wait = 1;

            if (*file) {
                char *ampersand = strchr(file, '&');
                if (ampersand) {
                    *ampersand = '\0'; // ���� `&`
                    wait = 0;
                }
            } else {
                file = "";
            }
            execute_notepad(wait, file);
        }

        //product.exe
        else if (strncmp(cmd, "product", 7) == 0) {
            //�����r��
            char *args[10] = {"product.exe"};
            int i = 1, valid = 1;
            char *token = strtok(cmd + 7, " ");
            while (token && i < 9) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            // �ˬd�ѼƬO�_���Ʀr
            for(i=1;args[i]!=NULL;i++){
                int num = atoi(args[i]);
                //printf("num = %d\n",num);
                if(num == 0){
                    setColor(4);
                    printf(":::: ERROR INPUT ::::\nExample: product 4 5 -6\n\n");
                    setColor(7);
                    break;
                }
            }
            if(args[i]==NULL)
            {
                spawnv(_P_WAIT, "product.exe", args);
            }
        }

        //help.exe
        else if (strcmp(cmd, "help") == 0) {
            spawnv(_P_WAIT, "C:\\Windows\\System32\\help.exe", (char *[]){"help.exe", NULL});
            printf("\n");
        }

        //hostname.exe
        else if (strcmp(cmd, "hostname") == 0) {
            printf("The hostname of this computer is: ");
            setColor(9);
            printf("%s\n\n",hostname);
        }

        //time
        else if (strcmp(cmd, "time") == 0) {
            time_t now = time(NULL);
            struct tm *localTime = localtime(&now);
            setColor(3);
            printf("# Time #\n");
            printf("> Date: %04d-%02d-%02d\n", localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday);
            printf("> Time: %02d:%02d:%02d\n\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
            setColor(7);
        }

        //hint ����
        else if (strcmp(cmd, "hint") == 0) {
            printf("====== The following commands can be used in this shell =====\n");
            printf("calc     -> Open Calculator\n");
            printf("notepad  -> Open Notepad.\n");
            printf("hostname -> List the machine name.\n");
            printf("help     -> Show command line instructions.\n");
            printf("time     -> Display current time.\n");
            printf("product  -> Calculate the product of given numbers.\n            Example : product 4 5 6 -> product.exe Ū�J3�ӼơA���n=120\n");
            printf("bye      -> Quit shell.\n");
            setColor(9);
            printf("# Any bugs contact me, thanks :)\n");
            printf("# Email: smax920410@gmail.com\n\n");
        }

        //�h�Xshell
        else if (strcmp(cmd, "bye") == 0) {
            setColor(7+2*16);
            printf("Quit Shell Successful!!\n");
            setColor(7);
            break;
        }

        // ���~�R�O
        else {
            setColor(4);
            printf("::::: ERROR COMMAND!!! :::::\n");
            printf("Please enter command again, or type 'hint' for help.\n\n");
            setColor(7);
        }
    }

    return 0;
}
