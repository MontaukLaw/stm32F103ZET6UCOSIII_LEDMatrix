#include "upcom.h"

extern u8 transFlag;
extern u8 usartRecieveCounter;

u8 integrityCheck(void){
    u8 chk = 0;
    u8 i = 0;
    //首先检查包头包尾
    if(USART_RX_BUF[0] == 0xFF && USART_RX_BUF[1] == 0xFF && USART_RX_BUF[usartRecieveCounter-2] == 0xFE && USART_RX_BUF[usartRecieveCounter-1] == 0xFE){
        //然后累加消息长度跟消息体数据作为chk
        for(i = 3; i < usartRecieveCounter-2; i++){
            chk = chk + USART_RX_BUF[i];
        }
            DebugOutput("chk should be %d,\r\n",chk);  
        if(chk == USART_RX_BUF[2]){
            
            DebugOutput("trans complete,\r\n");  
            //这里要调用ACK
            return 1;
        }else{
            DebugOutput("trans error,\r\n");  
            return 0;
        }
        
    }
}

u8 blinkGridNumber = 0;

//灯控制
u8 comAnalyze(void){
    u16 t;
    u8 msgBodyLen;    
    u8 cmd;
    //u8 gridNumber = 0;
    //u16 len;
    if(transFlag == TRANS_END)
	{   
        //len = usartRecieveCounter;//得到此次接收到的数据长度
        DebugOutput("comAnalyze, %d bytes received:",usartRecieveCounter);  
        //如果数据完整
        if(integrityCheck()){
            msgBodyLen = USART_RX_BUF[3];
            cmd = USART_RX_BUF[5];
            switch(cmd){
                case GRID_BLINK:
                    blinkGridNumber = USART_RX_BUF[6];

                    break;
                case TURN_OFF_LED:
                    blinkGridNumber = 0;
                    break;
                                    
            }
            
            
        }
        //分析完后恢复初始状态, anyway
        transFlag = TRANS_NONE; 
        usartRecieveCounter = 0;    
    }
    
}


//灯控: 
//门控:
//ACK: 

u8 comAnalyzeOld(void){
 	u16 t;    
    u16 len;
    if(USART_RX_STA&0x8000)
	{					   
		len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
        DebugOutput("comAnalyze:"); 
        
        if(USART_RX_BUF[0]=='R' && USART_RX_BUF[1]=='D' && USART_RX_BUF[2]=='Y'){
            DebugOutput("Front Feeder Ready\r\n");
            //令进票口LED开始闪烁
            sendFrontFeederLEDEvent();
        }
		//for(t=0;t<len;t++)
		//{
			//DebugOutput("%c",USART_RX_BUF[t]);
            //USART_SendData(USART1, USART_RX_BUF[t]);//向串口1发送数据
			//while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
		//}
	    	//printf("\r\n\r\n");//插入换行
			USART_RX_STA=0;
	} 
           	
}




