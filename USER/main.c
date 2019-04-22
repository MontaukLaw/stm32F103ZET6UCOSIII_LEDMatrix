#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "malloc.h"
#include "upcom.h"

////////////////////////�¼���־��//////////////////////////////

//����������DEBUG������0
#define DEBUG  1
#define FRONT_FEEDER_READY   1
#define DATAMSG_Q_NUM	10	//�������ݵ���Ϣ���е�����

extern OS_FLAG_GRP	EventFlags;		//����һ���¼���־��
extern OS_Q  Com_Msg;				//����һ����Ϣ���У����ڷ�������

//�Զ���ȫ�ֱ���
OS_TMR 	tmr1;		//��ʱ��1
OS_TMR	tmr2;		//��ʱ��2
u8 msgContent[50];
u8 msgLength;

OS_Q DATA_Msg;				//����һ����Ϣ���У����ڷ�������

u8 runningStatus;
u8 feederLed;

//�Զ��庯��
void tmr1_callback(void *p_tmr, void *p_arg); 	//��ʱ��1�ص�����
void tmr2_callback(void *p_tmr, void *p_arg);	//��ʱ��2�ص�����


//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		512
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);


//---------------------------------------------------------------------
//
//�������ȼ�
#define TASK1_PRIO	20
//�����ջ
#define TASK1_STK_SIZE		128
//������ƿ�
OS_TCB	TASK1_TaskTCB;
//�����ջ
__align(8) CPU_STK	TASK1_TASK_STK[TASK1_STK_SIZE];
//������
void TASK1_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//�������ȼ�
#define TASK2_PRIO	5
//�����ջ
#define TASK2_STK_SIZE		128
//������ƿ�
OS_TCB	TASK2_TaskTCB;
//�����ջ
__align(8) CPU_STK	TASK2_TASK_STK[TASK2_STK_SIZE];
//������
void TASK2_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//�������ȼ�
#define TASK3_PRIO	20
//�����ջ
#define TASK3_STK_SIZE		128
//������ƿ�
OS_TCB	TASK3_TaskTCB;
//�����ջ
__align(8) CPU_STK	TASK3_TASK_STK[TASK3_STK_SIZE];
//������
void TASK3_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//�������ȼ�
#define TASK4_PRIO	7
//�����ջ
#define TASK4_STK_SIZE		128
//������ƿ�
OS_TCB	TASK4_TaskTCB;
//�����ջ
__align(8) CPU_STK	TASK4_TASK_STK[TASK4_STK_SIZE];
//������
void TASK4_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//�������ȼ�
#define TASK5_PRIO	30
//�����ջ
#define TASK5_STK_SIZE		128
//������ƿ�
OS_TCB	TASK5_TaskTCB;
//�����ջ
__align(8) CPU_STK	TASK5_TASK_STK[TASK5_STK_SIZE];
//������
void TASK5_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//�������ȼ�
#define FLAGSPROCESS_TASK_PRIO	5
//�����ջ��С	
#define FLAGSPROCESS_STK_SIZE 	128
//������ƿ�
OS_TCB Flagsprocess_TaskTCB;
//�����ջ	
CPU_STK FLAGSPROCESS_TASK_STK[FLAGSPROCESS_STK_SIZE];
//������
void flagsprocess_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//�������ȼ�
#define FLAGSPROCESS2_TASK_PRIO	6
//�����ջ��С	
#define FLAGSPROCESS2_STK_SIZE 	128
//������ƿ�
OS_TCB Flagsprocess2_TaskTCB;
//�����ջ	
CPU_STK FLAGSPROCESS2_TASK_STK[FLAGSPROCESS2_STK_SIZE];
//������
void flagsprocess2_task(void *p_arg);
//
//---------------------------------------------------------------------

void setup()
{
    runningStatus = 0;
    msgLength = 0;
    feederLed = 0;
}

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	uart_init(9600);    //���ڲ���������
    uart2_init(9600);  //for debug
	LED_Init();         //LED��ʼ��

	OSInit(&err);		//��ʼ��UCOSIII
	OS_CRITICAL_ENTER();//�����ٽ���
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);  //����UCOSIII
	while(1);
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif
    
	OS_CRITICAL_ENTER();	//�����ٽ���
	
    //����һ���¼���־��
	OSFlagCreate((OS_FLAG_GRP*)&EventFlags,		//ָ���¼���־��
                 (CPU_CHAR*	  )"Event Flags",	//����
                 (OS_FLAGS	  )FRONT_FEEDER_FLAGS_VALUE,	//�¼���־���ʼֵ
                 (OS_ERR*  	  )&err);			//������

    //������Ϣ����DATA_Msg
	OSQCreate((OS_Q*		)&Com_Msg,	
              (CPU_CHAR*	)"Com Msg",	
              (OS_MSG_QTY	)DATAMSG_Q_NUM,	
              (OS_ERR*)&err); 
    
	OSTaskCreate((OS_TCB 	* )&TASK1_TaskTCB,		
				 (CPU_CHAR	* )"task1", 		
                 (OS_TASK_PTR )TASK1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK1_PRIO,     
                 (CPU_STK   * )&TASK1_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK1_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	OSTaskCreate((OS_TCB 	* )&TASK2_TaskTCB,		
				 (CPU_CHAR	* )"task2", 		
                 (OS_TASK_PTR )TASK2_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK2_PRIO,     
                 (CPU_STK   * )&TASK2_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK2_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK2_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	
		 
	OSTaskCreate((OS_TCB 	* )&TASK3_TaskTCB,		
				 (CPU_CHAR	* )"task3", 		
                 (OS_TASK_PTR )TASK3_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK3_PRIO,     
                 (CPU_STK   * )&TASK3_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK3_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK3_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
                 
    OSTaskCreate((OS_TCB 	* )&TASK4_TaskTCB,		
				 (CPU_CHAR	* )"task4", 		
                 (OS_TASK_PTR )TASK4_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK4_PRIO,     
                 (CPU_STK   * )&TASK4_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK4_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK4_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
                               
    OSTaskCreate((OS_TCB 	* )&TASK5_TaskTCB,		
				 (CPU_CHAR	* )"task5", 		
                 (OS_TASK_PTR )TASK5_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK5_PRIO,     
                 (CPU_STK   * )&TASK5_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK5_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK5_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);

    OSTaskCreate((OS_TCB*     )&Flagsprocess_TaskTCB,		
				 (CPU_CHAR*   )"Flagsprocess task", 		
                 (OS_TASK_PTR )flagsprocess_task, 			
                 (void* 	  )0,					
                 (OS_PRIO	  )FLAGSPROCESS_TASK_PRIO,     
                 (CPU_STK* 	  )&FLAGSPROCESS_TASK_STK[0],	
                 (CPU_STK_SIZE)FLAGSPROCESS_STK_SIZE/10,	
                 (CPU_STK_SIZE)FLAGSPROCESS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void* 	  )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR* 	  )&err);	                    
 
    OSTaskCreate((OS_TCB*     )&Flagsprocess2_TaskTCB,		
				 (CPU_CHAR*   )"Flagsprocess2 task", 		
                 (OS_TASK_PTR )flagsprocess2_task, 			
                 (void* 	  )0,					
                 (OS_PRIO	  )FLAGSPROCESS2_TASK_PRIO,     
                 (CPU_STK* 	  )&FLAGSPROCESS2_TASK_STK[0],	
                 (CPU_STK_SIZE)FLAGSPROCESS2_STK_SIZE/10,	
                 (CPU_STK_SIZE)FLAGSPROCESS2_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void* 	  )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR* 	  )&err);	
                 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
}

void status_switch_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;   
	while(1)
	{    
        //OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ200ms
    }    
}

void TASK1_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;    
    while(0){
        DebugOutput("task 1\r\n");
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
    }
}

void TASK2_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;    
    while(0){
        DebugOutput("task 2\r\n");
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
    }
}

extern u8 transFlag;
extern u8 blinkGridNumber;
//LED��˸����
void TASK3_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;    
    DebugOutput("start task 3\r\n");
    while(1){
        
        if(blinkGridNumber){
            LED0 = 1;
            ledOff(blinkGridNumber);
            OSTimeDlyHMSM(0,0,LED_BLINK_INTERVAL_SEC,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1000ms
            LED0 = 0;
            ledOn(blinkGridNumber);
            OSTimeDlyHMSM(0,0,LED_BLINK_INTERVAL_SEC,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1000ms
        }
        
        if(feederLed){
            DebugOutput("task 3\r\n");
            LED0 = 0;
            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1000ms
            LED0 = 1;
            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1000ms
        }
        //OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1000ms
    }
}


void TASK4_task(void *p_arg)
{
    u8 *p;
    u8 i;
    OS_MSG_SIZE size;
	OS_ERR err;
	p_arg = p_arg;  
    DebugOutput("task 4 start\r\n"); 
    while(0){
        DebugOutput("task 4\r\n");
        OSTimeDlyHMSM(0,0,2,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
    }    
    while(1){
        DebugOutput("Pending for msg...\r\n");
        p = OSQPend((OS_Q*		)&Com_Msg,   
			        (OS_TICK	)0,
                    (OS_OPT		)OS_OPT_PEND_BLOCKING,
                    (OS_MSG_SIZE*	)&size,	
                    (CPU_TS*	)0,
                    (OS_ERR*	)&err);
        NOP();

        for(i = 0; i < size; i++)//�ж��ǲ��ǷǷ��ַ�!
        {   
            DebugOutput("%c", *p);
            p++;
        } 
        DebugOutput("Got Msg, size is %d", size);
        //DebugOutput("task 4\r\n");
        //OSTimeDlyHMSM(0,0,2,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
    }
}

void TASK5_task(void *p_arg)
{
    u8 counter = 0;  
	OS_ERR err;
    OS_FLAGS flags_num;
	p_arg = p_arg; 
    while(1){
        DebugOutput("TASK5 running \r\n");
        OSTimeDlyHMSM(0,0,3,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
        //sendComAnalyzeEvent();
    }
    while(0){
        counter++;
        DebugOutput("task 5 start\r\n");
        if( counter == 2){
             //���¼���־��EventFlags���ͱ�־
	        flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    				 (OS_FLAGS	  )FRONT_FEEDER_LED_FLAG,
				    			 (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					    		 (OS_ERR*     )&err);
		    DebugOutput("1 �¼���־��EventFlags��ֵ:%d\r\n",flags_num);
        }
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
        
        if(counter == 3){
             //���¼���־��EventFlags���ͱ�־
	        flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    				 (OS_FLAGS	  )FRONT_FEEDER_LED2_FLAG,
				    			 (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					    		 (OS_ERR*     )&err);
		    DebugOutput("2 �¼���־��EventFlags��ֵ:%d\r\n",flags_num);        
        }
    }
}


//�¼���־�鴦������
//����������ڴ���ͨѶ����
void flagsprocess_task(void *p_arg)
{
	OS_ERR err; 
    DebugOutput("flag 1 task start.\r\n");
    while(0){
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
    }
	while(1)
	{
		//�ȴ��¼���־�� ANA
		OSFlagPend((OS_FLAG_GRP*)&EventFlags,
				   (OS_FLAGS	)COM_ANA_FLAG,
		     	   (OS_TICK     )0,
				   (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL + OS_OPT_PEND_FLAG_CONSUME,
				   (CPU_TS*     )0,
				   (OS_ERR*	    )&err);
        //����ͨ��ʶ��
        DebugOutput("Start to analyze input cmd\r\n");
        
        comAnalyze();
		//DebugOutput("flag1 task �յ��¼���־��EventFlags��ֵ:%d\r\n",EventFlags.Flags);

	}
}

//�¼���־�鴦������
void flagsprocess2_task(void *p_arg)
{
	OS_ERR err; 
    DebugOutput("flag 2 task start.\r\n");
    while(0){
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
    }
	while(1)
	{
		//�ȴ��¼���־��
		OSFlagPend((OS_FLAG_GRP*)&EventFlags,
				   (OS_FLAGS	)FRONT_FEEDER_LED_FLAG,
		     	   (OS_TICK     )0,
                   (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL + OS_OPT_PEND_FLAG_CONSUME,
				   //(OS_OPT	)OS_OPT_PEND_FLAG_CONSUME,
				   (CPU_TS*     )0,
				   (OS_ERR*	    )&err);
        //OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
		DebugOutput("LED start to blink");
        feederLed = 1;
	}
    
  
}
