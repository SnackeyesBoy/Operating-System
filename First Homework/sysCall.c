/*
Program: sysCall.c (Report comments/bugs to chikh@yuntech.edu.tw)
Function: 以system()與spawn相關函式呼叫外部程式，並將被呼叫程式的傳回值顯示於螢幕
Notes: 
	1) 使用本程式之前，記得先把product.c編譯為product.exe，以供呼叫啟動
	2) 請同學留意本程式與product.exe之間的關係，二者可否同時並行運作。若可以，系統呼叫所用函式應如何設定？ 
	3) 使用system()難以區分「前景」與「背景」執行模式 
*/

#include <stdio.h>
#include <stdlib.h>		//for system()
#include <process.h>	//for spawn() family

int main()
{ 
    int result;
    char *args[] = {"product", "1", "2", "3", "4", "5", "-6", NULL};
    
    printf("\n*** 使用系統呼叫啟動外部程式範例 ***\n\n");
    
    result = system("product 2 3 5 8");
    printf("藉由system()執行product.exe傳回值的結果：%d\n",result);
	//result = spawnl(P_WAIT,"product","product","1","2","3","4","5",NULL); 
	result = spawnv(P_WAIT,"product",args); 
	printf("透過spawnv()執行product程式傳回值的結果：%d\n",result);
	
	//Windows作業系統另有CreateProcess()可用於啟動外部程式，但語法稍嫌複雜，恐令同學過於負擔，故暫不介紹 
    
	return 0; 
}
