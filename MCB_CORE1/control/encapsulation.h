/*
 * encapsulation.h
 *
 *  Created on: 2022Äê5ÔÂ23ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_ENCAPSULATION_H_
#define CONTROL_ENCAPSULATION_H_
#include "global.h"
#include "function.h"
#include "../drv_cache.h"
void IsrPrologue();
void IsrEpilogue();

double AcquireCurPoint(Axis axis);
double AcquireTargetPoint(Slot slotnum,Axis axis);
void FeedBackController(Slot slotnum,Axis axis);
void ForwardController(Slot slotnum,Axis axis);
void CalculateNextPoint(Slot slotnum,Axis axis,int *index,unsigned long long int delay,unsigned long long int proberdelay);
void RefreshSysteminfo();
void Clear(Slot slotnum,Axis axis);
void SetAxisRefreshFlag(Axis axis,unsigned char refreshflag);

#endif /* CONTROL_ENCAPSULATION_H_ */
