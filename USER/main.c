#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "malloc.h"
#include "upcom.h"

////////////////////////事件标志组//////////////////////////////

//生产环境将DEBUG开关置0
#define DEBUG  1
#define FRONT_FEEDER_READY   1
#define DATAMSG_Q_NUM	10	//发送数据的消息队列的数量

extern OS_FLAG_GRP	EventFlags;		//定义一个事件标志组
extern OS_Q  Com_Msg;				//定义一个消息队列，用于发送数据

//自定义全局变量
OS_TMR 	tmr1;		//定时器1
OS_TMR	tmr2;		//定时器2
u8 msgContent[50];
u8 msgLength;

OS_Q DATA_Msg;				//定义一个消息队列，用于发送数据

u8 runningStatus;
u8 feederLed;

//自定义函数
void tmr1_callback(void *p_tmr, void *p_arg); 	//定时器1回调函数
void tmr2_callback(void *p_tmr, void *p_arg);	//定时器2回调函数


//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);


//---------------------------------------------------------------------
//
//任务优先级
#define TASK1_PRIO	20
//任务堆栈
#define TASK1_STK_SIZE		128
//任务控制块
OS_TCB	TASK1_TaskTCB;
//任务堆栈
__align(8) CPU_STK	TASK1_TASK_STK[TASK1_STK_SIZE];
//任务函数
void TASK1_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//任务优先级
#define TASK2_PRIO	5
//任务堆栈
#define TASK2_STK_SIZE		128
//任务控制块
OS_TCB	TASK2_TaskTCB;
//任务堆栈
__align(8) CPU_STK	TASK2_TASK_STK[TASK2_STK_SIZE];
//任务函数
void TASK2_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//任务优先级
#define TASK3_PRIO	20
//任务堆栈
#define TASK3_STK_SIZE		128
//任务控制块
OS_TCB	TASK3_TaskTCB;
//任务堆栈
__align(8) CPU_STK	TASK3_TASK_STK[TASK3_STK_SIZE];
//任务函数
void TASK3_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//任务优先级
#define TASK4_PRIO	7
//任务堆栈
#define TASK4_STK_SIZE		128
//任务控制块
OS_TCB	TASK4_TaskTCB;
//任务堆栈
__align(8) CPU_STK	TASK4_TASK_STK[TASK4_STK_SIZE];
//任务函数
void TASK4_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//
//任务优先级
#define TASK5_PRIO	30
//任务堆栈
#define TASK5_STK_SIZE		128
//任务控制块
OS_TCB	TASK5_TaskTCB;
//任务堆栈
__align(8) CPU_STK	TASK5_TASK_STK[TASK5_STK_SIZE];
//任务函数
void TASK5_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//任务优先级
#define FLAGSPROCESS_TASK_PRIO	5
//任务堆栈大小	
#define FLAGSPROCESS_STK_SIZE 	128
//任务控制块
OS_TCB Flagsprocess_TaskTCB;
//任务堆栈	
CPU_STK FLAGSPROCESS_TASK_STK[FLAGSPROCESS_STK_SIZE];
//任务函数
void flagsprocess_task(void *p_arg);
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//任务优先级
#define FLAGSPROCESS2_TASK_PRIO	6
//任务堆栈大小	
#define FLAGSPROCESS2_STK_SIZE 	128
//任务控制块
OS_TCB Flagsprocess2_TaskTCB;
//任务堆栈	
CPU_STK FLAGSPROCESS2_TASK_STK[FLAGSPROCESS2_STK_SIZE];
//任务函数
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
	
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	uart_init(9600);    //串口波特率设置
    uart2_init(9600);  //for debug
	LED_Init();         //LED初始化

	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif
    
	OS_CRITICAL_ENTER();	//进入临界区
	
    //创建一个事件标志组
	OSFlagCreate((OS_FLAG_GRP*)&EventFlags,		//指向事件标志组
                 (CPU_CHAR*	  )"Event Flags",	//名字
                 (OS_FLAGS	  )FRONT_FEEDER_FLAGS_VALUE,	//事件标志组初始值
                 (OS_ERR*  	  )&err);			//错误码

    //创建消息队列DATA_Msg
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
                 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//退出临界区
}

void status_switch_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;   
	while(1)
	{    
        //OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //延时200ms
    }    
}

void TASK1_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;    
    while(0){
        DebugOutput("task 1\r\n");
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
    }
}

void TASK2_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;    
    while(0){
        DebugOutput("task 2\r\n");
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
    }
}

extern u8 transFlag;
extern u8 blinkGridNumber;
//LED闪烁任务
void TASK3_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;    
    DebugOutput("start task 3\r\n");
    while(1){
        
        if(blinkGridNumber){
            LED0 = 1;
            ledOff(blinkGridNumber);
            OSTimeDlyHMSM(0,0,LED_BLINK_INTERVAL_SEC,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1000ms
            LED0 = 0;
            ledOn(blinkGridNumber);
            OSTimeDlyHMSM(0,0,LED_BLINK_INTERVAL_SEC,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1000ms
        }
        
        if(feederLed){
            DebugOutput("task 3\r\n");
            LED0 = 0;
            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1000ms
            LED0 = 1;
            OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1000ms
        }
        //OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1000ms
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
        OSTimeDlyHMSM(0,0,2,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
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

        for(i = 0; i < size; i++)//判断是不是非法字符!
        {   
            DebugOutput("%c", *p);
            p++;
        } 
        DebugOutput("Got Msg, size is %d", size);
        //DebugOutput("task 4\r\n");
        //OSTimeDlyHMSM(0,0,2,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
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
        OSTimeDlyHMSM(0,0,3,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
        //sendComAnalyzeEvent();
    }
    while(0){
        counter++;
        DebugOutput("task 5 start\r\n");
        if( counter == 2){
             //向事件标志组EventFlags发送标志
	        flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    				 (OS_FLAGS	  )FRONT_FEEDER_LED_FLAG,
				    			 (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					    		 (OS_ERR*     )&err);
		    DebugOutput("1 事件标志组EventFlags的值:%d\r\n",flags_num);
        }
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
        
        if(counter == 3){
             //向事件标志组EventFlags发送标志
	        flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    				 (OS_FLAGS	  )FRONT_FEEDER_LED2_FLAG,
				    			 (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					    		 (OS_ERR*     )&err);
		    DebugOutput("2 事件标志组EventFlags的值:%d\r\n",flags_num);        
        }
    }
}


//事件标志组处理任务
//这个任务用于处理通讯处理
void flagsprocess_task(void *p_arg)
{
	OS_ERR err; 
    DebugOutput("flag 1 task start.\r\n");
    while(0){
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
    }
	while(1)
	{
		//等待事件标志组 ANA
		OSFlagPend((OS_FLAG_GRP*)&EventFlags,
				   (OS_FLAGS	)COM_ANA_FLAG,
		     	   (OS_TICK     )0,
				   (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL + OS_OPT_PEND_FLAG_CONSUME,
				   (CPU_TS*     )0,
				   (OS_ERR*	    )&err);
        //进行通信识别
        DebugOutput("Start to analyze input cmd\r\n");
        
        comAnalyze();
		//DebugOutput("flag1 task 收到事件标志组EventFlags的值:%d\r\n",EventFlags.Flags);

	}
}

//事件标志组处理任务
void flagsprocess2_task(void *p_arg)
{
	OS_ERR err; 
    DebugOutput("flag 2 task start.\r\n");
    while(0){
        OSTimeDlyHMSM(0,0,1,000,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
    }
	while(1)
	{
		//等待事件标志组
		OSFlagPend((OS_FLAG_GRP*)&EventFlags,
				   (OS_FLAGS	)FRONT_FEEDER_LED_FLAG,
		     	   (OS_TICK     )0,
                   (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL + OS_OPT_PEND_FLAG_CONSUME,
				   //(OS_OPT	)OS_OPT_PEND_FLAG_CONSUME,
				   (CPU_TS*     )0,
				   (OS_ERR*	    )&err);
        //OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
		DebugOutput("LED start to blink");
        feederLed = 1;
	}
    
  
}
