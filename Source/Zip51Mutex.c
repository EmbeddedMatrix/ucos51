/**********************************************************************************************************************
														zip51
												适合于在51芯片上的实时内核
												作者：孤独剑
												时间：2010 10  24
												包含本工程与互斥型信号量有关的函数
文件名					zip51_mutex.c
函数个数：				4
需要用户修改的个数：	无
这里没有解决优先级翻转问题 。用户使用时需要注意合理的分配优先级尽量避免优先级翻转
由于中断服务不能访问互斥型信号量所以本文件中所有函数都不能在中断服务程序中调用
**********************************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif




/*********************************************************************************************************
函数名		：	MutexCreate
描述		：	创建一个互斥型信号量 
参数 		：	i 		：第几个信号量 zip共支持4个信号量 (i 取值1到4 ）								
返回值		：	ERR_NOERR 		创建成功
				ERR_INVALIDARG	参数无效
				ERR_ALREADYEXIT	信号量已经存在
注意		：	不要再中断中调用该函数 ,刚刚创建时已经有一个信号值。
*********************************************************************************************************/ 
#if				MUTEX_EN
byte			MutexCreate (byte	i ) small
{
			byte		Temp;
			ENTER_CRITICAL ();
			i	--;
			if (i & 0xFC )
			{
					EXIT_CRITICAL();
					return	ERR_INVALIDARG ;
			}
			Temp		= 0x80;
			Temp	  >>= i;
			if (gMutex & Temp )
			{
					EXIT_CRITICAL();
					return	ERR_ALREADYEXIT ;
			}
			else
			{
					gMutex		   |= Temp;
					gWaitMutex[i]	= 0;
					Temp		  >>= 4;
					gMutex		   |= Temp;
					EXIT_CRITICAL();
					return	ERR_NOERR;
			}			
}
#endif


/*********************************************************************************************************
函数名		：	MutexPend
描述		：	等待得到一个互斥型信号量
参数 		：	i 		：第几个信号量 zip共支持4个信号量 (i 取值1到4 ）
				timeout	如果该互斥型信号量不可用等待时间								
返回值		：	ERR_NOERR 		申请成功
				ERR_INVALIDARG	参数无效
				ERR_NOTEXIT		该互斥型信号量不存在
				ERR_TIMEOUT		等待超时
注意		：	不要再中断中调用该函数
*********************************************************************************************************/
#if				MUTEX_EN
byte			MutexPend   (byte	i,SleepType			timeout  ) small
{
			byte		Temp;
			ENTER_CRITICAL();
			i	--;

			if (i & 0xFC )
			{
				EXIT_CRITICAL();
				return	ERR_INVALIDARG ;
			}
			Temp		= 0x80;
			Temp      >>= i;
			if ( (gMutex & Temp) == 0)
			{
				EXIT_CRITICAL();
				return	ERR_NOTEXIT ;
			}
			Temp		= 0x08;
			Temp	  >>= i;
			if ( gMutex & Temp )
			{
				gMutex	&= ~Temp;
				EXIT_CRITICAL();
				return	ERR_NOERR ;
			}
			gTCB[gSysPriCur	].TaskStat		|=	TASK_WAITMUTEX ;
			gTCB[gSysPriCur	].TimeDly	 	 =	timeout;
			RdyTable					    &=  ( ~Prio_RdyTable[gSysPriCur	]  & 0x0F);
			Temp							 = 0x08;
			Temp						   >>= gSysPriCur;
			gWaitMutex[i]					|=  Temp;
			EXIT_CRITICAL();

			OSSched ();	

			ENTER_CRITICAL();
			if ( gTCB[gSysPriCur].TaskStat 	& TASK_WAITMUTEX )
			{
					Temp			     = 0x08;				///这里再次计算是因为Keil会对局部变量做覆盖优化
					Temp			   >>= gSysPriCur					;		///导致Temp值有可能在开中断期间被修改
					gWaitMutex[i]		&= ~Temp;
					gTCB[gSysPriCur	].TaskStat	= TASK_RDY;
					EXIT_CRITICAL();
					return	ERR_TIMEOUT;
			}
			EXIT_CRITICAL();
			return	ERR_NOERR;					
}
#endif


/*********************************************************************************************************
函数名		：	MutexPost
描述		：	释放一个互斥型信号量
参数 		：	i 		：第几个信号量 zip共支持4个信号量 (i 取值1到4 ）												
返回值		：	ERR_NOERR 		0 释放成功
				ERR_INVALIDARG	参数无效	
				ERR_NOTEXIT		信号量不存在
				ERR_OVERFLOW	重复释放互斥型信号量
注意		：	不要在中断中调用该函数
*********************************************************************************************************/
#if				MUTEX_EN
byte			MutexPost	(byte	i ) small
{
			byte		Temp;
			ENTER_CRITICAL();
			i --;
			if (i & 0xFC )
			{
					EXIT_CRITICAL();
					return	ERR_INVALIDARG;
			}
			Temp		= 0x80;
			Temp      >>= i;
			if ( (gMutex & Temp) == 0)
			{
				EXIT_CRITICAL();
				return	ERR_NOTEXIT ;
			}
			Temp	  >>= 4;
			if ( gMutex & Temp )
			{
				EXIT_CRITICAL();
				return	ERR_OVERFLOW;
			}

			if (gWaitMutex[i] != 0 )
			{
			 	Temp				 = RdyTable_Prio[gWaitMutex[i]];			///说明有任务在等待 得到等待表中最高优先级
				gSysPriHighRdy						 = Temp;
				Temp				 = Prio_RdyTable[Temp];
				gWaitMutex[i]	   	&= ~Temp;
				gTCB[gSysPriHighRdy	].TimeDly			 = 0;
				if ( (gTCB[gSysPriHighRdy].TaskStat	&= ~TASK_WAITMUTEX ) == TASK_RDY )
				{
						RdyTable	|= Prio_RdyTable[gSysPriHighRdy					];
				}
				EXIT_CRITICAL();

				OSSched ();
				return	ERR_NOERR;
			}
			gMutex		|= Temp;
			EXIT_CRITICAL();
			return	ERR_NOERR ;
}			
#endif


/*********************************************************************************************************
函数名		：	MutexAccept
描述		：	无等待的得到一个信号量
参数 		：	i 		：第几个信号量 zip共支持4个信号量 (i 取值1到4 ）												
返回值		：	ERR_NOERR 		0 已经成功得到
				ERR_INVALIDARG	参数无效	
				ERR_NOTEXIT		信号量不存在
				ERR_NOTACCESS	信号量不可用	
注意		：	不要再中断中调用该函数
*********************************************************************************************************/
#if				MUTEX_EN
byte			MutexAccept (byte	i ) small
{
			byte		Temp;
			ENTER_CRITICAL();
			i --;

			if (i & 0xFC )
			{
					EXIT_CRITICAL();
					return	ERR_INVALIDARG;
			}
			Temp		= 0x80;
			Temp      >>= i;
			if ( (gMutex & Temp) == 0)
			{
				EXIT_CRITICAL();
				return	ERR_NOTEXIT ;
			}
			Temp	  >>= 4;
			if ( (gMutex & Temp) == 0 )
			{
				EXIT_CRITICAL();
				return	ERR_NOTACCESS;
			}
			gMutex		&= ~Temp;
			EXIT_CRITICAL();
			return	ERR_NOERR;
}
#endif