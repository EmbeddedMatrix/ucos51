/*************************************************************************************************
												zip51 
 										 51оƬ�ϵ�ʵʱ�ں�
										 ����Ϣ���й����йصĺ���
										 ���ߣ��¶���
														2010��10��29��
�ļ�����			zip51_Q.c
��������������		7
��Ҫ�û��޸ĸ�����	��
*************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif






/*********************************************************************************************************
������		��	QCreate
����		��	����һ����Ϣ����    ��ʼ����Ϣ���ƿ��ֵ
���� 		��	��
����ֵ		��	��
ע��		��	���ֵ��OSInit�����е��ã��û�����ʹ���������
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
������		��	QPend
����		��	�еȴ��Ĵ�һ����Ϣ�����еõ�һ����Ϣ
���� 		��	timeout			�����Ϣ�����в�������Ϣָ���ĵȴ�ʱ��	
				err				���صĴ���ֵ		
����ֵ		��	ͨ��err���صģ�
						ERR_NOERR			0 ��Ϣ����ɹ�	������ʱ���ص���Ϣֵ��������
						ERR_TIMEOUT			��ʱ��־	
				ֱ�ӷ��صģ�
						���� errΪ ERR_NOERRʱ ���ص���Ϣֵ��������
ע��		��	���������ж��е��øú�������Ϊ�ж��ǲ����Եȴ���
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
������		��	QPost
����		��	��һ����Ϣ������Ϣ�����У����ڶ���
���� 		��	Msg					��Ϣֵ				
����ֵ		��	ERR_OVERFLOW		��Ϣ��������Ϣ����
				ERR_NOERR			0 ��ϢͶ�ݳɹ�
ע��		��	�������ж��е��øú���  ���������ȴ���Ϣ�����е���Ϣ��ô������������������л�
				������IntExit���л�
				����Ͷ��ֵΪ0����Ϣ		 ��ʱ��
								����������ڵȴ���Ϣ����������0ֵ��Ϣ��������ȼ�������
								���û�������ڵȴ�����������Ϣ�����Բ����Խ�0ֵ��Ϣ������Ϣ������
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
				Temp					= RdyTable_Prio[Temp]; 				///�õ��ȴ�������������ȼ�
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
		if (Msg == 0 )			 										///��������Ϣ
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
������		��	QPostOpt
����		��	��һ����Ϣ������Ϣ�����У����ڶ��ף���ͬ�����������֧�ֹ㲥��ʽͶ�� 
 				Ҳ����һ��ѡ���˹㲥��ʽ��ô���еĵȴ����񶼻�õ���Ϣ
���� 		��	Msg			��Ϣֵ
				Opt			Ͷ�ݷ�ʽ
							POST_BROADCAST				���͹㲥��Ϣ
							POST_NOBROADCAST			���ͷǹ㲥��Ϣ
							POST_FRONT					�Ӷ���Ͷ��
			   	����ʹ��ʱ����������ĺ���� ��ɸ������    ���ϣ���Ӷ�β�����ֲ��㲥��ô����POST_NOBROADCAST
����ֵ		��	ERR_OVERFLOW		��Ϣ��������Ϣ����
				ERR_NOERR			��ϢͶ�ݳɹ�
ע��		��	�������ж��е��øú���  ���������ȴ���Ϣ�����е���Ϣ��ô������������������л�
				������IntExit���л�
			    ����Ͷ��ֵΪ0����Ϣ	 0ֵ��Ϣ ����ʽ ͬ��
				�������ж��е��øú���
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
		if (Temp != 0 )														///�������ڵȴ���Ϣ����
		{
				if (Opt	& POST_BROADCAST )	 								///ѡ���Թ㲥��ʽ����
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
				else							  							///�ǹ㲥��ʽ����
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
		if(Msg == 0 )				///0ֵ��Ϣ�����������Ϣ����
		{
			EXIT_CRITICAL();
			return	ERR_NOERR;
		}
		if (Opt & POST_FRONT ) 			///��Ϣ������Ϣ���ж���
		{
			if (gMTCB.QOut	== gMTCB.QStart )
			{
				gMTCB.QOut	= gMTCB.QEnd;
			}
			* --gMTCB.QOut	= Msg;
		}
		else  							///��Ϣ������Ϣ���ж�β
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
������		��	QPostFront
����		��	��һ����Ϣ������Ϣ�����У���ͬ��������÷�����Ϣ���еĶ��� ��һ���������һ������
													QPostOpt�Ѿ� ���������������Ĺ����ˣ�
���� 		��	Msg			��Ϣֵ
����ֵ		��	ERR_OVERFLOW		��Ϣ��������Ϣ����
				ERR_NOERR			��ϢͶ�ݳɹ�
ע��		��	�������ж��е��øú���  ���������ȴ���Ϣ�����е���Ϣ��ô������������������л�
				������IntExit���л�
				����Ͷ��ֵΪ0����Ϣ		  0ֵ��Ϣ ����ʽ ͬ��
				��ʱ�Ķ��о��ж�ջ�Ĺ��� ���Ƚ����
				�������ж��е��øú���
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
			Temp					= RdyTable_Prio[Temp];					///�õ��ȴ�����������ȼ�
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
		if(Msg == 0 )				///0ֵ��Ϣ�����������Ϣ����
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
������		��	Qquery
����		��	��ѯ��ǰ��Ϣ�����е���Ϣ��Ŀ
���� 		��	��
����ֵ		��	��Ϣ��Ŀ
ע��		��	�������ж��е��øú���
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
������		��	QAcceptOpt
����		��	�޵ȴ��Ĵ���Ϣ�������һ����Ϣ
���� 		��	Opt���յĲ���ȡֵ��
						GET_NOCLEAR				�Ӷ��׽��ղ����
						GET_CLEAR				�Ӷ��׽��ս���������
				err 	���շ���ֵ
����ֵ		��	ͨ��err���أ�
						ERR_NOERR				�ɹ��õ���Ϣֵ���������ʱ�򷵻ص�Msg��������
						ERR_NOTACCESS			��Ϣ������
				ֱ�ӷ��صģ�
						��Ϣֵ
ע��		��	�������ж��е��øú���
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

