/**********************************************************************************************************************
														zip51
												适合于在51芯片上的实时内核
												作者：孤独剑
												时间：2010 10  24
												包含本工程相关的一些宏定义   数据类型定义  全局变量定义 
												以及函数声明
文件名			zip51.h
**********************************************************************************************************************/
#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif

/*****************************************************************************************
									任务状态有关的宏
*****************************************************************************************/
#define			TASK_RDY					0x00				///任务就绪
#define			TASK_SUSPEND				0x20				///任务被挂起
#define			TASK_WAITSEM				0x01				///任务在等待信号量
#define			TASK_WAITMUTEX				0x02				///任务在等待互斥体
#define			TASK_WAITMBOX				0x04				///任务在等待消息邮箱
#define			TASK_WAITQ					0x08				///任务在等待消息队列
/*****************************************************************************************
									等待条件宏
*****************************************************************************************/
#define			GET_WAIT					0x01				///在得到一个内核数据块时等待
#define			GET_NO_WATI					0x02				///在得到一个内核数据块时不等待
#define			POST_BROADCAST				0x04				///发送广播消息
#define			POST_NOBROADCAST			0x08				///发送非广播消息
#define			GET_CLEAR					0x10				///得到并清除
#define			GET_NOCLEAR					0x20				///得到并不清除
#define			POST_FRONT					0x40				///以广播形式发送


/*****************************************************************************************
									返回错误值宏
*****************************************************************************************/
#define			ERR_NOERR						0x00
#define			ERR_ALREADYEXIT					0x01
#define			ERR_NOTEXIT						0x02
#define			ERR_OVERFLOW					0x04
#define			ERR_INVALIDARG					0x08
#define			ERR_TIMEOUT						0x10
#define			ERR_NOTACCESS					0x20



typedef		struct	OsTcb_{								///任务控制块的结构
			StkType					*StkPtr;			//指向栈顶的指针
			SleepType				TimeDly;			//任务延时数。
			#if		MBOX_EN || MESS_Q_EN
			MBoxType				Msg;				//消息
			#endif
			byte					TaskStat;			//任务状态；
}OS_TCB;



#if			SEM_EN
typedef		byte					WaitSem;
//等待第一个信号量的任务
#endif

#if			MUTEX_EN
typedef		byte					WaitMutex;
#endif

#if			MBOX_EN
typedef		byte					WaitMBox;
#endif



/*********************************************************************************************************
											全局变量定义
*********************************************************************************************************/
OS_EXT  		bit									OSRunning;
///系统是否在运行

OS_EXT			volatile	byte					bdata				gSysPriHighRdy	;
///系统运行时每一刻的	就绪了的最高优先级

OS_EXT			volatile	byte					bdata				gSysPriCur	;
///系统运行时每一刻的	当前优先级

OS_EXT			volatile	byte					data				gLockNest	;
///调度器锁定层数

OS_EXT			volatile	byte					data				gIntNest	;
///中断嵌套层数

OS_EXT			volatile	OS_TCB					data				gTCB[5]		;	
///任务控制块 4个一般任务 和一个空闲任务



OS_EXT			volatile	byte					data				RdyTable 	;	
//  0x1				任务就绪块（低四位）



#if						TIME_CNT_EN
OS_EXT			volatile	CntType					data				TimeCnt	;	
#endif	
//TIME_CNT_LENGTH	系统自启动以来运行的时间 以ms为单位


#if						IDLE_CNT_EN
OS_EXT			volatile	IdleCntType				data				IdleCnt	;
#endif
//				空闲任务计数 （这个可以 省略）




/*********************************************************************************************************
											与信号量有关的全局变量
*********************************************************************************************************/
#if				SEM_EN
OS_EXT			volatile	byte					data				gSem[2]		;
OS_EXT			volatile	WaitSem					data				gWaitSem[2] ;
#endif


/*********************************************************************************************************
											与互斥型信号量有关的全局变量
*********************************************************************************************************/
#if				MUTEX_EN
OS_EXT			volatile	byte					data				gMutex ;
OS_EXT			volatile	WaitMutex				data				gWaitMutex[4]	;
#endif



/*********************************************************************************************************
											与消息邮箱有关的全局变量
*********************************************************************************************************/
#if				MBOX_EN
OS_EXT			volatile	MBoxType				data				gMBox[MBOX_NUM];
OS_EXT			volatile	WaitMBox				data				gWaitMBox[MBOX_NUM]	;
#endif




/*********************************************************************************************************
											与消息队列有关的全局变量
*********************************************************************************************************/
#if				MESS_Q_EN
OS_EXT			volatile	MTCB					data				gMTCB;							///消息队列控制块			
OS_EXT				Mess_Q_Type											gMBoxArray[MAX_MESS_LEN];		///消息队列数组
#endif


///映射表
extern			byte					code				RdyTable_Prio[16] ;
extern			byte					code				Prio_RdyTable[4] ;





/*********************************************************************************************************
											全局函数声明
*********************************************************************************************************/ 
/*	task		指向任务的一个code指针
	ptos		栈顶指针			
	Prio		任务优先级
	经过多次测试发现只有这样定义编译器才会将task作为一个任务指针来对待！！*/	
void			TaskCreate	(uint16	task,StkType	*ptos,byte	Prio )	small;										
///创建一个任务





#if				TASK_SUSPEND_EN	
void			Suspend (byte Prio ) small;
/*功能：将一个任务挂起 Prio为任务参数
可以挂起自己但是此时必须由别的任务恢复*/

void			Resume  (byte Prio ) small;	
/*功能：将一个挂起的任务恢复
恢复自己没意义，因为能调用这个函数肯定没被挂起
Prio是任务的优先级*/		
#endif




void			Sleep (SleepType	ticks )  small;		
#if				WAKE_UP_EN										
void			WakeUp (byte	Prio ) 	small;	
#endif
/*唤醒一个睡眠中的任务，但如果任务已经被挂起则无法唤醒
一个任务被挂起绝对不可能再去睡眠  但一个任务在睡眠状态可以被挂起			
如果一个任务同时被挂起和睡眠则可以通过分别调用WakeUp();Resume()
函数使任务开始运行，调用顺序不限 */	





#if				TIME_CNT_EN		&&		TIME_GET_EN
CntType			TimeGet (void ) small;
#endif

#if				TIME_CNT_EN		&&		TIME_SET_EN
void			TimeSet (CntType Time) small;
#endif





void			TimeTick (void )  small;																			///时间节拍函数




/*
*********************************************************************************************************
*                                             各式函数声明
*********************************************************************************************************
*/
void			OsInit (void )  small;														///系统初始化

void			OSIntInit()small;															///系统中断的初始化。

void			IntEnter (void ) small ;													///从进入中断中的函数
 
void			IntExit (void )  small;														///从中断中退出的函数



#if				SCHED_LOCK_EN
void			SchedLock (void) small;
void			SchedUnLock(void) small;
#endif





void			OSStart	(void )  small;														///开启多任务环境
void			OSSched (void )  small;														//任务调度程序
void			TaskIdle (void )  small;													///空闲的任务。
void			IntSw (void )  small;														///在中断中的任务切换
void			TaskSw (void )  small;														///在任务过程中 任务间的切换






/*********************************************************************************************************
									以下是与信号量管理有关的函数
*********************************************************************************************************/
#if				SEM_EN
byte			SemAccept  (byte	i ) small;
byte			SemPend	   (byte	i ,SleepType		timeout) small;
byte			SemPost    (byte	i ) small;
byte			SemCreate   (byte	i ,byte	Init) small;
#endif		


/*********************************************************************************************************
									以下是与互斥型信号量管理有关的函数
*********************************************************************************************************/
#if				MUTEX_EN
byte			MutexCreate (byte	i ) small;
byte			MutexPend   (byte	i,SleepType			timeout  ) small;
byte			MutexPost	(byte	i ) small;
byte			MutexAccept (byte	i ) small;
#endif


/*********************************************************************************************************
									以下是与消息邮箱有关的函数
*********************************************************************************************************/
#if				MBOX_EN
byte			MBoxCreate	(byte	i ,MBoxType	Msg ) small;
byte			MBoxPost	(byte	i ,MBoxType	Msg ) small;
MBoxType		MBoxAcceptOpt(byte	i ,byte	Opt,ErrType	*err ) small;
MBoxType		MBoxPend	(byte	i ,SleepType	timeout ,ErrType	*err ) small;

#if				MBOX_POSTOPT_EN
byte			MBoxPostOpt (byte	i ,MBoxType  Msg ,byte	Opt )small;
#endif

#endif




/*********************************************************************************************************
									以下是与消息队列有关的函数
*********************************************************************************************************/
#if				MESS_Q_EN
void			QCreate (void ) small;										///消息队列创建
MBoxType		QPend (SleepType	timeout ,ErrType	*err ) small;		///等待从队列中得到一个消息
byte			QPost (MBoxType		Msg )small;								///向消息队列总投递一条消息
byte			Qquery(void) small;											///查询消息队列中有多少个消息
MBoxType		QAcceptOpt(byte	Opt ,ErrType	*err)small;					///不等待的得到一条消息
#if				Q_POST_OPT_EN
byte			QPostOpt (MBoxType	Msg ,byte	Opt)small;					///向消息队列总投递消息（允许广播式投递）
#endif
#if				Q_POST_FRONT_EN
byte			QPostFront (MBoxType Msg )small ;							///将发送的消息插入到队列的最前面
#endif
#endif















