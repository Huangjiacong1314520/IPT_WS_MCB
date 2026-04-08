/*
 * interface.h
 *
 *  Created on: 2022Äê5ÔÂ24ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_CONNECTOR_H_
#define CONTROL_CONNECTOR_H_

#include "global.h"
#include "function.h"
#include "../drv_srio.h"
#include "../drv_cache.h"
#include "../kern/macros.h"
#include "hw_soc_C6678.h"
#include "src/intc/csl_intc.h"
#include "src/intc/csl_intcAux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void IsrPrologue();
void IsrEpilogue(unsigned long long int ms);

void ClearGlobalVar();
void NotifyOtherCores();
void RulerDataProcess();
void ADDataProcess();
void StagePosReset();
void SetAxisRunflag(Axis axis,unsigned char runflag);
void SetallAxisRunflag(unsigned char runflag);
void SetAxisRefreshFlag(Axis axis,unsigned char refreshflag);
void SetallAxisRefreshFlag(unsigned char refreshflag);
void RefreshParam();
int CheckSParamCacheEmpty();
void RequestNewSParam();
void ReceiveControlCommand();
void ParseControlCommand();

void SwitchAxisTask(Axis axis,Task task);
void SwitchallAxisTask(Task task);

void CheckSensorDataTransfer(unsigned long long ms);
int CheckAxisFinish(Axis axis);
int CheckAllAxisFinish();
void ClearAllaxisFinishFlag();
void CommitData();
void TransferSysInfo();
void VxDooBellSend();
int CheckPeripheralState();
void MoveFinishLock(unsigned char lock);
void ReadSensorTopHalf();
void TimeCountStart();
void InitMoveMode(State curstate);
int ParseMoveMode();
int CheckAllAxisWithoutZ();
void RefreshZParam();
void ZPrefetchNext();
inline unsigned long long GetCurTime()
{
    unsigned tscl,tsch;
    TSCL = 0;
    tscl= TSCL;
    tsch= TSCH;
    return _itoll(tsch,tscl);
}

//inline int DelayInterval(unsigned int intervalnum)
//{
//
//}
#endif /* CONTROL_CONNECTOR_H_ */
