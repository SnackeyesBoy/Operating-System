//B11036014 �|�q���|B �L�ε�
/*
Program: MT-find-minimum-no-mutex.c (Report comments/bugs to chikh@yuntech.edu.tw)
Function: �X�Rfind-minimum.c�B�H�h�������@�u�����h���k�v�{�ǡA�O�C�@�Ӱ�����U�ۧ�M�̨θѡA���Ҧ������������A�A��
		�D�{���P�_���@�Ӱ���������G�̨Ψç@���̫�ѵ��C
Note: 1) �ЦP�ǥ���find-minimum.c���[�c�P�B�@�޿�x���n�A��Ū���{���N����i�J���p
	  2) �i�X�R���{���B���Φh��������S�ʡA��M��ƪ��ϰ�̤p��(�i��s�b�h��)
	  3) ���{�����o�A��h�Ӱ�����������u�P�B�v(synchronization)����A�ݧڭ̶i�ר�Ĥ��椸�A�P�ǥi�A�f�����{���X�þɤJ
		 ������(mutual exclusion lock)���{���󧹳�
*/

#include <stdio.h>
#include <stdlib.h>		/* for RAND_MAX */
#include <time.h>			/* for srand(), rand() */
#include <math.h>			/* for log() */
#include <pthread.h>		/* ��@�h������һ�Pthread������ƫ��A�P���*/
#include <unistd.h>		/* for sleep() */
#include <windows.h>      //setColor()

//�B��ѼƳ]�w
#define NumThreads	3	/* �N�Ыت�������Ӽ� */
#define InitialTemp	100	/* �_�l�ū� */
#define alpha 		0.95	/* ���Ųv */
#define N 			100 /* �̦h���N���� */

//�s�̨θ��|�A�@ 9 �I�A�]�t�^��_�I
int localMinRoute[NumThreads][9];

//�Ҧ����������Z��
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

//�q[min,max]�϶������ͤ@���H���ü�
double numRandom(double min, double max)
{
	return (double)rand()/RAND_MAX*(max-min)+min;
}

//Ū�J���X���|�A�p��Ҧ����X�`�Z�������̫�^��
double sumDistance(int route[9]){
    double sum = 0;
    for (int i=0 ; i<8 ;i++)
    {
        sum += distance[route[i]][route[i+1]];
    }
    return sum;
}

//�Ƶ{���ŧi
void printRouteA(int i);// �L�Ӱ�������̨θ��|
void printRouteB(int route[9]);// �L���w���|
void filePrintRoute(FILE* fp, int route[9]); // ��X���ɮ�
void setColor(int color);

// �L�Ӱ�������̨θ��|
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

// �L���w���|
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

// ��X���ɮ�
void filePrintRoute(FILE* fp, int route[9])
{
    int j;
    for(j = 0;j < 9;j++){
        fprintf(fp,"%d ",route[j]+1);
        if (j!=8){
            fprintf(fp,"-> ");
        }
    }
    //fseek(fp,-2,SEEK_END); //�����ɮץ��ݡA�q�`�ΨӨ��o�ɮפj�p
    //fprintf(fp,"  ");
    return;
}

// �]�w��r�C��
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

//��̤p�Ȩ�ơA�N�Q���������->�t��k�Ҧb
void *findExtreme(void *threadId)
{

    int route[9] = {0,1,2,3,4,5,6,7,0}; //�s�ثe���b���������|->��e�����h�����u�{�����|�v
    int new_route[9] = {0,1,2,3,4,5,6,7,0}; //�s�s�H�����ͪ��ܰʫ���|->�ΨӸձ��洫�⫰����A�p�ⲣ�ͪ��s���|

	int 	i, id = *(int*)threadId; //id�������
  	double 	T = InitialTemp, delta, x, xNew;

  	printf("����� %d �}�l�u�@...\n",id);
  	fflush(stdout);//�ߧY�L�X

	srand(time(NULL)+id*1000);	/* Ū�������ɶ��@���üƲ��;�����l�ƺؤl�A�ɤJid�@���ؤl���@�����A�Ǧ��קK�Ҧ�������ϥΨ�ۦP����l�� */

    //�إ߿�Xlog��
    char fname[64]; //�ɮצW�٦줸�j�p
    sprintf(fname,"Output_Log_Thread_%d.txt",id); //�ɮצW��
    FILE* fp = fopen(fname,"w"); //�}���ɮסA�g�J
    fprintf(fp,"Output_Log_Thread_%d �p�⵲�G: ",id);
    fprintf(fp,"\n========================================= �B���T =================================\n\n");
    fprintf(fp,"::�B�������s�� �G %d\n",id);
  	fprintf(fp,"::���檩��      �G �h���������\n");
  	fprintf(fp,"::������ƶq    �G %d\n",NumThreads);
  	fprintf(fp,"::�_�l�ū�     �G %d\n",InitialTemp);
  	fprintf(fp,"::���Ųv       �G %.2f\n",alpha);
  	fprintf(fp,"::�̦h���N����  �G %d\n",N);
    fprintf(fp,"\n========================================= ��e���� =================================\n\n");
    fprintf(fp,"�ثe�w�]�̤p�������|: ");
    filePrintRoute(fp,route);
    fprintf(fp,", �`��O����: %.1f",sumDistance(route));
    fprintf(fp,"\n\n========================================= �B��L�{ =================================\n\n");
    fprintf(fp,"�B��}�l...\n\n");
    fprintf(fp,"====\n\n");

	//�ƻs��l���|�ܥ��a�̨θ�
	memcpy(&localMinRoute[id],route,sizeof(route));

	//�i�J N �����N (�����h��)
	for (i = 0; i < N; i++) { 	// �P�ǥi��j�骺�����H�ūרӬݡA�˵��B��ĪG�AĴ�p��j�鳯�z�g�� for (i = 0; T > 1e-8; i++) {
        //�H���洫 2 �ӫ���
		int rnd_city_A = (int)numRandom(1,7.999);   //�]���_�I���I���Ocity 1�A�G�u��洫��1~7�Ӥ����A���H������@��1~7���Ʀr
		int rnd_city_B = (rnd_city_A+(int)numRandom(0,5.999))%7+1;  //�����ۤv�洫�A���H����0~5�P���D���Ʀr�ۥ[�A�]����ʲ�8�Ӥ����A�B�̫�|+1�첾���J��0�A�Gmod 7

        int temp = new_route[rnd_city_B];
        new_route[rnd_city_B] = new_route[rnd_city_A];
        new_route[rnd_city_A] = temp;

        //file�O����
        fprintf(fp,"[ Time: %d ]\n\n",i);
        fprintf(fp,"�H������ 2 �Ө��X����: City %d & City %d\n",route[rnd_city_A]+1,route[rnd_city_B]+1);
        fprintf(fp,"�ثe���|: ");
        filePrintRoute(fp,route);
        fprintf(fp,", ����: %.1f\n",sumDistance(route));
        fprintf(fp,"�洫���|: ");
        filePrintRoute(fp,new_route);
        fprintf(fp,", ����: %.1f\n\n",sumDistance(new_route));

        //�p�⦨���t
		delta = sumDistance(new_route) - sumDistance(route);			/* delta���s�Ѫ���ƭȻP�{���Ѫ���ƭȤ��t */
        fprintf(fp,"�����p���u�H���G:\n");
		fprintf(fp,"�s���|��O���� - ����|��O���� = %.1f",delta);
        delta > 0 ? fprintf(fp,", �s���|��O�����G��(���t)\n\n") : fprintf(fp,", �s���|��O�����G��(���u)\n\n");

        //�����s�� (��n���@�w�����A�t���̾��v����)
        double rnd_num = numRandom(0,1);

		if (delta > 0) {
            fprintf(fp,"���ƹB�⵲�G: %.4f , �H���ܼ�: %.4f ",exp(-delta/T),rnd_num);
            if(exp(-delta/T) > rnd_num){
                memcpy(route, new_route, sizeof(route));	/* ��������t�����|�@���ثe���| */
                fprintf(fp,"/ �H���ܼơG�p -> ��s(V) ����t�����| �@�� �{�����|\n\n");
            }
            else{
                fprintf(fp,"/ �H���ܼơG�j -> ����s(X) ������|\n\n");
            }
		}
		//�p�G����v���u�Ѧn�A��s
		else{
            memcpy(route, new_route, sizeof(route));	/* �ϥΧ�n�����| */
		}

        //�L�X�̨ιB�⵲�G
		if (sumDistance(route) < sumDistance(localMinRoute[id])) {
                setColor(10);
                memcpy(&localMinRoute[id],route,sizeof(route));		/* �Y�ثe���|��Thread���̨θ��|�٧��u�A�л\�̨θ��|����e���| */
                printf("�����-> %d �o�{���θ��|�A����= %.1f\n",id,sumDistance(route)); //��ܦb�׺ݤ�
                fprintf(fp,"�����-> %d �o�{���θ��|�A����= %.1f\n\n",id,sumDistance(route)); //�g�i�O���ɤ�
		}
		T *= alpha;	/* �@�^�X�w�B�z���A���� */
  	}
  	fprintf(fp,"\n===\n\n");
  	fprintf(fp,"\n[�B�⵲��]\n");
  	fprintf(fp,"\n�H�W�� [������ %d] �Ҧ��B��L�{\n",id);
  	fprintf(fp,"\n==================================== �B�⵲�G ==============================================");
  	fprintf(fp,"\n\n=> �̧C�������|: ");
    filePrintRoute(fp,route);
    fprintf(fp,", �`��O����: %.1f\n",sumDistance(route));
    fclose(fp);
    setColor(7);
	pthread_exit(NULL);	/* �h�X����� */
}

//�D�{��
int main()
{
    int originRoute[9] = {0,1,2,3,4,5,6,7,0}; //��l�w�]���|


	int i , j,minTid, id[NumThreads] = {0};	/* id[]���Ыذ������U�ǤJ���޼ơA�������e���y���s��0, 1, 2, ... */
	pthread_t threads[NumThreads];		/* threads[]�����c�ܼơA�x�s�@�~�t���ѧO�������T���� */
	double minFunVal = sumDistance(originRoute); //�p��w�]���|������
	int minroute[9]; //�̵u���|

    //�e���}�l
    setColor(7+16 * 21);
  	printf("-------------------------------------------------------------------------------------------\n");
  	printf("|                                     ** �����h���k **                                    |\n");
    printf("|                                    Simulated Annealing                                  |\n");
  	printf("-------------------------------------------------------------------------------------------\n");
  	setColor(12);
  	printf("\nQuesion -> �Ȧ�P������D(Travelling-Salesman Problem) �M��̵u���|\n\n");
    setColor(7);
  	printf("========================================= �B���T ========================================\n\n");
    setColor(11);
  	printf("::���檩��    : �h���������\n");
  	printf("::������ƶq  : %d\n",NumThreads);
  	printf("::�_�l�ū�    : %d\n",InitialTemp);
  	printf("::���Ųv      : %.2f\n",alpha);
  	printf("::�̦h���N����: %d\n\n",N);
  	setColor(7);
  	printf("========================================= �B��L�{ ========================================\n");
  	printf("\n�Ҧ������������|:\n");
  	printf("Distances between each pair of cities (unit: km)\n\n");

    //�L�X�Ҧ����������Z�Ϫ�

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

    printf("\n�ثe�w�]�̤p�������|: ");
    printRouteB(originRoute);
    printf(", ��O����: %.1f\n",sumDistance(originRoute));
    printf("\n----\n");
  	printf("\n�}�l�B������h���k���...\n");
    printf("\n----\n\n");

  	for (i = 0; i < NumThreads; id[++i] = i)
    	if (pthread_create(&threads[i],NULL,findExtreme,(void *)&id[i])) {	/* �Ыذ���� */
    	    setColor(4);
      		printf("������إߥ���\n");
      		printf("pthread_creat() ERROR!!\n");
      		setColor(7);
      		exit(-1);
    	}

  	sleep(1);	/* �D�{�ǼȰ�1��A��������̧����u�@�A�H�K�ù���ܪ��T�����ä�����{ */

  	printf("\n----\n");

  	printf("\n\n%d �Ӱ�����U�۳̨ε��G�p�U: \n\n",NumThreads);

  	for (i = 0; i < NumThreads; i++) {
		pthread_join(threads[i],NULL);	/* ���ݫ��w����������� */
  		printf("����� %d ���|: ",i);
  		printRouteA(i);
  		printf(", ����: %.1f\n",sumDistance(localMinRoute[i]));


  		sleep(1);
  		if (sumDistance(localMinRoute[i]) < minFunVal) {
                minFunVal = sumDistance(localMinRoute[i]);
                minTid = i;
                memcpy(&minroute,&localMinRoute[i],sizeof(localMinRoute[i]));
  		}
  	}

  	printf("\n====================================== CONCLUSION =========================================\n");
  	setColor(2);
  	printf("\n| 1.�̤p�������|      : ");
  	printRouteB(minroute);
  	printf("\n| 2.�̧C��O����      : %.1f",sumDistance(minroute));
  	printf("\n| 3.�̨ε��G�B������: %d\n",minTid);
  	setColor(3);
    printf("\n> �Ш� %d �Ӱ�����U�۪� Output_Log_Thread0,Thread1,Thread2.txt �O���ɡA���[�Ҧ��^�X�B��L�{\n\n",NumThreads);
    setColor(7);

	return 0;
}
