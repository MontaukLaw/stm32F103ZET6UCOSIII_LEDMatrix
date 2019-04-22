#include "sys.h"
#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os ʹ��	  
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  

void uart2_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART2��GPIOAʱ��
    
    //USART2_TX   GPIOA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
    
    //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Tx;	//�շ�ģʽ
    
    USART_Init(USART2, &USART_InitStructure); //��ʼ������2
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2 

}

u8 txBuffer[TX_MAX_LENGTH];
u16 txBufferLength;

//��������ֽ�����
void cleanTxBuffer(){
    u16 i;
    for(i = 0; i < TX_MAX_LENGTH; i++){
        txBuffer[i]=0;    
    }
}

//���ַ���ת�浽�ֽ�������
void cpStr2TxBuffer(const char *str){
    u16 outputStrLength;
    u16 i;
    cleanTxBuffer(); 
    for(i = 0; i < txBufferLength - 1; i++){
        txBuffer[i]=str[i];
    }
    
}

void NOP(void){
    u8 a = 0;
}

char str_tmp[100];
//use USART2 output debug info
void DebugOutput(char *fmt, ...)
{
    u8 TxCounter = 0;
    u8 txLength = 0;
    va_list vArgList;                            
    va_start (vArgList, fmt);                 
    txLength =_vsnprintf(str_tmp, 100, fmt, vArgList); 
    va_end(vArgList);                           
    NOP();
    
    while(TxCounter < txLength){
        USART_SendData(USART2,str_tmp[TxCounter]);
        TxCounter ++ ;
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET){
        }     
    }
    
}

//���debug log
void debugLog(const char *str){
    u8 TxCounter = 0;
    txBufferLength = strlen(str);
    cpStr2TxBuffer(str);
    
    while(TxCounter < txBufferLength-1){
        USART_SendData(USART2,txBuffer[TxCounter++]);
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET){
        //just wait
        }     
    }
    TxCounter = 0;
}

//��ʼ��IO ����1 
//bound:������
void uart_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
 	USART_DeInit(USART1);  //��λ����1
	//USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 

}

u8 transFlag = TRANS_NONE;
u8 usartRecieveCounter = 0;
//��дһ�´����ж�
void USART1_IRQHandler(void){
	u8 Res;
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){  //�����ж�(���յ������ݱ�����0xFE 0xFE��β)
        Res = USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������   
       
        //������յ�����0xFF
        if(Res == 0xFF){
            switch(transFlag){
                case TRANS_NONE:
                    usartRecieveCounter = 0;
                    transFlag = TRANS_HALFSTART;
                    break;
                case TRANS_HALFSTART:
                    transFlag = TRANS_SENDING;
                    break;
                //case TRANS_SENDING:
                    //transFlag = TRANS_READYEND;
                    //break;                                                    
            }
        //����յ���0xFE            
        }else if(Res == 0xFE){
            switch (transFlag){
                case TRANS_SENDING:
                    transFlag = TRANS_READYEND;
                    break;
                case TRANS_READYEND:
                    transFlag = TRANS_END;
                    sendComAnalyzeEvent();
                    break;
                default:
                   //���У����������254, Ҳ��Ҫȥ������.
                   transFlag = TRANS_SENDING; 
            }
                   
        }else{
            //����յ��ķ�0xFF0xFE, ���̽�״̬���⵽�����е�״̬.
            transFlag = TRANS_SENDING;
           
        }
        USART_RX_BUF[usartRecieveCounter] = Res;  
        usartRecieveCounter++;   
        
    }
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
}

void USART1_IRQHandler_bak(void)                	//����1�жϷ������
{
	u8 Res;

#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a){
                    USART_RX_STA=0;//���մ���,���¿�ʼ
				}else{ 
                    sendComAnalyzeEvent();
                    USART_RX_STA|=0x8000;	//��������� 
				}
            }
			else //��û�յ�0X0D
			{	
                //
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}   		 
     } 
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
} 
#endif	

