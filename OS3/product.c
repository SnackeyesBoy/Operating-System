/*
Program: product.c (Report comments/bugs to chikh@yuntech.edu.tw)
Function�G�q�R�O���ܦC�ҿ�J���޼Ƥ@�@Ū�J(�޼ƭӼƤ���)�A�íp��o�Ǥ޼ƪ����n
	��:�b�R�O���ܦC��J product 2 3 5 20 8 12 <Enter>
	�{���i��� "�@��J6�ӼơA���n=57600"
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
	    ret *= atoi(argv[i]); //atof()�N�r���ഫ���B�I��(float)

	printf("product.exe Ū�J%d�ӼơA���n=%d\n\n",argc-1,ret);

    return ret;
}
