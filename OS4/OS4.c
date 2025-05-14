//B11036014 四電機四B 林裕翔
/*
Program: MT-find-minimum-no-mutex.c (Report comments/bugs to chikh@yuntech.edu.tw)
Function: 擴充find-minimum.c、以多執行緒實作「模擬退火法」程序，令每一個執行緒各自找尋最佳解，迨所有執行緒結束後，再由
		主程式判斷哪一個執行緒的結果最佳並作為最後解答。
Note: 1) 請同學先把find-minimum.c的架構與運作邏輯掌握好，研讀本程式將易於進入狀況
	  2) 可擴充本程式、善用多執行緒的特性，找尋函數的區域最小值(可能存在多個)
	  3) 本程式未牽涉到多個執行緒之間的「同步」(synchronization)機制，待我們進度到第六單元，同學可再審視此程式碼並導入
		 互斥鎖(mutual exclusion lock)讓程式更完備
*/

#include <stdio.h>
#include <stdlib.h>		/* for RAND_MAX */
#include <time.h>			/* for srand(), rand() */
#include <math.h>			/* for log() */
#include <pthread.h>		/* 實作多執行緒所需Pthread相關資料型態與函數*/
#include <unistd.h>		/* for sleep() */
#include <windows.h>      //setColor()

//運行參數設定
#define NumThreads	3	/* 將創建的執行緒個數 */
#define InitialTemp	100	/* 起始溫度 */
#define alpha 		0.95	/* 降溫率 */
#define N 			100 /* 最多迭代次數 */

//存最佳路徑，共 9 點，包含回到起點
int localMinRoute[NumThreads][9];

//所有城市之間距離
double distance[8][8] = {
{0,91.8,105.2,89.9,189.9,76.2,278.3,54.4},
{91.8,0,187.2,38.9,271.3,162.9,363.3,88.4},
{105.2,187.2,0,194.1,182.3,31.4,176.1,153.8},
{89.9,38.9,194.1,0,249.4,166.1,368.3,63.6},
{189.9,271.3,182.3,249.4,0,168.0,243.0,185.9},
{76.2,162.9,31.4,166.1,168.0,0,202.2,122.8},
{278.3,363.3,176.1,368.3,243.0,202.2,0,320.0},
{54.4,88.4,153.8,63.6,185.9,122.8,320.0,0},
};

//從[min,max]區間內產生一個隨機亂數
double numRandom(double min, double max)
{
	return (double)rand()/RAND_MAX*(max-min)+min;
}

//讀入走訪路徑，計算所有走訪總距離成本最後回傳
double sumDistance(int route[9]){
    double sum = 0;
    for (int i=0 ; i<8 ;i++)
    {
        sum += distance[route[i]][route[i+1]];
    }
    return sum;
}

//副程式宣告
void printRouteA(int i);// 印該執行緒的最佳路徑
void printRouteB(int route[9]);// 印指定路徑
void filePrintRoute(FILE* fp, int route[9]); // 輸出到檔案
void setColor(int color);

// 印該執行緒的最佳路徑
void printRouteA(int i)
{
    int j;
    for(j = 0;j < 9;j++){
        printf("%d ",localMinRoute[i][j]+1);
        if (j!=8){
            printf("-> ");
        }
    }
    return;
}

// 印指定路徑
void printRouteB(int route[9])
{
    int j;
    for(j = 0;j < 9;j++){
        printf("%d ",route[j]+1);
        if (j!=8){
            printf("-> ");
        }
    }
    return;
}

// 輸出到檔案
void filePrintRoute(FILE* fp, int route[9])
{
    int j;
    for(j = 0;j < 9;j++){
        fprintf(fp,"%d ",route[j]+1);
        if (j!=8){
            fprintf(fp,"-> ");
        }
    }
    //fseek(fp,-2,SEEK_END); //跳到檔案末端，通常用來取得檔案大小
    //fprintf(fp,"  ");
    return;
}

// 設定文字顏色
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

//找最小值函數，將被執行緒執行->演算法所在
void *findExtreme(void *threadId)
{

    int route[9] = {0,1,2,3,4,5,6,7,0}; //存目前正在評估的路徑->當前模擬退火的「現有路徑」
    int new_route[9] = {0,1,2,3,4,5,6,7,0}; //存新隨機產生的變動後路徑->用來試探交換兩城市後，計算產生的新路徑

	int 	i, id = *(int*)threadId; //id為執行緒
  	double 	T = InitialTemp, delta, x, xNew;

  	printf("執行緒 %d 開始工作...\n",id);
  	fflush(stdout);//立即印出

	srand(time(NULL)+id*1000);	/* 讀取機器時間作為亂數產生器的初始化種子，導入id作為種子的一部分，藉此避免所有執行緒使用到相同的初始值 */

    //建立輸出log檔
    char fname[64]; //檔案名稱位元大小
    sprintf(fname,"Output_Log_Thread_%d.txt",id); //檔案名稱
    FILE* fp = fopen(fname,"w"); //開啟檔案，寫入
    fprintf(fp,"Output_Log_Thread_%d 計算結果: ",id);
    fprintf(fp,"\n========================================= 運行資訊 =================================\n\n");
    fprintf(fp,"::運行執行續編號 ： %d\n",id);
  	fprintf(fp,"::執行版本      ： 多執行緒版本\n");
  	fprintf(fp,"::執行緒數量    ： %d\n",NumThreads);
  	fprintf(fp,"::起始溫度     ： %d\n",InitialTemp);
  	fprintf(fp,"::降溫率       ： %.2f\n",alpha);
  	fprintf(fp,"::最多迭代次數  ： %d\n",N);
    fprintf(fp,"\n========================================= 當前成本 =================================\n\n");
    fprintf(fp,"目前預設最小成本路徑: ");
    filePrintRoute(fp,route);
    fprintf(fp,", 總花費成本: %.1f",sumDistance(route));
    fprintf(fp,"\n\n========================================= 運行過程 =================================\n\n");
    fprintf(fp,"運算開始...\n\n");
    fprintf(fp,"====\n\n");

	//複製初始路徑至本地最佳解
	memcpy(&localMinRoute[id],route,sizeof(route));

	//進入 N 次迭代 (模擬退火)
	for (i = 0; i < N; i++) { 	// 同學可把迴圈的條件改以溫度來看，檢視運算效果，譬如把迴圈陳述寫為 for (i = 0; T > 1e-8; i++) {
        //隨機交換 2 個城市
		int rnd_city_A = (int)numRandom(1,7.999);   //因為起點終點都是city 1，故只能交換第1~7個元素，先隨機選取一個1~7的數字
		int rnd_city_B = (rnd_city_A+(int)numRandom(0,5.999))%7+1;  //不能跟自己交換，先隨機選0~5與剛剛挑的數字相加，因不更動第8個元素，且最後會+1位移不遇到0，故mod 7

        int temp = new_route[rnd_city_B];
        new_route[rnd_city_B] = new_route[rnd_city_A];
        new_route[rnd_city_A] = temp;

        //file記錄檔
        fprintf(fp,"[ Time: %d ]\n\n",i);
        fprintf(fp,"隨機互換 2 個走訪城市: City %d & City %d\n",route[rnd_city_A]+1,route[rnd_city_B]+1);
        fprintf(fp,"目前路徑: ");
        filePrintRoute(fp,route);
        fprintf(fp,", 成本: %.1f\n",sumDistance(route));
        fprintf(fp,"交換路徑: ");
        filePrintRoute(fp,new_route);
        fprintf(fp,", 成本: %.1f\n\n",sumDistance(new_route));

        //計算成本差
		delta = sumDistance(new_route) - sumDistance(route);			/* delta為新解的函數值與現有解的函數值之差 */
        fprintf(fp,"本次計算優劣結果:\n");
		fprintf(fp,"新路徑花費成本 - 原路徑花費成本 = %.1f",delta);
        delta > 0 ? fprintf(fp,", 新路徑花費成本：高(較差)\n\n") : fprintf(fp,", 新路徑花費成本：少(較優)\n\n");

        //接受新解 (更好的一定接受，差的依機率接受)
        double rnd_num = numRandom(0,1);

		if (delta > 0) {
            fprintf(fp,"指數運算結果: %.4f , 隨機變數: %.4f ",exp(-delta/T),rnd_num);
            if(exp(-delta/T) > rnd_num){
                memcpy(route, new_route, sizeof(route));	/* 接受比較差的路徑作為目前路徑 */
                fprintf(fp,"/ 隨機變數：小 -> 更新(V) 比較差的路徑 作為 現有路徑\n\n");
            }
            else{
                fprintf(fp,"/ 隨機變數：大 -> 不更新(X) 不改路徑\n\n");
            }
		}
		//如果比歷史最優解好，更新
		else{
            memcpy(route, new_route, sizeof(route));	/* 使用更好的路徑 */
		}

        //印出最佳運算結果
		if (sumDistance(route) < sumDistance(localMinRoute[id])) {
                setColor(10);
                memcpy(&localMinRoute[id],route,sizeof(route));		/* 若目前路徑比本Thread之最佳路徑還更優，覆蓋最佳路徑為當前路徑 */
                printf("執行緒-> %d 發現較佳路徑，成本= %.1f\n",id,sumDistance(route)); //顯示在終端中
                fprintf(fp,"執行緒-> %d 發現較佳路徑，成本= %.1f\n\n",id,sumDistance(route)); //寫進記錄檔中
		}
		T *= alpha;	/* 一回合已處理畢，降溫 */
  	}
  	fprintf(fp,"\n===\n\n");
  	fprintf(fp,"\n[運算結束]\n");
  	fprintf(fp,"\n以上為 [執行續 %d] 所有運算過程\n",id);
  	fprintf(fp,"\n==================================== 運算結果 ==============================================");
  	fprintf(fp,"\n\n=> 最低成本路徑: ");
    filePrintRoute(fp,route);
    fprintf(fp,", 總花費成本: %.1f\n",sumDistance(route));
    fclose(fp);
    setColor(7);
	pthread_exit(NULL);	/* 退出執行緒 */
}

//主程式
int main()
{
    int originRoute[9] = {0,1,2,3,4,5,6,7,0}; //原始預設路徑


	int i , j,minTid, id[NumThreads] = {0};	/* id[]為創建執行緒當下傳入的引數，元素內容為流水編號0, 1, 2, ... */
	pthread_t threads[NumThreads];		/* threads[]為結構變數，儲存作業系統識別執行緒資訊之用 */
	double minFunVal = sumDistance(originRoute); //計算預設路徑的成本
	int minroute[9]; //最短路徑

    //畫面開始
    setColor(7+16 * 21);
  	printf("-------------------------------------------------------------------------------------------\n");
  	printf("|                                     ** 模擬退火法 **                                    |\n");
    printf("|                                    Simulated Annealing                                  |\n");
  	printf("-------------------------------------------------------------------------------------------\n");
  	setColor(12);
  	printf("\nQuesion -> 旅行銷售員問題(Travelling-Salesman Problem) 尋找最短路徑\n\n");
    setColor(7);
  	printf("========================================= 運行資訊 ========================================\n\n");
    setColor(11);
  	printf("::執行版本    : 多執行緒版本\n");
  	printf("::執行緒數量  : %d\n",NumThreads);
  	printf("::起始溫度    : %d\n",InitialTemp);
  	printf("::降溫率      : %.2f\n",alpha);
  	printf("::最多迭代次數: %d\n\n",N);
  	setColor(7);
  	printf("========================================= 運算過程 ========================================\n");
  	printf("\n所有城市之間路徑:\n");
  	printf("Distances between each pair of cities (unit: km)\n\n");

    //印出所有城市間間距圖表

    setColor(3);
    printf("-------------------------------------------------------------------------------------------\n");
    printf("| %-8s", "");
    setColor(7);
    for (i = 0; i < 8; i++)
    {
        setColor(3);
        printf("| ");
        setColor(4);
        printf("%s%2d ","City ",i+1);
        setColor(7);
    }
    setColor(3);
    printf("|\n-------------------------------------------------------------------------------------------\n");
    setColor(7);

    for (i = 0; i < 8; i++) {
        setColor(3);
        printf("| ");
        setColor(4);
        printf("%s%2d ","City ",i+1);
        setColor(7);
        for (j = 0; j < 8; j++) {
            setColor(3);
            printf("| ");
            setColor(6);
            if(distance[i][j]==0){
                setColor(12);
            }
            printf("%6.1f  ", distance[i][j]);
            setColor(7);
        }
        setColor(3);
        printf("|\n-------------------------------------------------------------------------------------------\n");
        setColor(7);
    }

    printf("\n目前預設最小成本路徑: ");
    printRouteB(originRoute);
    printf(", 花費成本: %.1f\n",sumDistance(originRoute));
    printf("\n----\n");
  	printf("\n開始運算模擬退火法函數...\n");
    printf("\n----\n\n");

  	for (i = 0; i < NumThreads; id[++i] = i)
    	if (pthread_create(&threads[i],NULL,findExtreme,(void *)&id[i])) {	/* 創建執行緒 */
    	    setColor(4);
      		printf("執行緒建立失敗\n");
      		printf("pthread_creat() ERROR!!\n");
      		setColor(7);
      		exit(-1);
    	}

  	sleep(1);	/* 主程序暫停1秒，讓執行緒們完成工作，以免螢幕顯示的訊息錯亂不易辨認 */

  	printf("\n----\n");

  	printf("\n\n%d 個執行緒各自最佳結果如下: \n\n",NumThreads);

  	for (i = 0; i < NumThreads; i++) {
		pthread_join(threads[i],NULL);	/* 等待指定的執行緒結束 */
  		printf("執行緒 %d 路徑: ",i);
  		printRouteA(i);
  		printf(", 成本: %.1f\n",sumDistance(localMinRoute[i]));


  		sleep(1);
  		if (sumDistance(localMinRoute[i]) < minFunVal) {
                minFunVal = sumDistance(localMinRoute[i]);
                minTid = i;
                memcpy(&minroute,&localMinRoute[i],sizeof(localMinRoute[i]));
  		}
  	}

  	printf("\n====================================== CONCLUSION =========================================\n");
  	setColor(2);
  	printf("\n| 1.最小成本路徑      : ");
  	printRouteB(minroute);
  	printf("\n| 2.最低花費成本      : %.1f",sumDistance(minroute));
  	printf("\n| 3.最佳結果運算執行緒: %d\n",minTid);
  	setColor(3);
    printf("\n> 請見 %d 個執行續各自的 Output_Log_Thread0,Thread1,Thread2.txt 記錄檔，內涵所有回合運算過程\n\n",NumThreads);
    setColor(7);

	return 0;
}
