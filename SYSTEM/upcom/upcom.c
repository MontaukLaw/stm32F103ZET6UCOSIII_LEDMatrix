#include "upcom.h"

u8 comAnalyze(void){
 	u16 t;    
    u16 len;
    if(USART_RX_STA&0x8000)
	{					   
		len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
        DebugOutput("comAnalyze:"); 
        
        if(USART_RX_BUF[0]=='R' && USART_RX_BUF[1]=='D' && USART_RX_BUF[2]=='Y'){
            DebugOutput("Front Feeder Ready\r\n");
            //���Ʊ��LED��ʼ��˸
            sendFrontFeederLEDEvent();
        }
		//for(t=0;t<len;t++)
		//{
			//DebugOutput("%c",USART_RX_BUF[t]);
            //USART_SendData(USART1, USART_RX_BUF[t]);//�򴮿�1��������
			//while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
		//}
	    	//printf("\r\n\r\n");//���뻻��
			USART_RX_STA=0;
	} 
           	
}




