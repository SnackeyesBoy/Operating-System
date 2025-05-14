/*
Program: product.c (Report comments/bugs to chikh@yuntech.edu.tw)
Function：從命令提示列所輸入的引數一一讀入(引數個數不限)，並計算這些引數的乘積
	例:在命令提示列鍵入 product 2 3 5 20 8 12 <Enter>
	程式可顯示 "共輸入6個數，乘積=57600"
*/

#include <stdio.h>
#include <stdlib.h>	/* for exit(), atoi() */

int main (int argc, char *argv[])
{
    int i, ret = 1;

    if (argc < 3) {
	    printf("Too few arguments :( \n\n");
	    exit(0) ;
    }

    for (i = 1; i < argc; i++)
	    ret *= atoi(argv[i]); //atof()將字串轉換成浮點數(float)

	printf("product.exe 讀入%d個數，乘積=%d\n\n",argc-1,ret);

    return ret;
}
