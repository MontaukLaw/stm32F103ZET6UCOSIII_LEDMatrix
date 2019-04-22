#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include "msg.h" 
#include "includes.h"

//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define TX_MAX_LENGTH      20   //usar2 send bytes
//#define CMD_BUFFER_LEN 200

#define  TRANS_NONE  0
#define TRANS_HALFSTART  1
#define TRANS_SENDING 2
#define TRANS_READYEND 3
#define TRANS_END 4




extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void uart2_init(u32 bound);

void debugLog(const char *str);
void DebugOutput(char *fmt, ...);

void NOP(void);

extern OS_FLAG_GRP	EventFlags;		//定义一个事件标志组
extern OS_Q Com_Msg;				//定义一个消息队列，用于发送数据

#endif


