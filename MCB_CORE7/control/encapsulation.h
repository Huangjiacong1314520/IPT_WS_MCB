/*
 * ecapsulation.h
 *
 *  Created on: 2022Äê5ÔÂ26ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_ENCAPSULATION_H_
#define CONTROL_ENCAPSULATION_H_
#include "global.h"
#include <stdint.h>
void IsrPrologue();
void IsrEpilogue();


void GBcompensate(double Motoroutput[MotorCount]);
void GScompensate();
void DAChannelAssign(double value,DAChannel nChannel);
int DAOutput(unsigned char nSFPChannel);

void ProtectOut(double value[MotorCount],double limit);
void ConvertDouble2DA(const double fOut[MotorCount], Uint32 uOut[MotorCount]);
void RefreshShareData(double *Motoroutput,int n);
inline unsigned long long GetCurTime()
{
    unsigned tscl,tsch;
    TSCL = 0;
    tscl= TSCL;
    tsch= TSCH;
    return _itoll(tsch,tscl);
}
#endif /* CONTROL_ENCAPSULATION_H_ */
