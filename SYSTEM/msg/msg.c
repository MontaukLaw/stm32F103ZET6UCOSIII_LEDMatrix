#include "msg.h"
#include "usart.h"

OS_FLAG_GRP	EventFlags;		//定义一个事件标志组
OS_Q Com_Msg;				//定义一个消息队列，用于发送数据

void sendFrontFeederLEDEvent(void)
{
    OS_ERR err;
    OS_FLAGS flags_num;
    
	flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    	     (OS_FLAGS	  )FRONT_FEEDER_LED_FLAG,
				         (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					     (OS_ERR*     )&err);
    //过程保护
    if(err != OS_ERR_NONE)
    {
        DebugOutput("msg/sendFrontFeederLEDEvent: Send event fail!");
    } 
}

void sendComAnalyzeEvent(void)
{
    OS_ERR err;
    OS_FLAGS flags_num;
    
    //发送flag为0x04
	flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    	     (OS_FLAGS	  )COM_ANA_FLAG,
				         (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					     (OS_ERR*     )&err);
    //过程保护
    if(err != OS_ERR_NONE)
    {
        DebugOutput("msg/sendComAnalyzeEvent: Send event fail!");
    } 
}

void sendMsg2StatusSwitch(void){
    OS_ERR err;
    const char *msgContent="DEMO";
    //发送消息
    OSQPost((OS_Q*		)&Com_Msg,		
            (void*		)msgContent,
            (OS_MSG_SIZE)COM_MSG_SIZE,
            (OS_OPT		)OS_OPT_POST_FIFO,
            (OS_ERR*	)&err);
    if(err != OS_ERR_NONE)
    {
        DebugOutput("Send msg fail!");
    } 
    
}


