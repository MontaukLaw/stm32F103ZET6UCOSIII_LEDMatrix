#include "msg.h"
#include "usart.h"

OS_FLAG_GRP	EventFlags;		//����һ���¼���־��
OS_Q Com_Msg;				//����һ����Ϣ���У����ڷ�������

void sendFrontFeederLEDEvent(void)
{
    OS_ERR err;
    OS_FLAGS flags_num;
    
	flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    	     (OS_FLAGS	  )FRONT_FEEDER_LED_FLAG,
				         (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					     (OS_ERR*     )&err);
    //���̱���
    if(err != OS_ERR_NONE)
    {
        DebugOutput("msg/sendFrontFeederLEDEvent: Send event fail!");
    } 
}

void sendComAnalyzeEvent(void)
{
    OS_ERR err;
    OS_FLAGS flags_num;
    
    //����flagΪ0x04
	flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
			    	     (OS_FLAGS	  )COM_ANA_FLAG,
				         (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					     (OS_ERR*     )&err);
    //���̱���
    if(err != OS_ERR_NONE)
    {
        DebugOutput("msg/sendComAnalyzeEvent: Send event fail!");
    } 
}

void sendMsg2StatusSwitch(void){
    OS_ERR err;
    const char *msgContent="DEMO";
    //������Ϣ
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


