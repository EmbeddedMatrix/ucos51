/*************************************************************************************************
												zip51 
 										 51芯片上的实时内核
										 全局数据类型及数据结构定义
										 作者：孤独剑
														2010年10月20日
文件名：			zip51_numtype.h
包含函数个数：		无
用户需修改内容：	无
*************************************************************************************************/


/*************************************************************************************************
注意51与x86的不同点：
1 ）大小端不同
2 ）堆栈增长方向不同
3 ）P标志位意义不同
4 ）x86上中断控制位在标志寄存器上，而51则是在另外的标志IE上（这个不同很重要） 故需要在切换时开中断
*************************************************************************************************/
typedef					unsigned		char				byte;
typedef					unsigned		int					uint16;
typedef					unsigned		long	int			uint32;

typedef					byte			idata				StkType;				
///堆栈定义  这里让堆栈使用 idata(片内RAM),间接寻址。

typedef					byte			data				ErrType;
///让每一个错误值都是用 data 区域内存



#if				TIME_CNT_EN
#if				TIME_CNT_LENGTH			== 		1	
typedef					byte								CntType;
#endif
#if				TIME_CNT_LENGTH			== 		2			
typedef					uint16								CntType;
#endif

#if				TIME_CNT_LENGTH			== 		4			
typedef					uint32								CntType;
#endif
#endif

#if				IDLE_CNT_EN
#if				IDLE_CNT_LENGTH			==		1
typedef					byte								IdleCntType;
#endif
#if				IDLE_CNT_LENGTH			== 		2
typedef					uint16								IdleCntType;
#endif
#if				IDLE_CNT_LENGTH			==		4
typedef					uint32								IdleCntType;
#endif
#endif


#if				SLEEP_LENGTH			== 		1
typedef					byte								SleepType;
#endif
#if				SLEEP_LENGTH			== 		2
typedef					uint16								SleepType;
#endif
#if				SLEEP_LENGTH			==		4
typedef					uint32								SleepType;
#endif



#if				MBOX_EN	||	MESS_Q_EN
#if				MESSBOXLENGTH			==		1
typedef					byte								MBoxType;
#endif
#if				MESSBOXLENGTH			==		2
typedef					uint16								MBoxType;
#endif
#if				MESSBOXLENGTH			==		4
typedef					uint32								MBoxType;
#endif
#endif



#if				MESS_Q_EN
#if				MESS_Q_POSI				== 1
typedef			MBoxType		idata						Mess_Q_Type;
///如果这个宏定义为1则放在 idata 上采用间接寻址。
#endif
#if				MESS_Q_POSI				== 0
typedef			MBoxType		 data						Mess_Q_Type;
#endif
#endif


#if				MESS_Q_EN
typedef		struct	MTCB_{											///消息队列控制块
			byte			MessNum;								///当前消息数目
			byte			WaitTable;								///等待消息队列的列表
			Mess_Q_Type *	QStart;									///消息队列首指针
			Mess_Q_Type	*	QOut;									///当前队首指针 （时刻有效）
			Mess_Q_Type	*	QIn;									///当前对位指针 （时刻无效）
			Mess_Q_Type	*	QEnd;									///指向消息数组的下一个元素
}MTCB;
#endif




#define			ENTER_CRITICAL()			EA		= 0		///关闭总中断
#define			EXIT_CRITICAL()				EA		= 1		///打开总中断












