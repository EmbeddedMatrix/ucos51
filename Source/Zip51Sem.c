/*************************************************************************************************
												zip51 
 										 51芯片上的实时内核
										 与信号量管理有关的函数
										 作者：孤独剑
														2011年4月12日   修改
文件名：			zip51_Sem.c
包含函数个数：		4
需要用户修改个数：	无
*************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif


/*********************************************************************************************************
如果一个任务A调用了Pend类函数 （包括所有的信号量 互斥型信号量 消息邮箱		消息队列  ）指定了等待超时时限
timeout如果在指定的时限之前另外一个任务B调用相应的Post则如果： 
1）B优先级低那么A就立刻运行 并且等待时限减为0直到A放弃CPU使用权并且没有比B更高级任务需运行则B才从Post函数返回
2）B优先级高那么A不会立刻运行
如果在时限到之后仍然没有任务Post  那么A将得到一个timeout标志  标识等待超时
如果A调用Pend时指定超时时限为0则无限期等待直到等待的信号量到达为止
*********************************************************************************************************/


/*********************************************************************************************************
函数名		：	SemAccept
描述		：	无等待的申请一个信号量
参数 		：	i 		：第几个信号量 zip共支持2个信号量	（i取值1表示第一个）			
返回值		：	0  ERR_NOERR表示成功  非0值表示失败				
注意		：	注意对返回值要检查，如果该值不为0说明信号量无效
				中断服务子程序可以调用SemAccept函数；但是并不推荐任务和中断服务程序共享信号量。因为信号量
				一般用于任务级。如果确实需要在任务和中断服务程序中传递信号量则中断服务子程序只可以调用SemPost
*********************************************************************************************************/
#if				SEM_EN
byte			SemAccept  (byte	i ) small
{			
			byte		cnt;
			ENTER_CRITICAL();
			i --;
			if (i & 0xFE )
			{
				EXIT_CRITICAL();
				return	ERR_INVALIDARG;
			}
			cnt			= gSem[i];
			if (cnt	 > 0 )
			{
					-- gSem[i];
				   EXIT_CRITICAL();
				   return 0;
			}
			EXIT_CRITICAL();
			return		1;
}			
#endif	


/*********************************************************************************************************
函数名		：	SemPend
描述		：	如果信号量存在则直接返回 无果不存在则等待申请一个信号量
参数 		：	i 		：第几个信号量 zip共支持2个信号量 (i 属于1或2 ）
				timeout	：如果该信号量不存在那么等待多久				
返回值		：	ERR_NOERR 		0 成功已经申请到信号量
				ERR_TIMEOUT 	等待时间超时	
				ERR_INVALIDARG	参数错误
注意		：	不要在中断中调用该函数
*********************************************************************************************************/  
#if				SEM_EN
byte			SemPend	   (byte	i ,SleepType		timeout) small
{
			byte		Temp;
			ENTER_CRITICAL();
			i --;

			if (i & 0xFE )
			{
				EXIT_CRITICAL();
				return	ERR_INVALIDARG;
			}
			if (gSem[i]	> 0 )
			{
				gSem[i]	--;
				EXIT_CRITICAL();
				return	ERR_NOERR;
			}
			gTCB[gSysPriCur	].TaskStat		|=	TASK_WAITSEM ;
			gTCB[gSysPriCur	].TimeDly		 =	timeout;
			RdyTable					    &=  ( ~Prio_RdyTable[gSysPriCur	]  & 0x0F);	
			Temp						     =  0x08;
			Temp					       >>= gSysPriCur;
			gWaitSem[i]					    |= Temp; 
			EXIT_CRITICAL();

			OSSched ();									///进行一次任务切换

			ENTER_CRITICAL();
			if (gTCB[gSysPriCur	].TaskStat 	& TASK_WAITSEM )
			{
					Temp			     = 0x08;				///这里再次计算是因为Keil会对局部变量做覆盖优化
					Temp			   >>= gSysPriCur					;		///导致Temp值有可能在开中断期间被修改
					gWaitSem[i]			&= ~Temp;
 				    gTCB[gSysPriCur	].TaskStat	= TASK_RDY ;
					EXIT_CRITICAL();
					return	ERR_TIMEOUT;
			}
			EXIT_CRITICAL();
			return	ERR_NOERR;

}
#endif	





/*********************************************************************************************************
函数名		：	SemPost
描述		：	发出一个信号量
参数 		：	i 		：第几个信号量 zip共支持2个信号量							
返回值		：	ERR_NOERR 		已经发送出信号量
				ERR_OVERFLOW 	信号量已满	
				ERR_INVALIDARG 	参数错误	
注意		：	可以在中断中调用该函数与普通任务通信 （但是在中断中调用该函数不会切换任务
				一直到IntExit的时候才会切换
*********************************************************************************************************/
#if				SEM_EN
byte			SemPost    (byte	i ) small
{
			byte		Temp;
			ENTER_CRITICAL();
			i --;

			if (i & 0xFE )
			{
				EXIT_CRITICAL();
				return	ERR_INVALIDARG;
			}
			Temp		= gWaitSem[i];
			if (Temp != 0 )
			{
			 	Temp			= RdyTable_Prio[Temp];			///得到等待表中最高优先级
				gSysPriHighRdy	= Temp;
				Temp			= Prio_RdyTable[Temp];
				gWaitSem[i]	   &= ~Temp;
				gTCB[gSysPriHighRdy	].TimeDly			 = 0;
				if ( (gTCB[gSysPriHighRdy].TaskStat	&= ~TASK_WAITSEM ) == TASK_RDY )
				{
						RdyTable	|= Prio_RdyTable[gSysPriHighRdy	];
				}
				EXIT_CRITICAL();

				OSSched ();
				return	ERR_NOERR;
			}
			if (gSem[i] < 255 )
			{
				gSem[i] ++;
				EXIT_CRITICAL();
				return	ERR_NOERR;
			}
			EXIT_CRITICAL();
			return	ERR_OVERFLOW ;
}
#endif		

/*********************************************************************************************************
函数名		：	SemCreate
描述		：	创建一个信号量
参数 		：	i 				：第几个信号量 zip共支持2个信号量 i取值1或者2
				Init			：信号量初始值取0到255							
返回值		：	ERR_NOERR		创建成功
				ERR_INVALIDARG	参数错误
注意		：	如果i取值非法则不创建信号量
*********************************************************************************************************/
#if				SEM_EN
byte			SemCreate   (byte	i ,byte	Init) small
{	
		ENTER_CRITICAL();
		i --;
		if ((i & 0xFE) == 0 )
		{
				
				gSem[i]		= Init;
				gWaitSem[i]	= 0;
				EXIT_CRITICAL();
				return	ERR_NOERR;
		}		
		EXIT_CRITICAL();
		return	ERR_INVALIDARG;
}
#endif	


