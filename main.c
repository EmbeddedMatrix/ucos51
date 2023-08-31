/**********************************************************************************************************************
														zip51
												适合于在51芯片上的实时内核
												作者：孤独剑
												时间：2011 4  13 修改
												包含本工程所有内嵌汇编的函数
文件名：				main.c
函数个数：				5
需要用户修改的个数：	无
唯一需要用户修改的两个文件之一 另外一个是zip51_cfg.h
**********************************************************************************************************************/
#include "includes.h"



/********************************************************************************************************
					可以将idata区的数据作为堆栈和消息队列使用
********************************************************************************************************/
StkType				TaskStk0[STK0_LEN]		;		//第一个堆栈

StkType				TaskStk1[STK1_LEN]		;		//第二个堆栈

StkType				TaskStk2[STK2_LEN]		;		//第三个堆栈

StkType				TaskStk3[STK3_LEN]		;		//第四个堆栈

StkType				TaskStkIdle[STKIDLE_LEN] ;		///空闲任务的堆栈




void			Task0 (void)  small;

void			Task1 (void)  small;

void			Task2 (void)  small;

void			Task3 (void)  small;

/*********************************************************************************************************
函数名		：	main
描述		：	本工程入口点 zip51不支持动态进程创建和删除所以开始时已经将所有任务建立
参数 		：	无
返回值		：	无
注意		：	无论用户需要与否zip51提供而且仅仅提供了五个任务 （四个分级任务一个空闲任务）
				如果不需要某一个任务可以调用一个 Suspend（）或者用:  while(1)		Sleep (xx);
				但是绝对不能在一个任务函数里面什么也不写 这样会导致系统崩溃
*********************************************************************************************************/
void	main(void )
{
		OsInit ();
		TaskCreate ((uint16)Task0 ,TaskStk0 ,0 );
		TaskCreate ((uint16)Task1 ,TaskStk1 ,1 );
		TaskCreate ((uint16)Task2 ,TaskStk2 ,2 );
		TaskCreate ((uint16)Task3 ,TaskStk3 ,3 );
		
		TaskCreate ((uint16)TaskIdle,TaskStkIdle,4);

		OSStart ();	
}


/*********************************************************************************************************
函数名		：	TaskSw
描述		：	任务切换函数 将 gSysPriCur	优先级任务挂起。开始运行 gSysPriHighRdy	优先级任务
参数 		：	无
返回值		：	无
注意		：	在进入这个函数之前中断已经关闭所以在ret之前还应当开中断
				为系统调用用户不必关心				
*********************************************************************************************************/
void			TaskSw (void )  small 												///通过这个函数可以进入空闲级，但不可以从空闲级出来！！
{
		#pragma		asm																///在进入这个函数之前中断是关闭的，ret之前应当开中断
		push		ACC
		push		B
		push		DPH
		push		DPL
		push		PSW

		push		00H
		push		01H
		push		02H
		push		03H
		push		04H
		push		05H
		push		06H
		push		07H
		#pragma		endasm						
		
		gTCB[gSysPriCur	].StkPtr		= SP;	
		gSysPriCur						= gSysPriHighRdy;				///在这里PriHighRdy是可以是第三级，或者空闲级 当然也可是0 1 2 级
		SP								= gTCB[gSysPriCur].StkPtr;
		
		#pragma		asm
		pop			07H
		pop			06H
		pop			05H
		pop			04H
		pop			03H
		pop			02H
		pop			01H
		pop			00H

		pop			PSW
		pop			DPL
		pop			DPH
		pop			B
		pop			ACC
		#pragma		endasm

		EA			= 1;									///任务切换时中断时关闭的，在ret进入任务之前需要把中断打开
		#pragma		asm
		ret
		#pragma		endasm	
}	



/*********************************************************************************************************
函数名		：	IntSw
描述		：	中断服务程序（ISR）在调用Int_Exit （）时，Int_Exit根据是否应该切换任务调用这个函数
参数 		：	无
返回值		：	无
注意		：	完成在中断中的任务切换。在进入这个函数之前中断已经被关闭
				所以在ret之前应该开中断（这里与x86不同）
				这个函数是系统内核调用用户不必关心
*********************************************************************************************************/
void			IntSw (void )  small									///在中断中切换任务   注意返回指令是 ：reti
{
		gSysPriCur						= gSysPriHighRdy;
		SP								= gTCB[gSysPriCur].StkPtr;
		
		#pragma		asm
		pop			07H
		pop			06H
		pop			05H
		pop			04H
		pop			03H
		pop			02H
		pop			01H
		pop			00H

		pop			PSW
		pop			DPL
		pop			DPH
		pop			B
		pop			ACC
		#pragma		endasm
	
		EA			= 1;
		#pragma		asm
		reti	
		#pragma		endasm
}

	

/*********************************************************************************************************
函数名		：	OSStart
描述		：	启动多任务环境 ，将OSRunning置为1，开启系统定时器
参数 		：	无
返回值		：	无
注意		：	仅仅在main函数中调用这个函数，由于zip51不支持动态任务的创建和删除。所有任务在开始就建立
				所以这个函数开启的第一个任务是优先级为0的任务
				在ret之前还需注意打开全局中断
				用户无需调用这个函数
*********************************************************************************************************/
void			OSStart	(void )  small														
{
		OSRunning					= 1;
		gSysPriHighRdy				= RdyTable_Prio[RdyTable];								///通过查表得到当前的最高就绪了的任务优先级		
		gSysPriCur					= gSysPriHighRdy;
		SP							= gTCB[gSysPriCur].StkPtr;

		#pragma			asm
		pop			07H
		pop			06H
		pop			05H
		pop			04H
		pop			03H
		pop			02H
		pop			01H
		pop			00H

		pop			PSW
		pop			DPL
		pop			DPH
		pop			B
		pop			ACC
		#pragma			endasm
		EA							= 1;	
		#pragma			asm
		ret																	
			///通过这条指令 开始执行第一个任务的第一条指令
		#pragma			endasm
}


/*********************************************************************************************************
函数名		：	time
描述		：	时钟中断服务函数（ISR）每隔1ms会执行一次
参数 		：	无
返回值		：	无
注意		：	为系统运行提供最基本的时钟服务，用户必须保留 这个中断
*********************************************************************************************************/
void			time(void)		interrupt	3 	using	3		///中断服务程序，因为这个ISR使用了第三组寄存器所以对于第三组应当保存REG
{																///系统仅仅保存PC对于其它的寄存器不保存 
		TH1							= TH;
		TL1							= TL;
	
		IntEnter ();

		ENTER_CRITICAL();										///经过反汇编得知 编译系统自动的保存了常用SFR所以这里不用保存PSW 
																///任务使用0组寄存器，次中断使用第3组寄存器。
		#pragma			asm
		push		00H
		push		01H
		push		02H
		push		03H
		push		04H
		push		05H
		push		06H
		push		07H
		#pragma			endasm

		if (gIntNest == 1 )
		{
			gTCB[gSysPriCur].StkPtr		= SP ;
		}										
		EXIT_CRITICAL();

		TimeTick (); 
		
		IntExit ();

		#pragma			asm
		pop			07H
		pop			06H
		pop			05H
		pop			04H
		pop			03H
		pop			02H
		pop			01H
		pop			00H
		#pragma			endasm
}




