//B11036014 四電機四B 林裕翔
#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// 設定文字顏色
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 執行 calc.exe
void execute_calc(int wait) {
    char *cmd[] = {"calc", NULL};
    spawnv(wait ? _P_WAIT : _P_NOWAIT, "calc.exe", cmd);
    setColor(9);
    printf("(Mode:%s)\n", wait ? "Foreground" : "Background");//分前後景
    setColor(2);
    printf("Status> Calculator open successful!\n\n");
    setColor(7);
}

// 執行 notepad.exe
void execute_notepad(int wait, const char *file) {
    char *cmd[] = {"notepad", (char *)file, NULL};
    spawnv(wait ? _P_WAIT : _P_NOWAIT, "notepad.exe", cmd);
    setColor(9);
    printf("(Mode:%s)\n", wait ? "Foreground" : "Background");//分前後景
    setColor(2);
    printf("Status> Notepad open successful!\n\n");
    setColor(7);
}

// 取得主機名稱
void get_hostname(char *hostname, int size) {
    FILE *fp = popen("hostname", "r");
    if (fp) {
        fgets(hostname, size, fp);
        hostname[strcspn(hostname, "\n")] = 0; // 移除換行
        pclose(fp);
    } else {
        setColor(3);
        strcpy(hostname, "UNKNOWN");
    }
}

int main() {
    char cmd[100];//指令輸入
    char hostname[128];//主機名稱
    char ampm[3];//上午下午

    setColor(2 + 16 * 21);
    printf("********************************************************\n");
    printf("*                 It's Snackeyes Shell!!!              *\n");
    printf("*                     Version: 2025.03                 *\n");
    printf("********************************************************\n");
    setColor(7);
    printf("Command Hint: Enter 'hint' to see all commands.\n");
    printf("========================================================\n\n");

    get_hostname(hostname, sizeof(hostname));//呼叫hostname

    while (1) {
        setColor(6);
        //顯示hostname + time
        time_t now = time(NULL);
        struct tm *localTime = localtime(&now);
        int hour = localTime->tm_hour;
        strcpy(ampm, (hour >= 12) ? "PM" : "AM");
        if (hour == 0) hour = 12;
        else if (hour > 12) hour -= 12;

        printf("%s@%02d:%02d:%02d %s\n-> ", hostname, hour, localTime->tm_min, localTime->tm_sec, ampm);
        setColor(7);

        fgets(cmd, sizeof(cmd), stdin);//輸入指令
        cmd[strcspn(cmd, "\n")] = 0; // 移除換行符

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
            while (*file == ' ') file++; // 移除前導空白
            int wait = 1;

            if (*file) {
                char *ampersand = strchr(file, '&');
                if (ampersand) {
                    *ampersand = '\0'; // 移除 `&`
                    wait = 0;
                }
            } else {
                file = "";
            }
            execute_notepad(wait, file);
        }

        //product.exe
        else if (strncmp(cmd, "product", 7) == 0) {
            //切分字串
            char *args[10] = {"product.exe"};
            int i = 1, valid = 1;
            char *token = strtok(cmd + 7, " ");
            while (token && i < 9) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            // 檢查參數是否為數字
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

        //hint 提示
        else if (strcmp(cmd, "hint") == 0) {
            printf("====== The following commands can be used in this shell =====\n");
            printf("calc     -> Open Calculator\n");
            printf("notepad  -> Open Notepad.\n");
            printf("hostname -> List the machine name.\n");
            printf("help     -> Show command line instructions.\n");
            printf("time     -> Display current time.\n");
            printf("product  -> Calculate the product of given numbers.\n            Example : product 4 5 6 -> product.exe 讀入3個數，乘積=120\n");
            printf("bye      -> Quit shell.\n");
            setColor(9);
            printf("# Any bugs contact me, thanks :)\n");
            printf("# Email: smax920410@gmail.com\n\n");
        }

        //退出shell
        else if (strcmp(cmd, "bye") == 0) {
            setColor(7+2*16);
            printf("Quit Shell Successful!!\n");
            setColor(7);
            break;
        }

        // 錯誤命令
        else {
            setColor(4);
            printf("::::: ERROR COMMAND!!! :::::\n");
            printf("Please enter command again, or type 'hint' for help.\n\n");
            setColor(7);
        }
    }

    return 0;
}
