/*************************************************************************************************
												zip51 
 										 51芯片上的实时内核
										 与时间管理管理有关的函数
										 作者：孤独剑
														2011年4月12日  修改
文件名：			zip51_time.c
包含函数个数：		4
用户需修改个数：	无
*************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif



/*********************************************************************************************************
函数名		：	Sleep
描述		：	调用该函数睡眠一段时间（放弃CPU使用权）
参数 		：	ticks 睡眠多少  ms （zip51的时间管理单位）
返回值		：	无
注意		：	不要在空闲任务中调用该函数 否则将直接返回
				不要在锁定了调度器未打开之前调用该函数。
				最长的睡眠时间由配置宏SLEEP_LENGTH决定。
*********************************************************************************************************/
void			Sleep (SleepType	ticks )  small
{
		if ( (ticks > 0 ) && ((gSysPriCur					 & 0xFC) == 0) )
		{	
				ENTER_CRITICAL();
				RdyTable						&= ( ~Prio_RdyTable[gSysPriCur]  & 0x0F);				///从就绪表中去掉当前任务级
				gTCB[gSysPriCur	].TimeDly		= ticks;
				EXIT_CRITICAL();
				OSSched ();
		}
}


/*********************************************************************************************************
函数名		：	WakeUp
描述		：	将一个睡眠的任务唤醒
参数 		：	Prio 待唤醒的任务的优先级
返回值		：	无
注意		：	唤醒空闲任务则什么也不做直接返回  空闲任务无法睡眠
				调用该函数的时候被唤醒的任务如果仅仅处在睡眠态则无论睡眠多长时间都会醒来
				但如果还有其他状态（比如被挂起或者等待信号量）则：
				1）如果任务被挂起则还需要等待到用Resume恢复之后才能运行。
				2）如果任务在等待一个内核事件（消息盒子，消息队列，互斥体，信号量）则会立即唤醒该任务
					但此时该任务会认为自己等待超时，实际上并没有等待指定的时间。
					对于无期限等待内核事件的任务则不会唤醒。
*********************************************************************************************************/
#if				WAKE_UP_EN
void			WakeUp (byte	Prio ) 	small
{
		if ( (Prio & 0xFC) == 0 )				///判断优先级是否合法
		{	
			ENTER_CRITICAL();
			if ( gTCB[Prio].TimeDly != 0 )		///仅仅唤醒正在睡眠的函数 ，还可以保证不唤醒无期限等待的任务。
			{
				gTCB[Prio].TimeDly			= 0;
				if ( (gTCB[Prio].TaskStat & TASK_SUSPEND )	== TASK_RDY )
				{
					RdyTable			|= Prio_RdyTable[Prio];
				}
				EXIT_CRITICAL();
				OSSched ();
			 }
			 else
			 {
			 	EXIT_CRITICAL();
			 }
		}
}
#endif


/*********************************************************************************************************
函数名		：	TimeGet
描述		：	得到系统自上电以来的时间 以ms位单位
参数 		：	无
返回值		：	系统子上电以来的时间ms为单位
注意		：	由于zip51的时钟节拍设置为1ms所以如果想让系统运行更长时间这个值仍然有效地话就
				应该将CntType的类型设置成uint16（约65s） 或者uint32 （约49天7小时）的				
*********************************************************************************************************/
#if				TIME_CNT_EN		&&		TIME_GET_EN
CntType			TimeGet (void ) small
{
		CntType		Ticks;
		ENTER_CRITICAL();
		Ticks		= TimeCnt;
		EXIT_CRITICAL();
		return 		Ticks;
}
#endif


/*********************************************************************************************************
函数名		：	TimeSet
描述		：	设置系统自上电以来运行的时间
参数 		：	Time  单位 ms
返回值		：	无
注意		：	无			
*********************************************************************************************************/
#if				TIME_CNT_EN		&&		TIME_SET_EN
void			TimeSet (CntType Time) small
{
		ENTER_CRITICAL();
		TimeCnt		= Time;
		EXIT_CRITICAL();
}
#endif