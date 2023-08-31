/*************************************************************************************************
												zip51 
 										 51оƬ�ϵ�ʵʱ�ں�
										 ���ź��������йصĺ���
										 ���ߣ��¶���
														2011��4��12��   �޸�
�ļ�����			zip51_Sem.c
��������������		4
��Ҫ�û��޸ĸ�����	��
*************************************************************************************************/
#ifndef  ZIP51_FILE
#include "includes.h"
#endif


/*********************************************************************************************************
���һ������A������Pend�ຯ�� ���������е��ź��� �������ź��� ��Ϣ����		��Ϣ����  ��ָ���˵ȴ���ʱʱ��
timeout�����ָ����ʱ��֮ǰ����һ������B������Ӧ��Post������� 
1��B���ȼ�����ôA���������� ���ҵȴ�ʱ�޼�Ϊ0ֱ��A����CPUʹ��Ȩ����û�б�B���߼�������������B�Ŵ�Post��������
2��B���ȼ�����ôA������������
�����ʱ�޵�֮����Ȼû������Post  ��ôA���õ�һ��timeout��־  ��ʶ�ȴ���ʱ
���A����Pendʱָ����ʱʱ��Ϊ0�������ڵȴ�ֱ���ȴ����ź�������Ϊֹ
*********************************************************************************************************/


/*********************************************************************************************************
������		��	SemAccept
����		��	�޵ȴ�������һ���ź���
���� 		��	i 		���ڼ����ź��� zip��֧��2���ź���	��iȡֵ1��ʾ��һ����			
����ֵ		��	0  ERR_NOERR��ʾ�ɹ�  ��0ֵ��ʾʧ��				
ע��		��	ע��Է���ֵҪ��飬�����ֵ��Ϊ0˵���ź�����Ч
				�жϷ����ӳ�����Ե���SemAccept���������ǲ����Ƽ�������жϷ���������ź�������Ϊ�ź���
				һ���������񼶡����ȷʵ��Ҫ��������жϷ�������д����ź������жϷ����ӳ���ֻ���Ե���SemPost
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
������		��	SemPend
����		��	����ź���������ֱ�ӷ��� �޹���������ȴ�����һ���ź���
���� 		��	i 		���ڼ����ź��� zip��֧��2���ź��� (i ����1��2 ��
				timeout	��������ź�����������ô�ȴ����				
����ֵ		��	ERR_NOERR 		0 �ɹ��Ѿ����뵽�ź���
				ERR_TIMEOUT 	�ȴ�ʱ�䳬ʱ	
				ERR_INVALIDARG	��������
ע��		��	��Ҫ���ж��е��øú���
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

			OSSched ();									///����һ�������л�

			ENTER_CRITICAL();
			if (gTCB[gSysPriCur	].TaskStat 	& TASK_WAITSEM )
			{
					Temp			     = 0x08;				///�����ٴμ�������ΪKeil��Ծֲ������������Ż�
					Temp			   >>= gSysPriCur					;		///����Tempֵ�п����ڿ��ж��ڼ䱻�޸�
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
������		��	SemPost
����		��	����һ���ź���
���� 		��	i 		���ڼ����ź��� zip��֧��2���ź���							
����ֵ		��	ERR_NOERR 		�Ѿ����ͳ��ź���
				ERR_OVERFLOW 	�ź�������	
				ERR_INVALIDARG 	��������	
ע��		��	�������ж��е��øú�������ͨ����ͨ�� ���������ж��е��øú��������л�����
				һֱ��IntExit��ʱ��Ż��л�
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
			 	Temp			= RdyTable_Prio[Temp];			///�õ��ȴ�����������ȼ�
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
������		��	SemCreate
����		��	����һ���ź���
���� 		��	i 				���ڼ����ź��� zip��֧��2���ź��� iȡֵ1����2
				Init			���ź�����ʼֵȡ0��255							
����ֵ		��	ERR_NOERR		�����ɹ�
				ERR_INVALIDARG	��������
ע��		��	���iȡֵ�Ƿ��򲻴����ź���
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


