#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>

HANDLE hMutex;
HANDLE output_hMutex;
typedef struct _PARAMETER{
    int id;
    char *fileName;
    int *order;
    int *ax;
    int *flag;
    int *Immediate_NUM;
    int *object1;
    int *object2;
}PARAMETER;

int data[2000] = {0};

int power2(int n)
{
    int cnt,y = 1;
    for(cnt = 1 ; cnt <= n ; cnt++){
        y *= 2;
    }
    return y;
}
int binay_TO_decimal(int x[],int cnt1,int cnt2)                                //二进制码 -> 十进制数
{
    int y = 0;
    int cnt;
    if(x[cnt1] == 0){
        for(cnt = cnt1 + 1 ; cnt <= cnt2 ; cnt++){
            y = 2 * y + x[cnt];
        }
    }else if(cnt1 < 16){
        for(cnt = cnt1 ; cnt <= cnt2 ; cnt++){
            y = 2 * y + x[cnt]; 
        }
    }else{
        for(cnt = cnt1 + 1 ; cnt <= cnt2 ; cnt++){
            y = 2 * y + 1 - x[cnt];
        }
        y = - y - 1;
    }
    return y;
}
void getorder(char *fileName,int order[],int object1[],int object2[],int Immediate_NUM[])                 //从dict.dic文件取指令并转化为十进制数，分别保存在命令，操作对象，立即数中
{
     FILE *fptr;
    int p;
    fptr = fopen(fileName,"r");
    if(fptr == NULL){
        printf("open error\n");
    }else{
        int cnt = 0 , i;
        char m;
        while(!feof(fptr)){
            int ORDER[32] = {0};
            i = 0;
            m = fgetc(fptr);
            while(m != '\n' && !feof(fptr)){
                ORDER[i] = m - '0';
                m = fgetc(fptr);
                i++;
            }
            order[cnt] = binay_TO_decimal(ORDER,0,7);
            object1[cnt] = binay_TO_decimal(ORDER,8,11);
            object2[cnt] = binay_TO_decimal(ORDER,12,15);
            Immediate_NUM[cnt] = binay_TO_decimal(ORDER,16,31);
            if(order[cnt] == 0){
                break;
            }
            cnt++;
        }
        fclose(fptr);
    }
}
void data_transfer(int object1,int object2,int Immediate_NUM,int ax[])                  //数据传送
{
    if(object2 == 0){
        ax[object1] = Immediate_NUM;
    }else if(object1 <= 4 && object2 > 4){
        ax[object1] = data[(ax[object2] - 16384)/2];
    }else{
        data[(ax[object1] - 16384)/2] = ax[object2];
    }
}
void arithmetic_plus(int object1,int object2,int Immediate_NUM,int ax[])                //算数加
{
    if(object2 == 0){
        ax[object1] += Immediate_NUM;
    }else{
        ax[object1] += data[(ax[object2] - 16384)/2];
    }
}
void arithmetic_minus(int object1,int object2,int Immediate_NUM,int ax[])               //算数减
{
    if(object2 == 0){
        ax[object1] -= Immediate_NUM;
    }else{
        ax[object1] -= data[(ax[object2] - 16384)/2];
    }
}
void arithmetic_multiplication(int object1,int object2,int Immediate_NUM,int ax[])      //算数乘
{
    if(object2 == 0){
        ax[object1] *= Immediate_NUM;
    }else{
        ax[object1] *= data[(ax[object2] - 16384)/2];
    }
}
void arithmetic_division(int object1,int object2,int Immediate_NUM,int ax[])            //算数除
{
    if(object2 == 0){
        ax[object1] /= Immediate_NUM;
    }else{
        ax[object1] /= data[(ax[object2] - 16384)/2];
    }
}
void logic_and(int object1,int object2,int Immediate_NUM,int ax[])                     //逻辑与
{
    if(object2 == 0){
        if(ax[object1] && Immediate_NUM){
            ax[object1] = 1;
        }else{
            ax[object1] = 0;
        }
    }else{
        if(ax[object1] && data[(ax[object2] - 16384)/2]){
            ax[object1] = 1;
        }else{
            ax[object1] = 0;
        }
    }
}
void logic_or(int object1,int object2,int Immediate_NUM,int ax[])                    //逻辑或
{
    if(object2 == 0){
        if(ax[object1] || Immediate_NUM){
            ax[object1] = 1;
        }else{
            ax[object1] = 0;
        }
    }else{
        if(ax[object1] || data[(ax[object2] - 16384)/2]){
            ax[object1] = 1;
        }else{
            ax[object1] = 0;
        }
    }
}
void logic_not(int object1,int object2,int Immediate_NUM,int ax[])                  //逻辑非
{ 
    if(object2 == 0){
        if(ax[object1]){
            ax[object1] = 0;
        }else{
            ax[object1] = 1;
        }
    }else{
        if(data[(ax[object2] - 16384)/2]){
            data[(ax[object2] - 16384)/2] = 0;
        }else{
            data[(ax[object2] - 16384)/2] = 1;
        }
    }
}
void compare(int object1,int object2,int Immediate_NUM,int *flag,int ax[])         //比较
{
    if(object2 == 0){
        if(ax[object1] == Immediate_NUM){
            *flag = 0;
        }else if(ax[object1] > Immediate_NUM){
            *flag = 1;
        }else{
            *flag = -1;
        }
    }else{
        if(ax[object1] == data[(ax[object2] - 16384)/2]){
            *flag = 0;
        }else if(ax[object1] > data[(ax[object2] - 16384)/2]){
            *flag = 1;
        }else{
            *flag = -1;
        }
    }
}
int jump(int now,int object1,int object2,int Immediate_NUM,int flag)                        //跳转
{
    int y = now;
    if(object2 == 0){
        y = now + Immediate_NUM/4 - 1;
    }else if(object2 == 1 && flag == 0){
        y = now + Immediate_NUM/4 - 1;
    }else if(object2 == 2 && flag == 1){
        y = now + Immediate_NUM/4 - 1;
    }else if(object2 == 3 && flag == -1){
        y = now + Immediate_NUM/4 - 1;
    }
    return y;
} 
void input(int object1,int ax[])                                                          //输入
{
    printf("in:\n");
    scanf("%d",&ax[object1]);
}
void output(int id,int object1,int ax[])                                                         //输出
{
    WaitForSingleObject(output_hMutex, INFINITE);
    printf("\n***********************************************   id is %d, out: %d\n",id,ax[object1]);
    ReleaseMutex (output_hMutex); 
}
void lock_it(int Immediate_NUM)
{
    WaitForSingleObject(hMutex, INFINITE);
}
void unlock_it(int Immediate_NUM)
{
    ReleaseMutex (hMutex); 
}
void sleep_it(int Immediate_NUM)
{
    Sleep(Immediate_NUM);
}

void printNOW(int id,int cnt,int cnt2,int order[],int object1[],int object2[],int flag,int ax[])                                                //输出当前状态
{
	WaitForSingleObject(output_hMutex, INFINITE);
	printf("\n=======ID is %d======\n",id);
	printf("ip = %d\nflag = %d\nir = %d\n",(cnt + 1)*4,flag,order[cnt2]*power2(8) + object1[cnt2]*power2(4) + object2[cnt2]);
	int i;
	for(i = 1 ; i <= 4 ; i++){
		printf("ax%d = %d ",i,ax[i]);
	}
	printf("\n");
	for(i = 5 ; i <= 8 ; i++){
		printf("ax%d = %d ",i,ax[i]);
	}
	printf("\n");
	ReleaseMutex (output_hMutex); 
}
void work(int id,int order[],int object1[],int object2[],int Immediate_NUM[],int *flag,int ax[])                                                                    //逐条分析指令并执行
{
	int CNT,CNT2;                                                                                                                      //CNT为计数器,(CNT+1)*4为指令计数器
    for(CNT = 0 ; order[CNT] != 0 ; CNT++){
    	CNT2 = CNT;
        switch(order[CNT]){
            case 1:
                data_transfer( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break; 
            case 2:
                arithmetic_plus( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break;
            case 3: 
                arithmetic_minus( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break;
            case 4:
                arithmetic_multiplication( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break;
            case 5:
                arithmetic_division( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break;
            case 6:
                logic_and( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break;
            case 7:
                logic_or( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break; 
            case 8:
                logic_not( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , ax );break;
            case 9:
                compare( object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , flag , ax );break;
            case 10:
                CNT2 = CNT; CNT = jump( CNT,object1[CNT] , object2[CNT] , Immediate_NUM[CNT] , *flag );break;
            case 11:
                input( object1[CNT] , ax );break;
            case 12:
                output( id , object1[CNT] , ax );break;
            case 13:
                lock_it(Immediate_NUM[CNT]);break;
            case 14:
				unlock_it(Immediate_NUM[CNT]);break;
			case 15:
				sleep_it(Immediate_NUM[CNT]);break;
            default:
                printf("error");break;
        }
        printNOW( id , CNT , CNT2 , order , object1 , object2 , *flag , ax );                                                                              //输出当前状态
    }
    printNOW( id , CNT , CNT , order , object1 , object2 , *flag , ax );
}
void printDATA()                                                                                                                            //打印数据库
{
    int cnt;
    printf("\ndataSegment :\n");
    for(cnt = 0 ; cnt < 16*16 ; cnt++){
        printf("%d%c",data[cnt],((cnt + 1)%16) ? ' ':'\n');
    }
}
void printCODE(int order[],int object1[],int object2[],int Immediate_NUM[])                                                                           //打印代码
{
	int cnt;
	for(cnt = 0 ; cnt < 8*8 ; cnt++){
        if(Immediate_NUM[cnt] >= 0){
            printf("%d%c",order[cnt]*power2(24) + object1[cnt]*power2(20) + object2[cnt]*power2(16) + Immediate_NUM[cnt],((cnt + 1)%8) ? ' ':'\n');
        }else{
            printf("%d%c",order[cnt]*power2(24) + object1[cnt]*power2(20) + object2[cnt]*power2(16) + power2(16) + Immediate_NUM[cnt],((cnt + 1)%8) ? ' ':'\n');
        }
	}	
}

unsigned _runCode_(void *nptr)
{
    PARAMETER *it = (PARAMETER*)nptr;
    getorder(it -> fileName , it -> order , it -> object1 , it -> object2 , it -> Immediate_NUM );
    work( it -> id , it -> order , it -> object1 , it -> object2 , it -> Immediate_NUM , it -> flag , it -> ax );
    _endthreadex(0);
}

int main()
{
    int order1[1000] = {0};                                                         //存储代码段的前8位，作为命令
    int order2[1000] = {0};
    int object11[1000] = {0};                                                       //储存代码段的9~12位，作为操作对象1
    int object12[1000] = {0};
    int object21[1000] = {0};                                                       //储存代码段的13~16位，作为操作对象2
    int object22[1000] = {0};
    int Immediate_NUM1[1000] = {0};
    int Immediate_NUM2[1000] = {0};                                                 //储存代码段17~32位，作为立即数                                以上为代码段内存
    int ax1[9] = {0};                                                              //通用寄存器 ax1 ~ ax8 ，其中ax0=0不做处理，ax1 ~ ax8为有效段
    int ax2[9] = {0};                                                               
    int flag1 = 0;                                                                 //标志寄存器
    int flag2 = 0;

    PARAMETER th1 = {1,"dict.dic",order1,ax1,&flag1,Immediate_NUM1,object11,object12};
    PARAMETER th2 = {2,"dict.dic",order2,ax2,&flag2,Immediate_NUM2,object21,object22};
    data[0] = 100;

    HANDLE hThread1,hThread2;
    unsigned ThreadID1,ThreadID2;
    hMutex = CreateMutex(NULL,FALSE,NULL);
    output_hMutex = CreateMutex(NULL,FALSE,NULL);
    hThread1 = (HANDLE)_beginthreadex(NULL,0,_runCode_,(void*)&th1,0,&ThreadID1);
    hThread1 = (HANDLE)_beginthreadex(NULL,0,_runCode_,(void*)&th2,0,&ThreadID1);
    WaitForSingleObject(hThread1,INFINITE);
    CloseHandle(hThread1);
    WaitForSingleObject(hThread2,INFINITE);
    CloseHandle(hThread1);
    
    printf("\ncodeSegment :\n");
    printCODE( order1 , object11 , object12 , Immediate_NUM1 );
    printCODE( order2 , object21 , object22 , Immediate_NUM2 );
    printDATA();
    return 0;
}