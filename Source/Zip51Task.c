/*************************************************************************************************
												zip51 
 										 51芯片上的实时内核
										 与任务管理有关的函数
										 作者：孤独剑
														2010年10月20日
文件名：			zip51_task.c
包含函数个数：		4
*************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif




/*********************************************************************************************************
函数名		：	TaskCreate
描述		：	创建一个非空闲的任务，创建之后不可以被删除但可以被挂起
参数 		：	task	指向一个无参数无返回值函数的指针
				ptos	分配给任务堆栈的栈顶指针
				Prio	创建的任务的优先级
返回值		：	无
注意		：	仅仅在main函数中调用这个函数，由于zip51不支持动态任务的创建和删除。所有任务在开始就建立
				所以用户无需调用这个函数
				唯一需要用户注意的是在配置文件中给任务分配足够的堆栈
*********************************************************************************************************/
void			TaskCreate	(uint16	task,StkType	*ptos,byte	Prio )  small
{
		
		///这里写成这样很重要！！否则按照51上的大小端 。如果task 是0x40C1 则会返回到0xC104去执行导致系统崩溃
		*ptos			++ 	= *( (byte *)&task + 1 );
		*ptos			++ 	= *(((byte *)&task) );

		*ptos			++ 	= ACC;					///初始化特殊寄存器
		*ptos 	 		++ 	= B;
		*ptos			++ 	= DPH;
		*ptos			++ 	= DPL;

		*ptos			++ 	= (PSW & 0xE7);			//使用第0组寄存器
	   	
		*ptos			++	= 0x00;					//初始化功能寄存器
		*ptos			++	= 0x01;
		*ptos			++	= 0x02;
		*ptos			++	= 0x03;
		*ptos	 		++	= 0x04;
		*ptos			++	= 0x05;
		*ptos			++	= 0x06;
		*ptos				= 0x07;

		gTCB[Prio].StkPtr			= ptos;
		gTCB[Prio].TimeDly			= 0;
		gTCB[Prio].TaskStat			= TASK_RDY;
		#if			MBOX_EN || MESS_Q_EN
		gTCB[Prio].Msg				= 0;
		#endif
		if(Prio	!= 4)
		{
			RdyTable			   |= Prio_RdyTable[Prio];	
		}
}


/*********************************************************************************************************
函数名		：	Resume
描述		：	唤醒（不同于WakeUp）一个被Suspend挂起的任务
参数 		：	Prio	待挂起的任务的优先级			
返回值		：	无
注意		：	Prio仅仅可以为非空闲级的任务如果传递的优先级大于3则什么也不做直接返回
				这个函数是唯一的一个能将  被Susped挂起的任务  恢复的函数
				只能被任务调用
*********************************************************************************************************/
#if				TASK_SUSPEND_EN
void			Resume  (byte Prio ) small
{
		if ( (Prio & 0xFC ) == 0 )
		{
			ENTER_CRITICAL();
			if (  gTCB[Prio].TaskStat & TASK_SUSPEND )
			{
				if ( ( (gTCB[Prio].TaskStat			&= ~TASK_SUSPEND ) 	== TASK_RDY ) &&
				 	 ( gTCB[Prio].TimeDly			== 0 ) )
				{
				 		RdyTable					|= Prio_RdyTable[Prio];
						EXIT_CRITICAL();
						OSSched ();
				}
				else
				{
						EXIT_CRITICAL();
				}
			}
		}
}
#endif







/*********************************************************************************************************
函数名		：	Suspend
描述		：	无条件的挂起一个非空闲的任务
参数 		：	Prio	待挂起的任务的优先级			
返回值		：	无
注意		：	Prio仅仅可以为非空闲级的任务如果传递的优先级大于3则什么也不做直接返回
				任务挂起可以和其它的条件叠加：
				如被挂起的任务可以正在睡眠状态，挂起之后时间到只能用Resume恢复
				被挂起任务正在等待信号量则信号量到达之后只能也不能运行只能通过Resume恢复
				不要在中断中调用这个函数
*********************************************************************************************************/
#if				TASK_SUSPEND_EN
void			Suspend (byte Prio ) small
{
		if ( (Prio & 0xFC) == 0 )
		{
			ENTER_CRITICAL();
			gTCB[Prio].TaskStat		    |= TASK_SUSPEND;
			RdyTable					&= ( ~Prio_RdyTable[Prio]  & 0x0F);				///从就绪表中去掉当前任务级
			EXIT_CRITICAL();
			OSSched ();
		}
}
#endif





















