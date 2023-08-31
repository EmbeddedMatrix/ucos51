/*************************************************************************************************
												zip51 
 										 51оƬ�ϵ�ʵʱ�ں�
										 ȫ���������ͼ����ݽṹ����
										 ���ߣ��¶���
														2010��10��20��
�ļ�����			zip51_numtype.h
��������������		��
�û����޸����ݣ�	��
*************************************************************************************************/


/*************************************************************************************************
ע��51��x86�Ĳ�ͬ�㣺
1 ����С�˲�ͬ
2 ����ջ��������ͬ
3 ��P��־λ���岻ͬ
4 ��x86���жϿ���λ�ڱ�־�Ĵ����ϣ���51����������ı�־IE�ϣ������ͬ����Ҫ�� ����Ҫ���л�ʱ���ж�
*************************************************************************************************/
typedef					unsigned		char				byte;
typedef					unsigned		int					uint16;
typedef					unsigned		long	int			uint32;

typedef					byte			idata				StkType;				
///��ջ����  �����ö�ջʹ�� idata(Ƭ��RAM),���Ѱַ��

typedef					byte			data				ErrType;
///��ÿһ������ֵ������ data �����ڴ�



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
///�������궨��Ϊ1����� idata �ϲ��ü��Ѱַ��
#endif
#if				MESS_Q_POSI				== 0
typedef			MBoxType		 data						Mess_Q_Type;
#endif
#endif


#if				MESS_Q_EN
typedef		struct	MTCB_{											///��Ϣ���п��ƿ�
			byte			MessNum;								///��ǰ��Ϣ��Ŀ
			byte			WaitTable;								///�ȴ���Ϣ���е��б�
			Mess_Q_Type *	QStart;									///��Ϣ������ָ��
			Mess_Q_Type	*	QOut;									///��ǰ����ָ�� ��ʱ����Ч��
			Mess_Q_Type	*	QIn;									///��ǰ��λָ�� ��ʱ����Ч��
			Mess_Q_Type	*	QEnd;									///ָ����Ϣ�������һ��Ԫ��
}MTCB;
#endif




#define			ENTER_CRITICAL()			EA		= 0		///�ر����ж�
#define			EXIT_CRITICAL()				EA		= 1		///�����ж�












