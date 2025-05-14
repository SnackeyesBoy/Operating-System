#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

//設定輸出文字顏色
void setColor(int color){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color);
}
//
void output(int result,int mode){
    if (mode==1){
        if (result==-1)
        {
            setColor(4);
            printf("Status> Calculator open fail!\n\n");
        }else{
            setColor(2);
            printf("(Mode:Frontground)\n"); //前景模式
            setColor(2);
            printf("Status> Calculator open successful!\n\n");
        }
    }
    if (mode==2){
        if (result==-1)
        {
            setColor(4);
            printf("Status> Calculator open fail!\n\n");
        }else{
            setColor(2);
            printf("(Mode:Backgrunod)\n"); //背景模式
            setColor(2);
            printf("Status> Calculator open successful!\n\n");
        }
    }
}

int main() {
	char cmd[100];
	setColor(2 + 16*21);
	char hostname[128],_hostname[128];
    char *token;
    int numbers[100];
    char ampm[3];  // AM 或 PM
    int i = 0;

    time_t rawtime;
    struct tm *timeinfo;
    char time_str[50]; // 用於儲存時間字串
    int str_len;


    FILE *fp;
    fp = popen("hostname", "r");
    fgets(_hostname, sizeof(_hostname), fp);
    int h;
    for(h=0;_hostname[h]!='\0';h++){
        if(_hostname[h] == '\n')
            break;
        hostname[h] = _hostname[h];
    }

	printf("********************************************************\n");
	printf("*                     it's MyGo Shell!!!               *\n");
	printf("*                     Verion:2025.03                   *\n");
	printf("********************************************************\n");
	setColor(7);
	printf("Command Hint: Enter 'hint' to see all commands.\n");
	printf("========================================================\n\n");

	while(1){

        setColor(6);
        //hostname+時間
        time_t now;
        struct tm *localTime;
        time(&now);
        localTime = localtime(&now);
        int hour = localTime->tm_hour;
        if (hour >= 12) {
        strcpy(ampm, "PM");
        if (hour > 12) hour -= 12;  // 轉換為 12 小時制
        } else {
        strcpy(ampm, "AM");
        if (hour == 0) hour = 12;  // 00:00 變成 12:00 AM
        }
		//printf("Snackeyes@Windows-powershell\n");
		//printf("->");

		//Enter block -> Hostname + Time
		printf("%s@%02d:%02d:%02d %s\n", hostname,hour,localTime->tm_min,localTime->tm_sec,ampm);
		printf("-> ");

		setColor(7);
		fgets(cmd,sizeof(cmd),stdin); //Get Enter
		cmd[strcspn(cmd,"\n")] = '\0';
		//token = strtok(cmd, " "); // 使用空格作為分隔符號
        //token = strtok(NULL, " "); // 跳過 "product"


		//scanf("",&cmd);
		printf(">Your Enter:%s\n",cmd);


        setColor(7);
		//Open calc.exe
        if(strcmp(cmd, "calc")==0)
        {
            int mode = 1;
            const char *args[] = {"calc.exe", NULL};
			int result = spawnv(_P_WAIT, "calc.exe", args);
			output(result,mode);
        }
        else if (strcmp(cmd, "calc &")==0)
		{
		    int mode = 2;
		    const char *args[] = {"calc.exe", NULL};
			int result = spawnv(_P_NOWAIT, "calc.exe", args);
            output(result,mode);
        }

        //Open notepad.exe
		else if (strcmp(cmd, "notepad")==0)
		{
		    const char *args[] = {"notepad.exe", NULL};
			int result = spawnv(_P_WAIT, "notepad.exe", args);
            if (result==-1)
            {
                setColor(4);
                printf("Status> Notepad open fail!\n\n");
            }else{
                setColor(7+16*4);
                printf("(Mode:Frontground)\n"); //前景模式
                setColor(2);
                printf("Status> Notepad open successful!\n\n");
            }
		}

		else if (strcmp(cmd, "notepad &")==0)
		{
		    const char *args[] = {"notepad.exe", NULL};
			int result = spawnv(_P_NOWAIT, "notepad.exe", args);
            setColor(2);

            if (result==-1)
            {
                setColor(4);
                printf("Status> Notepad open fail!\n");
                printf("\n");
            }else{
                setColor(7+16*2);
                printf("(Mode:Backgrunod)\n"); //背景模式
                setColor(2);
                printf("Status> Notepad open successful!\n");
                printf("\n");
            }

		}

        //Open help.exe
		else if (strcmp(cmd, "help")==0)
		{
		    setColor(7);
		    const char *args[] = {"help.exe", NULL};
			//int result = spawnv(_P_NOWAIT, "help.exe", args);
            int result = spawnv(_P_WAIT, "C:\\Windows\\System32\\help.exe", args);
            printf("\n");
		}

        //Display hostname
        else if (strcmp(cmd,"hostname")==0)
        {
            printf("The hostname of this computer is ");
            const char *args[] = {"hostname.exe", NULL};
            int result = spawnv(_P_WAIT, "hostname.exe", args);
        }

        //Display time.exe
		else if(strcmp(cmd, "time")==0)
        {

            while(1)
            {
               time_t now;
               struct tm *localTime;
               time(&now);
               localTime = localtime(&now);

               //system("cls");
               setColor(3);
               printf("# Time #\n");
               printf("> Date: %04d-%02d-%02d\n",localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday);
               printf("> Time: %02d:%02d:%02d\n",localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
               printf("\n");

               break;
            }

            //system("cd: C:\Windows\System32\^time");
        }

        //Open product.exe and calculate numbers
        else if (strcmp(cmd,"product")==0)
        {
             // 轉換為數字

            while (token != NULL && i < sizeof(cmd)) {
                numbers[i] = atoi(token);
                token = strtok(NULL, " ");
                //printf("%d,",numbers[i]);
                i++;
            }
            const char *args[] = {"prouct.exe", NULL};
            int result = spawnv(_P_WAIT, "product.exe", args);
            //for(i=1;cmd[i]!=NULL;i++){
			//int num = atoi(cmd[i]);

			//if(num == 0){
				//printf("Wrong Enter, Number Only!");
				//break;
			//}
            //}
        }

        //List hint
        else if (strcasecmp(cmd,"hint")==0)
        {
            printf("====== The following commands can be used in this shell =====\n");
            printf("calc     -> Open Calculator\n");
            printf("notepad  -> Open Notepad.\n");
            printf("hostname -> List the machine names currently used by the user.\n");
            printf("help     -> List instructions for using command line instructions.\n");
            printf("time     -> Display current machine time.\n");
            printf("product  -> Calculate the product of given sets of numbers.\n");
            printf("            Example : product 4 5 6\n");
            printf("bye      -> Quit shell.\n\n");
        }

        //Quit shell
        else if (strcmp(cmd,"bye")==0)
        {
            setColor(2);
            printf("Qit Shell Successful!!\n");
            break;
        }

        //Avoid wrong enter
		else{

            setColor(4);
			printf("****        ERROR COMMAND!!!        ****\n");
			printf("*Please enter command again,           *\n");
			printf("*or enter 'hint' to look for commmands.*\n");
			printf("****************************************\n\n");
		}
	}

    pclose(fp);
    return 0;
}

