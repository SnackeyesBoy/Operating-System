/*
Program: sysCall.c (Report comments/bugs to chikh@yuntech.edu.tw)
Function: �Hsystem()�Pspawn�����禡�I�s�~���{���A�ñN�Q�I�s�{�����Ǧ^����ܩ�ù�
Notes: 
	1) �ϥΥ��{�����e�A�O�o����product.c�sĶ��product.exe�A�H�ѩI�s�Ұ�
	2) �ЦP�ǯd�N���{���Pproduct.exe���������Y�A�G�̥i�_�P�ɨæ�B�@�C�Y�i�H�A�t�ΩI�s�ҥΨ禡���p��]�w�H 
	3) �ϥ�system()���H�Ϥ��u�e���v�P�u�I���v����Ҧ� 
*/

#include <stdio.h>
#include <stdlib.h>		//for system()
#include <process.h>	//for spawn() family

int main()
{ 
    int result;
    char *args[] = {"product", "1", "2", "3", "4", "5", "-6", NULL};
    
    printf("\n*** �ϥΨt�ΩI�s�Ұʥ~���{���d�� ***\n\n");
    
    result = system("product 2 3 5 8");
    printf("�ǥ�system()����product.exe�Ǧ^�Ȫ����G�G%d\n",result);
	//result = spawnl(P_WAIT,"product","product","1","2","3","4","5",NULL); 
	result = spawnv(P_WAIT,"product",args); 
	printf("�z�Lspawnv()����product�{���Ǧ^�Ȫ����G�G%d\n",result);
	
	//Windows�@�~�t�Υt��CreateProcess()�i�Ω�Ұʥ~���{���A���y�k�y�������A���O�P�ǹL��t��A�G�Ȥ����� 
    
	return 0; 
}
