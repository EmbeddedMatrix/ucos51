/**********************************************************************************************************************
														zip51
												�ʺ�����51оƬ�ϵ�ʵʱ�ں�
												���ߣ��¶���
												ʱ�䣺2010 10  24
												������������ص�һЩ�궨��   �������Ͷ���  ȫ�ֱ������� 
												�Լ���������
�ļ���			zip51.h
**********************************************************************************************************************/
#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif

/*****************************************************************************************
									����״̬�йصĺ�
*****************************************************************************************/
#define			TASK_RDY					0x00				///�������
#define			TASK_SUSPEND				0x20				///���񱻹���
#define			TASK_WAITSEM				0x01				///�����ڵȴ��ź���
#define			TASK_WAITMUTEX				0x02				///�����ڵȴ�������
#define			TASK_WAITMBOX				0x04				///�����ڵȴ���Ϣ����
#define			TASK_WAITQ					0x08				///�����ڵȴ���Ϣ����
/*****************************************************************************************
									�ȴ�������
*****************************************************************************************/
#define			GET_WAIT					0x01				///�ڵõ�һ���ں����ݿ�ʱ�ȴ�
#define			GET_NO_WATI					0x02				///�ڵõ�һ���ں����ݿ�ʱ���ȴ�
#define			POST_BROADCAST				0x04				///���͹㲥��Ϣ
#define			POST_NOBROADCAST			0x08				///���ͷǹ㲥��Ϣ
#define			GET_CLEAR					0x10				///�õ������
#define			GET_NOCLEAR					0x20				///�õ��������
#define			POST_FRONT					0x40				///�Թ㲥��ʽ����


/*****************************************************************************************
									���ش���ֵ��
*****************************************************************************************/
#define			ERR_NOERR						0x00
#define			ERR_ALREADYEXIT					0x01
#define			ERR_NOTEXIT						0x02
#define			ERR_OVERFLOW					0x04
#define			ERR_INVALIDARG					0x08
#define			ERR_TIMEOUT						0x10
#define			ERR_NOTACCESS					0x20



typedef		struct	OsTcb_{								///������ƿ�Ľṹ
			StkType					*StkPtr;			//ָ��ջ����ָ��
			SleepType				TimeDly;			//������ʱ����
			#if		MBOX_EN || MESS_Q_EN
			MBoxType				Msg;				//��Ϣ
			#endif
			byte					TaskStat;			//����״̬��
}OS_TCB;



#if			SEM_EN
typedef		byte					WaitSem;
//�ȴ���һ���ź���������
#endif

#if			MUTEX_EN
typedef		byte					WaitMutex;
#endif

#if			MBOX_EN
typedef		byte					WaitMBox;
#endif



/*********************************************************************************************************
											ȫ�ֱ�������
*********************************************************************************************************/
OS_EXT  		bit									OSRunning;
///ϵͳ�Ƿ�������

OS_EXT			volatile	byte					bdata				gSysPriHighRdy	;
///ϵͳ����ʱÿһ�̵�	�����˵�������ȼ�

OS_EXT			volatile	byte					bdata				gSysPriCur	;
///ϵͳ����ʱÿһ�̵�	��ǰ���ȼ�

OS_EXT			volatile	byte					data				gLockNest	;
///��������������

OS_EXT			volatile	byte					data				gIntNest	;
///�ж�Ƕ�ײ���

OS_EXT			volatile	OS_TCB					data				gTCB[5]		;	
///������ƿ� 4��һ������ ��һ����������



OS_EXT			volatile	byte					data				RdyTable 	;	
//  0x1				��������飨����λ��



#if						TIME_CNT_EN
OS_EXT			volatile	CntType					data				TimeCnt	;	
#endif	
//TIME_CNT_LENGTH	ϵͳ�������������е�ʱ�� ��msΪ��λ


#if						IDLE_CNT_EN
OS_EXT			volatile	IdleCntType				data				IdleCnt	;
#endif
//				����������� ��������� ʡ�ԣ�




/*********************************************************************************************************
											���ź����йص�ȫ�ֱ���
*********************************************************************************************************/
#if				SEM_EN
OS_EXT			volatile	byte					data				gSem[2]		;
OS_EXT			volatile	WaitSem					data				gWaitSem[2] ;
#endif


/*********************************************************************************************************
											�뻥�����ź����йص�ȫ�ֱ���
*********************************************************************************************************/
#if				MUTEX_EN
OS_EXT			volatile	byte					data				gMutex ;
OS_EXT			volatile	WaitMutex				data				gWaitMutex[4]	;
#endif



/*********************************************************************************************************
											����Ϣ�����йص�ȫ�ֱ���
*********************************************************************************************************/
#if				MBOX_EN
OS_EXT			volatile	MBoxType				data				gMBox[MBOX_NUM];
OS_EXT			volatile	WaitMBox				data				gWaitMBox[MBOX_NUM]	;
#endif




/*********************************************************************************************************
											����Ϣ�����йص�ȫ�ֱ���
*********************************************************************************************************/
#if				MESS_Q_EN
OS_EXT			volatile	MTCB					data				gMTCB;							///��Ϣ���п��ƿ�			
OS_EXT				Mess_Q_Type											gMBoxArray[MAX_MESS_LEN];		///��Ϣ��������
#endif


///ӳ���
extern			byte					code				RdyTable_Prio[16] ;
extern			byte					code				Prio_RdyTable[4] ;





/*********************************************************************************************************
											ȫ�ֺ�������
*********************************************************************************************************/ 
/*	task		ָ�������һ��codeָ��
	ptos		ջ��ָ��			
	Prio		�������ȼ�
	������β��Է���ֻ����������������ŻὫtask��Ϊһ������ָ�����Դ�����*/	
void			TaskCreate	(uint16	task,StkType	*ptos,byte	Prio )	small;										
///����һ������





#if				TASK_SUSPEND_EN	
void			Suspend (byte Prio ) small;
/*���ܣ���һ��������� PrioΪ�������
���Թ����Լ����Ǵ�ʱ�����ɱ������ָ�*/

void			Resume  (byte Prio ) small;	
/*���ܣ���һ�����������ָ�
�ָ��Լ�û���壬��Ϊ�ܵ�����������϶�û������
Prio����������ȼ�*/		
#endif




void			Sleep (SleepType	ticks )  small;		
#if				WAKE_UP_EN										
void			WakeUp (byte	Prio ) 	small;	
#endif
/*����һ��˯���е����񣬵���������Ѿ����������޷�����
һ�����񱻹�����Բ�������ȥ˯��  ��һ��������˯��״̬���Ա�����			
���һ������ͬʱ�������˯�������ͨ���ֱ����WakeUp();Resume()
����ʹ����ʼ���У�����˳���� */	





#if				TIME_CNT_EN		&&		TIME_GET_EN
CntType			TimeGet (void ) small;
#endif

#if				TIME_CNT_EN		&&		TIME_SET_EN
void			TimeSet (CntType Time) small;
#endif





void			TimeTick (void )  small;																			///ʱ����ĺ���




/*
*********************************************************************************************************
*                                             ��ʽ��������
*********************************************************************************************************
*/
void			OsInit (void )  small;														///ϵͳ��ʼ��

void			OSIntInit()small;															///ϵͳ�жϵĳ�ʼ����

void			IntEnter (void ) small ;													///�ӽ����ж��еĺ���
 
void			IntExit (void )  small;														///���ж����˳��ĺ���



#if				SCHED_LOCK_EN
void			SchedLock (void) small;
void			SchedUnLock(void) small;
#endif





void			OSStart	(void )  small;														///���������񻷾�
void			OSSched (void )  small;														//������ȳ���
void			TaskIdle (void )  small;													///���е�����
void			IntSw (void )  small;														///���ж��е������л�
void			TaskSw (void )  small;														///����������� �������л�






/*********************************************************************************************************
									���������ź��������йصĺ���
*********************************************************************************************************/
#if				SEM_EN
byte			SemAccept  (byte	i ) small;
byte			SemPend	   (byte	i ,SleepType		timeout) small;
byte			SemPost    (byte	i ) small;
byte			SemCreate   (byte	i ,byte	Init) small;
#endif		


/*********************************************************************************************************
									�������뻥�����ź��������йصĺ���
*********************************************************************************************************/
#if				MUTEX_EN
byte			MutexCreate (byte	i ) small;
byte			MutexPend   (byte	i,SleepType			timeout  ) small;
byte			MutexPost	(byte	i ) small;
byte			MutexAccept (byte	i ) small;
#endif


/*********************************************************************************************************
									����������Ϣ�����йصĺ���
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
									����������Ϣ�����йصĺ���
*********************************************************************************************************/
#if				MESS_Q_EN
void			QCreate (void ) small;										///��Ϣ���д���
MBoxType		QPend (SleepType	timeout ,ErrType	*err ) small;		///�ȴ��Ӷ����еõ�һ����Ϣ
byte			QPost (MBoxType		Msg )small;								///����Ϣ������Ͷ��һ����Ϣ
byte			Qquery(void) small;											///��ѯ��Ϣ�������ж��ٸ���Ϣ
MBoxType		QAcceptOpt(byte	Opt ,ErrType	*err)small;					///���ȴ��ĵõ�һ����Ϣ
#if				Q_POST_OPT_EN
byte			QPostOpt (MBoxType	Msg ,byte	Opt)small;					///����Ϣ������Ͷ����Ϣ������㲥ʽͶ�ݣ�
#endif
#if				Q_POST_FRONT_EN
byte			QPostFront (MBoxType Msg )small ;							///�����͵���Ϣ���뵽���е���ǰ��
#endif
#endif















