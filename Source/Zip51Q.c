/*************************************************************************************************
												zip51 
 										 51芯片上的实时内核
										 与消息队列管理有关的函数
										 作者：孤独剑
														2010年10月29日
文件名：			zip51_Q.c
包含函数个数：		7
需要用户修改个数：	无
*************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif






/*********************************************************************************************************
函数名		：	QCreate
描述		：	创建一个消息队列    初始化消息控制块的值
参数 		：	无
返回值		：	无
注意		：	这个值在OSInit函数中调用，用户并不使用这个函数
*********************************************************************************************************/
#if				MESS_Q_EN
void			QCreate (void ) small
{
		gMTCB.MessNum		= 0;
		gMTCB.WaitTable		= 0;
		gMTCB.QStart		= &gMBoxArray[0];
		gMTCB.QOut			= &gMBoxArray[0];
		gMTCB.QIn			= &gMBoxArray[0];
		gMTCB.QEnd			= &gMBoxArray[MAX_MESS_LEN];	
}
#endif


/*********************************************************************************************************
函数名		：	QPend
描述		：	有等待的从一个消息队列中得到一个消息
参数 		：	timeout			如果消息队列中不存在消息指定的等待时间	
				err				返回的错误值		
返回值		：	通过err返回的：
						ERR_NOERR			0 消息申请成功	仅当此时返回的消息值才有意义
						ERR_TIMEOUT			超时标志	
				直接返回的：
						仅当 err为 ERR_NOERR时 返回的消息值才有意义
注意		：	不允许在中断中调用该函数，因为中断是不可以等待的
*********************************************************************************************************/
#if				MESS_Q_EN
MBoxType		QPend (SleepType	timeout ,ErrType	*err ) small
{
		byte		Temp;
		MBoxType	Msg;
		ENTER_CRITICAL();
		if (gMTCB.MessNum > 0 )
		{
			Msg		= *gMTCB.QOut ++;
			gMTCB.MessNum --;
			if (gMTCB.QOut	== gMTCB.QEnd )
			{
				gMTCB.QOut	= gMTCB.QStart;
			}
			*err	= ERR_NOERR;
			EXIT_CRITICAL();
			return	Msg;
	  	}
		gTCB[gSysPriCur	].TaskStat	   		|= TASK_WAITQ;
		gTCB[gSysPriCur	].TimeDly			= timeout;
		RdyTable					       	&=  ( ~Prio_RdyTable[gSysPriCur]  & 0x0F);	
		Temp						        =  0x08;
		Temp					          	>>= gSysPriCur;
		gMTCB.WaitTable					   	|= Temp; 
		EXIT_CRITICAL();

		OSSched ();	

		ENTER_CRITICAL();
		Msg									= gTCB[gSysPriCur].Msg;
		if (Msg )
		{
				gTCB[gSysPriCur	].Msg		= 0;
				gTCB[gSysPriCur	].TaskStat	= TASK_RDY;
				*err						= ERR_NOERR;
				EXIT_CRITICAL();
				return	Msg;
		}
		Temp			     				= 0x08;			
		Temp			  				  >>= gSysPriCur					;	
		gMTCB.WaitTable					   &= ~Temp;
 		gTCB[gSysPriCur	].TaskStat			= TASK_RDY ;
		*err				 				= ERR_TIMEOUT;
		EXIT_CRITICAL();
		return	0;		
}
#endif




/*********************************************************************************************************
函数名		：	QPost
描述		：	将一个消息放在消息队列中，放在队首
参数 		：	Msg					消息值				
返回值		：	ERR_OVERFLOW		消息队列中消息已满
				ERR_NOERR			0 消息投递成功
注意		：	可以在中断中调用该函数  如果有任务等待消息队列中的消息那么不立即在这个函数中切换
				而是在IntExit中切换
				允许投递值为0的消息		 此时：
								如果有任务在等待消息队列则把这个0值消息给最高优先级的任务
								如果没有任务在等待，则丢弃此消息。绝对不可以将0值消息放在消息队列上
*********************************************************************************************************/
#if				MESS_Q_EN
byte			QPost (MBoxType		Msg )small
{
		byte		Temp;
		ENTER_CRITICAL();
		if (gMTCB.MessNum	== MAX_MESS_LEN )
		{
				EXIT_CRITICAL();
				return	ERR_OVERFLOW;
		}
		Temp		= gMTCB.WaitTable;
		if (Temp )
		{
				Temp					= RdyTable_Prio[Temp]; 				///得到等待任务中最高优先级
				gSysPriHighRdy			= Temp;
				Temp					= Prio_RdyTable[Temp];
				gMTCB.WaitTable		   &= ~Temp;
				gTCB[gSysPriHighRdy	].TimeDly			= 0;
				gTCB[gSysPriHighRdy	].Msg				= Msg;
				if ( (gTCB[gSysPriHighRdy].TaskStat	    &= ~TASK_WAITQ ) == TASK_RDY )
				{
						RdyTable	|= Prio_RdyTable[gSysPriHighRdy					];
				}
				EXIT_CRITICAL();

				OSSched ();
				return	ERR_NOERR;
		}
		if (Msg == 0 )			 										///丢弃此消息
		{
			EXIT_CRITICAL();
			return	ERR_NOERR;
		}
		*gMTCB.QIn ++	= Msg;
		gMTCB.MessNum ++;
		if (gMTCB.QIn	== gMTCB.QEnd )
		{
			gMTCB.QIn	= gMTCB.QStart;
		}
		EXIT_CRITICAL();
		return	ERR_NOERR;
}
#endif




/*********************************************************************************************************
函数名		：	QPostOpt
描述		：	将一个消息放在消息队列中，放在队首，不同的是这个函数支持广播方式投递 
 				也就是一旦选择了广播方式那么所有的等待任务都会得到消息
参数 		：	Msg			消息值
				Opt			投递方式
							POST_BROADCAST				发送广播消息
							POST_NOBROADCAST			发送非广播消息
							POST_FRONT					从队首投递
			   	具体使用时可以用上面的宏相或 组成各种情况    如果希望从队尾进入又不广播那么则用POST_NOBROADCAST
返回值		：	ERR_OVERFLOW		消息队列中消息已满
				ERR_NOERR			消息投递成功
注意		：	可以在中断中调用该函数  如果有任务等待消息队列中的消息那么不立即在这个函数中切换
				而是在IntExit中切换
			    允许投递值为0的消息	 0值消息 处理方式 同上
				可以在中断中调用该函数
*********************************************************************************************************/
#if				Q_POST_OPT_EN && MESS_Q_EN
byte			QPostOpt (MBoxType	Msg ,byte	Opt)small
{
		byte		Temp;
		ENTER_CRITICAL();
		if (gMTCB.MessNum	== MAX_MESS_LEN )
		{
				EXIT_CRITICAL();
				return	ERR_OVERFLOW;
		}
		Temp		= gMTCB.WaitTable;
		if (Temp != 0 )														///有任务在等待消息队列
		{
				if (Opt	& POST_BROADCAST )	 								///选择以广播形式发送
				{
					while (Temp )
					{
						Temp					= RdyTable_Prio[Temp];
						gSysPriHighRdy			= Temp;
						Temp					= Prio_RdyTable[Temp];
						gMTCB.WaitTable		   &= ~Temp;
						gTCB[gSysPriHighRdy	].TimeDly					= 0;
						gTCB[gSysPriHighRdy	].Msg						= Msg;
						if ( (gTCB[gSysPriHighRdy].TaskStat	&= ~TASK_WAITQ ) == TASK_RDY )
						{
							RdyTable	|= Prio_RdyTable[gSysPriHighRdy					];
						}
						Temp					= gMTCB.WaitTable;
					}
				}
				else							  							///非广播形式发送
				{
						Temp					= RdyTable_Prio[Temp];
						gSysPriHighRdy							= Temp;
						Temp					= Prio_RdyTable[Temp];
						gMTCB.WaitTable		   &= ~Temp;
						gTCB[gSysPriHighRdy					].TimeDly					= 0;
						gTCB[gSysPriHighRdy					].Msg						= Msg;
						if ( (gTCB[gSysPriHighRdy					].TaskStat	       &= ~TASK_WAITQ ) == TASK_RDY )
						{
							RdyTable	|= Prio_RdyTable[gSysPriHighRdy					];
						}
				}
				EXIT_CRITICAL();

				OSSched ();
				return	ERR_NOERR;
		}
		if(Msg == 0 )				///0值消息不允许进入消息队列
		{
			EXIT_CRITICAL();
			return	ERR_NOERR;
		}
		if (Opt & POST_FRONT ) 			///消息进入消息队列队首
		{
			if (gMTCB.QOut	== gMTCB.QStart )
			{
				gMTCB.QOut	= gMTCB.QEnd;
			}
			* --gMTCB.QOut	= Msg;
		}
		else  							///消息进入消息队列队尾
		{
			*gMTCB.QIn ++	= Msg;
			if (gMTCB.QIn	== gMTCB.QEnd )
			{
				gMTCB.QIn	= gMTCB.QStart;
			}
		}
		gMTCB.MessNum	++;
		EXIT_CRITICAL();
		return	ERR_NOERR;
}
#endif

/*********************************************************************************************************
函数名		：	QPostFront
描述		：	将一个消息放在消息队列中，不同的是这个得放在消息队列的队首 （一般情况下上一个函数
													QPostOpt已经 可以完成这个函数的功能了）
参数 		：	Msg			消息值
返回值		：	ERR_OVERFLOW		消息队列中消息已满
				ERR_NOERR			消息投递成功
注意		：	可以在中断中调用该函数  如果有任务等待消息队列中的消息那么不立即在这个函数中切换
				而是在IntExit中切换
				允许投递值为0的消息		  0值消息 处理方式 同上
				此时的队列具有堆栈的功能 即先进后出
				可以在中断中调用该函数
*********************************************************************************************************/
#if				Q_POST_FRONT_EN && MESS_Q_EN
byte			QPostFront (MBoxType Msg )small 
{
		byte		Temp;
		ENTER_CRITICAL();
		if (gMTCB.MessNum	== MAX_MESS_LEN )
		{
				EXIT_CRITICAL();
				return ERR_OVERFLOW;
		}
		Temp		= gMTCB.WaitTable;
		if (Temp !=  0 )
		{
			Temp					= RdyTable_Prio[Temp];					///得到等待表中最高优先级
			gSysPriHighRdy			= Temp;
			Temp					= Prio_RdyTable[Temp];
			gMTCB.WaitTable		   &= ~Temp;
			gTCB[gSysPriHighRdy	].TimeDly					= 0;
			gTCB[gSysPriHighRdy	].Msg						= Msg;
			if ( (gTCB[gSysPriHighRdy].TaskStat	       		&= ~TASK_WAITQ ) == TASK_RDY )
			{
					RdyTable	|= Prio_RdyTable[gSysPriHighRdy	];
			}
			EXIT_CRITICAL();

			OSSched ();
			return	ERR_NOERR;
		}
		if(Msg == 0 )				///0值消息不允许进入消息队列
		{
			EXIT_CRITICAL();
			return	ERR_NOERR;
		}
		if (gMTCB.QOut	== gMTCB.QStart )
		{
			gMTCB.QOut	= gMTCB.QEnd ;
		}
		* --gMTCB.QOut	= Msg ;
		gMTCB.MessNum ++;
		EXIT_CRITICAL();
		return	ERR_NOERR;
}
#endif



/*********************************************************************************************************
函数名		：	Qquery
描述		：	查询当前消息队列中的消息数目
参数 		：	无
返回值		：	消息数目
注意		：	可以在中断中调用该函数
*********************************************************************************************************/
#if				MESS_Q_EN
byte			Qquery(void) small
{
		byte	Cnt;
		ENTER_CRITICAL();
		Cnt		= gMTCB.MessNum;
		EXIT_CRITICAL();
		return	Cnt;
}
#endif


/*********************************************************************************************************
函数名		：	QAcceptOpt
描述		：	无等待的从消息邮箱接收一个消息
参数 		：	Opt接收的参数取值：
						GET_NOCLEAR				从队首接收不清除
						GET_CLEAR				从队首接收接收完后清除
				err 	接收返回值
返回值		：	通过err返回：
						ERR_NOERR				成功得到消息值，仅当这个时候返回的Msg才有意义
						ERR_NOTACCESS			消息不可用
				直接返回的：
						消息值
注意		：	可以在中断中调用该函数
*********************************************************************************************************/ 
#if				MESS_Q_EN
MBoxType		QAcceptOpt(byte	Opt ,ErrType	*err)small
{
			MBoxType		Msg;
			ENTER_CRITICAL();
			if (gMTCB.MessNum > 0 )
			{
					Msg		= *gMTCB.QOut ;
					if (Opt & GET_CLEAR )
					{
						gMTCB.QOut	++;
						if (gMTCB.QOut 	== gMTCB.QEnd )
						{
							gMTCB.QOut	= gMTCB.QStart;
						}
						gMTCB.MessNum --;
					}
					*err	= ERR_NOERR;
					EXIT_CRITICAL();
					return Msg;					
			}
			*err	= ERR_NOTACCESS;
			EXIT_CRITICAL();	
			return	0;	
}
#endif

