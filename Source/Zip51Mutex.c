/**********************************************************************************************************************
														zip51
												�ʺ�����51оƬ�ϵ�ʵʱ�ں�
												���ߣ��¶���
												ʱ�䣺2010 10  24
												�����������뻥�����ź����йصĺ���
�ļ���					zip51_mutex.c
����������				4
��Ҫ�û��޸ĵĸ�����	��
����û�н�����ȼ���ת���� ���û�ʹ��ʱ��Ҫע�����ķ������ȼ������������ȼ���ת
�����жϷ����ܷ��ʻ������ź������Ա��ļ������к������������жϷ�������е���
**********************************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif




/*********************************************************************************************************
������		��	MutexCreate
����		��	����һ���������ź��� 
���� 		��	i 		���ڼ����ź��� zip��֧��4���ź��� (i ȡֵ1��4 ��								
����ֵ		��	ERR_NOERR 		�����ɹ�
				ERR_INVALIDARG	������Ч
				ERR_ALREADYEXIT	�ź����Ѿ�����
ע��		��	��Ҫ���ж��е��øú��� ,�ոմ���ʱ�Ѿ���һ���ź�ֵ��
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
������		��	MutexPend
����		��	�ȴ��õ�һ���������ź���
���� 		��	i 		���ڼ����ź��� zip��֧��4���ź��� (i ȡֵ1��4 ��
				timeout	����û������ź��������õȴ�ʱ��								
����ֵ		��	ERR_NOERR 		����ɹ�
				ERR_INVALIDARG	������Ч
				ERR_NOTEXIT		�û������ź���������
				ERR_TIMEOUT		�ȴ���ʱ
ע��		��	��Ҫ���ж��е��øú���
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
					Temp			     = 0x08;				///�����ٴμ�������ΪKeil��Ծֲ������������Ż�
					Temp			   >>= gSysPriCur					;		///����Tempֵ�п����ڿ��ж��ڼ䱻�޸�
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
������		��	MutexPost
����		��	�ͷ�һ���������ź���
���� 		��	i 		���ڼ����ź��� zip��֧��4���ź��� (i ȡֵ1��4 ��												
����ֵ		��	ERR_NOERR 		0 �ͷųɹ�
				ERR_INVALIDARG	������Ч	
				ERR_NOTEXIT		�ź���������
				ERR_OVERFLOW	�ظ��ͷŻ������ź���
ע��		��	��Ҫ���ж��е��øú���
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
			 	Temp				 = RdyTable_Prio[gWaitMutex[i]];			///˵���������ڵȴ� �õ��ȴ�����������ȼ�
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
������		��	MutexAccept
����		��	�޵ȴ��ĵõ�һ���ź���
���� 		��	i 		���ڼ����ź��� zip��֧��4���ź��� (i ȡֵ1��4 ��												
����ֵ		��	ERR_NOERR 		0 �Ѿ��ɹ��õ�
				ERR_INVALIDARG	������Ч	
				ERR_NOTEXIT		�ź���������
				ERR_NOTACCESS	�ź���������	
ע��		��	��Ҫ���ж��е��øú���
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