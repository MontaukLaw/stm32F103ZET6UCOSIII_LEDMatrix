#include "upcom.h"

u8 comAnalyze(void){
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




