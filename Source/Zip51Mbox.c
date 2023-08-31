/**********************************************************************************************************************
														zip51
												适合于在51芯片上的实时内核
												作者：孤独剑
												时间：2010 10  27
												包含本工程与消息邮箱管理有关的函数
文件名					zip51_mbox.c
函数个数：				5
需要用户修改的个数：	无
最多的消息邮箱个数由配置文件中的 MBOX_NUM 决定
**********************************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif





/*********************************************************************************************************
函数名		：	MBoxCreate
描述		：	创建一个消息邮箱
参数 		：	i 		：		第几个消息邮箱 i值不能大于MBOX_NUM	
				Msg		:		初始的消息值，不可以是0				
返回值		：	ERR_NOERR 		0 创建成功
				ERR_INVALIDARG	参数无效
注意		：	不要在中断中调用该函数	 Msg为0则失败			
*********************************************************************************************************/ 
#if				MBOX_EN
byte			MBoxCreate	(byte	i ,MBoxType	Msg ) small
{
	ENTER_CRITICAL();
	if ((i > MBOX_NUM) || (Msg == 0) )
	{
		EXIT_CRITICAL();
		return	ERR_INVALIDARG;
	}
	i --;
	gWaitMBox[i]	= 0;
	gMBox[i]		= Msg;
	EXIT_CRITICAL();	
	return ERR_NOERR;
}
#endif




/*********************************************************************************************************
函数名		：	MBoxPend
描述		：	等待得到一个消息邮箱
参数 		：	i 				：第几个消息邮箱 i值不能大于 MBOX_NUM
				timeout			如果消息邮箱中没有消息  则指定的等待时间		
				err				指针指向一个返回值 带有错误代码				
返回值		：	通过err返回的：
						ERR_NOERR 		0 申请成功
						ERR_INVALIDARG	参数无效
						ERR_TIMEOUT		等待超时
				直接返回的：
						消息值   仅当不为0才有效
注意		：	不要在中断中调用该函数
				调用该函数时应当定义一个在idata内存中的一字节数据并将地址传递过去 注意必须是全局变量
				如果返回超时并不一定是等待时间到了没有得到消息还有可能是
				被另外一个任务调用Post发送了一个空消息 （消息值为0 ）
*********************************************************************************************************/
#if				MBOX_EN
MBoxType		MBoxPend  (byte	i ,SleepType	timeout ,ErrType	*err ) small
{
		byte		Temp;
		MBoxType	Msg;
		ENTER_CRITICAL();
		if (i > MBOX_NUM )
		{
				*err	= ERR_INVALIDARG;
				EXIT_CRITICAL();
				return	0;
		}
		i --;

		if (gMBox[i]	!= 0 )
		{
				Msg		= gMBox[i];
				gMBox[i]= 0;
				*err	= ERR_NOERR;
				EXIT_CRITICAL();
				return Msg;
		}
		gTCB[gSysPriCur	].TaskStat	   	   |= TASK_WAITMBOX;
		gTCB[gSysPriCur	].TimeDly			= timeout;
		RdyTable					       &=  ( ~Prio_RdyTable[gSysPriCur]  & 0x0F);	
		Temp						        =  0x08;
		Temp					          >>= gSysPriCur;
		gWaitMBox[i]					   |= Temp; 
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

		Temp			  		    = 0x08;				///这里再次计算是因为Keil会对局部变量做覆盖优化
		Temp			   		  >>= gSysPriCur;		///导致Temp值有可能在开中断期间被修改
		gWaitMBox[i]			   &= ~Temp;
 		gTCB[gSysPriCur	].TaskStat	= TASK_RDY ;
		*err				 		= ERR_TIMEOUT;
		EXIT_CRITICAL();
		return	0;		
}
#endif




/*********************************************************************************************************
函数名		：	MBoxPost
描述		：	向消息邮箱中发送一个消息
参数 		：	i 				：第几个消息邮箱 i值不能大于 MBOX_NUM
				Msg				指定的要发送的消息	这里可以是 0
返回值		：	ERR_NOERR 		0 发送成功
				ERR_INVALIDARG	参数无效
				ERR_OVERFLOW	消息邮箱中已经存在消息
注意		：	中断中可以调用该函数但是在该函数中不会切换 一直等到IntExit函数才会切换
*********************************************************************************************************/
#if				MBOX_EN
byte			MBoxPost	(byte	i ,MBoxType	Msg ) small
{	
		byte		Temp;
		ENTER_CRITICAL();
		if (i > MBOX_NUM )
		{
				EXIT_CRITICAL();
				return	ERR_INVALIDARG;
		}
		i --;

		if ( gMBox[i] )														///已经有消息，不可能再有等待的任务了
		{
				EXIT_CRITICAL();
				return	ERR_OVERFLOW;
		}
		
		Temp		= gWaitMBox[i];
		if (Temp !=  0 )
		{
			Temp							= RdyTable_Prio[Temp];					///得到等待表中最高优先级
			gSysPriHighRdy					= Temp;
			Temp							= Prio_RdyTable[Temp];
			gWaitMBox[i]		   		   &= ~Temp;
			gTCB[gSysPriHighRdy	].TimeDly	= 0;
			gTCB[gSysPriHighRdy	].Msg		= Msg;
			if ( (gTCB[gSysPriHighRdy].TaskStat	 &= ~TASK_WAITMBOX ) == TASK_RDY )
			{
					RdyTable	|= Prio_RdyTable[gSysPriHighRdy	];
			}
			EXIT_CRITICAL();

			OSSched ();
			return	ERR_NOERR;
		}
		gMBox[i]	= Msg;
		EXIT_CRITICAL();
		return	ERR_NOERR;
}
#endif




/*********************************************************************************************************
函数名		：	MBoxPostOpt
描述		：	有选择的向消息邮箱中发送一个消息
参数 		：	i 				：第几个消息邮箱 i值不能大于 MBOX_NUM
				Msg				指定的要发送的消息		这里可以是 0
				opt				发送的方式取值
								POST_BROADCAST		以广播方式发送所有等待该邮箱的任务都会得到消息并就绪
								POST_NOBROADCAST	不是以广播的方式发送，仅仅最高优先级任务会得到消息
返回值		：	ERR_NOERR 		0 发送成功
				ERR_INVALIDARG	参数无效
				ERR_OVERFLOW	消息邮箱中已经存在消息
注意		：	中断中可以调用该函数但是在该函数中不会切换 一直等到IntExit函数才会切换
				在这里Msg可以是0,如果为0那么：
						被这个函数就绪了的所有任务则得到超时标志
						并且如果同时指定opt为 POST_BROADCAST 那么相当于删除了这个消息邮箱
						（所有等待的任务状态都就绪 去掉等待消息邮箱标志  等待列表为0 并且所得到的消息邮箱的值为0 ）
*********************************************************************************************************/
#if				MBOX_EN & MBOX_POSTOPT_EN
byte			MBoxPostOpt (byte	i ,MBoxType	  Msg ,byte	Opt ) small
{
		byte		Temp;
		ENTER_CRITICAL();
		if (i > MBOX_NUM )
		{
				EXIT_CRITICAL();
				return	ERR_INVALIDARG;
		}
		i --;

		if (gMBox[i] )
		{
				EXIT_CRITICAL();
				return	ERR_OVERFLOW;
		}


		Temp		= gWaitMBox[i];
		if (Temp !=  0 )									///有任务在等待
		{
			if ( (Opt	& POST_BROADCAST) != 0 )						///广播
			{
					while ( Temp )
					{
						Temp					= RdyTable_Prio[Temp];					///得到等待表中最高优先级
						gSysPriHighRdy			= Temp;
						Temp					= Prio_RdyTable[Temp];
						gWaitMBox[i]		   				   &= ~Temp;				///从邮箱等待表中去除任务
						gTCB[gSysPriHighRdy	].TimeDly 			= 0;
						gTCB[gSysPriHighRdy	].Msg				= Msg;
						if ( (gTCB[gSysPriHighRdy].TaskStat		&= ~TASK_WAITMBOX ) == TASK_RDY )
						{
								RdyTable	|= Prio_RdyTable[gSysPriHighRdy	];
						}
						Temp			= gWaitMBox[i];
					}
			}
			else																///非广播
			{
				Temp					= RdyTable_Prio[Temp];					///得到等待表中最高优先级
				gSysPriHighRdy			= Temp;
				Temp					= Prio_RdyTable[Temp];
				gWaitMBox[i]		   &= ~Temp;								///从邮箱等待表中去除任务
				gTCB[gSysPriHighRdy	].TimeDly					= 0;
				gTCB[gSysPriHighRdy	].Msg						= Msg;
				if ( (gTCB[gSysPriHighRdy ].TaskStat	       &= ~TASK_WAITMBOX ) == TASK_RDY )
				{
						RdyTable	|= Prio_RdyTable[gSysPriHighRdy	];
				}
			}
			EXIT_CRITICAL();

			OSSched ();
			return	ERR_NOERR;
		}
		gMBox[i]	= Msg;
		EXIT_CRITICAL();
		return	ERR_NOERR;
}
#endif




/*********************************************************************************************************
函数名		：	MBoxAcceptOpt
描述		：	无等待的从消息邮箱中得到消息
参数 		：	i 				：第几个消息邮箱 i值不能大于 MBOX_NUM	
				Opt				申请邮箱的方式取值：
								GET_CLEAR		消息邮箱中有消息得到消息之后将原来的消息清除
								GET_NOCLEAR		消息邮箱中有消息得到消息之后不将原来的消息清除
返回值		：	通过err返回 ：
								ERR_NOERR 		0 成功   此时返回值就是邮箱值
								ERR_INVALIDARG  参数无效	
								ERR_NOTACCESS	邮箱不可用
				如果是0则说明没有得到消息
				如果非零则说明已经得到消息  返回值就是消息
注意		：	中断中可以调用该函数
*********************************************************************************************************/
#if				MBOX_EN
MBoxType		MBoxAcceptOpt (byte	i ,byte	Opt ,ErrType	*err ) small
{
		MBoxType	Temp;
		ENTER_CRITICAL();
		if (i > MBOX_NUM )
		{
			*err	= ERR_INVALIDARG;
			EXIT_CRITICAL();
			return 0;
		}
		i --;

		Temp	= gMBox[i];
		if (Opt & GET_CLEAR )
		{
			gMBox[i]	= 0;
		}
		if (Temp != 0 )
		{
			*err	= ERR_NOERR;
			EXIT_CRITICAL();
			return Temp;
		}
		*err	= ERR_NOTACCESS;
		EXIT_CRITICAL();
		return 0;
}	
#endif


