#ifndef __MSG_H
#define __MSG_H
#include "includes.h"

#define FRONT_FEEDER_FLAGS_VALUE	0X00
#define FRONT_FEEDER_LED_FLAG  0x01
#define FRONT_FEEDER_LED2_FLAG  0x02
#define COM_ANA_FLAG  0x04

#define COM_MSG_SIZE 4

void sendComAnalyzeEvent(void);
void sendMsg2StatusSwitch(void);
void sendFrontFeederLEDEvent(void);

#endif

