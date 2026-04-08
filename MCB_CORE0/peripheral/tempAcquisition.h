/*
 * tempAcquisition.h
 *
 *  Created on: 2022Äê5ÔÂ18ÈÕ
 *      Author: Administrator
 */

#ifndef PERIPHERAL_TEMPACQUISITION_H_
#define PERIPHERAL_TEMPACQUISITION_H_

#include "../RS485/RS485.h"
#include "../srio/srio.h"
#include "../control/diagnose.h"
typedef enum TdDataType{
    PT100,
    PT1000,
    voltage
}TdDataType;
//int  Td_init(unsigned char Sfpchannel,unsigned short RulerMode,unsigned char Rs485pin);
void OgModeSet(unsigned char nSfpCh, int nOgMode);
int  Td_init(unsigned char Sfpchannel,unsigned short RulerMode,unsigned char Rs485pin,unsigned int BaudRate);
int  TdReadTopHalf(unsigned char Sfpchannel,unsigned char Rs485pin,unsigned char StartAddr,
                   unsigned char ReadNum,   TdDataType type);
int TDReadBottomHalf(unsigned char Sfpchannel,unsigned char Rs485pin,unsigned char ReadNum,double *buf);
int TDWrite(unsigned char Sfpchannel,unsigned char Rs485pin,unsigned char StartAddr,unsigned char WriteNum,unsigned char value);
#endif /* PERIPHERAL_TEMPACQUISITION_H_ */
