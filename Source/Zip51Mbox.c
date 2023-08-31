/**********************************************************************************************************************
														zip51
												�ʺ�����51оƬ�ϵ�ʵʱ�ں�
												���ߣ��¶���
												ʱ�䣺2010 10  27
												��������������Ϣ��������йصĺ���
�ļ���					zip51_mbox.c
����������				5
��Ҫ�û��޸ĵĸ�����	��
������Ϣ��������������ļ��е� MBOX_NUM ����
**********************************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif





/*********************************************************************************************************
������		��	MBoxCreate
����		��	����һ����Ϣ����
���� 		��	i 		��		�ڼ�����Ϣ���� iֵ���ܴ���MBOX_NUM	
				Msg		:		��ʼ����Ϣֵ����������0				
����ֵ		��	ERR_NOERR 		0 �����ɹ�
				ERR_INVALIDARG	������Ч
ע��		��	��Ҫ���ж��е��øú���	 MsgΪ0��ʧ��			
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
������		��	MBoxPend
����		��	�ȴ��õ�һ����Ϣ����
���� 		��	i 				���ڼ�����Ϣ���� iֵ���ܴ��� MBOX_NUM
				timeout			�����Ϣ������û����Ϣ  ��ָ���ĵȴ�ʱ��		
				err				ָ��ָ��һ������ֵ ���д������				
����ֵ		��	ͨ��err���صģ�
						ERR_NOERR 		0 ����ɹ�
						ERR_INVALIDARG	������Ч
						ERR_TIMEOUT		�ȴ���ʱ
				ֱ�ӷ��صģ�
						��Ϣֵ   ������Ϊ0����Ч
ע��		��	��Ҫ���ж��е��øú���
				���øú���ʱӦ������һ����idata�ڴ��е�һ�ֽ����ݲ�����ַ���ݹ�ȥ ע�������ȫ�ֱ���
				������س�ʱ����һ���ǵȴ�ʱ�䵽��û�еõ���Ϣ���п�����
				������һ���������Post������һ������Ϣ ����ϢֵΪ0 ��
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

		Temp			  		    = 0x08;				///�����ٴμ�������ΪKeil��Ծֲ������������Ż�
		Temp			   		  >>= gSysPriCur;		///����Tempֵ�п����ڿ��ж��ڼ䱻�޸�
		gWaitMBox[i]			   &= ~Temp;
 		gTCB[gSysPriCur	].TaskStat	= TASK_RDY ;
		*err				 		= ERR_TIMEOUT;
		EXIT_CRITICAL();
		return	0;		
}
#endif




/*********************************************************************************************************
������		��	MBoxPost
����		��	����Ϣ�����з���һ����Ϣ
���� 		��	i 				���ڼ�����Ϣ���� iֵ���ܴ��� MBOX_NUM
				Msg				ָ����Ҫ���͵���Ϣ	��������� 0
����ֵ		��	ERR_NOERR 		0 ���ͳɹ�
				ERR_INVALIDARG	������Ч
				ERR_OVERFLOW	��Ϣ�������Ѿ�������Ϣ
ע��		��	�ж��п��Ե��øú��������ڸú����в����л� һֱ�ȵ�IntExit�����Ż��л�
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

		if ( gMBox[i] )														///�Ѿ�����Ϣ�����������еȴ���������
		{
				EXIT_CRITICAL();
				return	ERR_OVERFLOW;
		}
		
		Temp		= gWaitMBox[i];
		if (Temp !=  0 )
		{
			Temp							= RdyTable_Prio[Temp];					///�õ��ȴ�����������ȼ�
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
������		��	MBoxPostOpt
����		��	��ѡ�������Ϣ�����з���һ����Ϣ
���� 		��	i 				���ڼ�����Ϣ���� iֵ���ܴ��� MBOX_NUM
				Msg				ָ����Ҫ���͵���Ϣ		��������� 0
				opt				���͵ķ�ʽȡֵ
								POST_BROADCAST		�Թ㲥��ʽ�������еȴ�����������񶼻�õ���Ϣ������
								POST_NOBROADCAST	�����Թ㲥�ķ�ʽ���ͣ�����������ȼ������õ���Ϣ
����ֵ		��	ERR_NOERR 		0 ���ͳɹ�
				ERR_INVALIDARG	������Ч
				ERR_OVERFLOW	��Ϣ�������Ѿ�������Ϣ
ע��		��	�ж��п��Ե��øú��������ڸú����в����л� һֱ�ȵ�IntExit�����Ż��л�
				������Msg������0,���Ϊ0��ô��
						��������������˵�����������õ���ʱ��־
						�������ͬʱָ��optΪ POST_BROADCAST ��ô�൱��ɾ���������Ϣ����
						�����еȴ�������״̬������ ȥ���ȴ���Ϣ�����־  �ȴ��б�Ϊ0 �������õ�����Ϣ�����ֵΪ0 ��
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
		if (Temp !=  0 )									///�������ڵȴ�
		{
			if ( (Opt	& POST_BROADCAST) != 0 )						///�㲥
			{
					while ( Temp )
					{
						Temp					= RdyTable_Prio[Temp];					///�õ��ȴ�����������ȼ�
						gSysPriHighRdy			= Temp;
						Temp					= Prio_RdyTable[Temp];
						gWaitMBox[i]		   				   &= ~Temp;				///������ȴ�����ȥ������
						gTCB[gSysPriHighRdy	].TimeDly 			= 0;
						gTCB[gSysPriHighRdy	].Msg				= Msg;
						if ( (gTCB[gSysPriHighRdy].TaskStat		&= ~TASK_WAITMBOX ) == TASK_RDY )
						{
								RdyTable	|= Prio_RdyTable[gSysPriHighRdy	];
						}
						Temp			= gWaitMBox[i];
					}
			}
			else																///�ǹ㲥
			{
				Temp					= RdyTable_Prio[Temp];					///�õ��ȴ�����������ȼ�
				gSysPriHighRdy			= Temp;
				Temp					= Prio_RdyTable[Temp];
				gWaitMBox[i]		   &= ~Temp;								///������ȴ�����ȥ������
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
������		��	MBoxAcceptOpt
����		��	�޵ȴ��Ĵ���Ϣ�����еõ���Ϣ
���� 		��	i 				���ڼ�����Ϣ���� iֵ���ܴ��� MBOX_NUM	
				Opt				��������ķ�ʽȡֵ��
								GET_CLEAR		��Ϣ����������Ϣ�õ���Ϣ֮��ԭ������Ϣ���
								GET_NOCLEAR		��Ϣ����������Ϣ�õ���Ϣ֮�󲻽�ԭ������Ϣ���
����ֵ		��	ͨ��err���� ��
								ERR_NOERR 		0 �ɹ�   ��ʱ����ֵ��������ֵ
								ERR_INVALIDARG  ������Ч	
								ERR_NOTACCESS	���䲻����
				�����0��˵��û�еõ���Ϣ
				���������˵���Ѿ��õ���Ϣ  ����ֵ������Ϣ
ע��		��	�ж��п��Ե��øú���
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


