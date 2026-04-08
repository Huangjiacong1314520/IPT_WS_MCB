/*
 * function.h
 *
 *  Created on: 2021年8月22日
 *      Author: lkx
 */

#ifndef CONTROL_FUNCTION_H_
#define CONTROL_FUNCTION_H_

#include "global.h"
#include "../drv_srio.h"

void * MyMemSet(void *dst, int fill, size_t len);
double Controller(double Err_Now, double *U_, double *e_, double *Param, Uint16 n);

void VectorSet(double* A, int n, double value);
void Vector2Set(double* A, int row, int column, double value);

double MaxDouble(double* data, int num);
void ParamInit();
void SetMultiCoreInterruptFlag();
void SetMultiCoreRunFlag(unsigned char);
void SetMultiCoreFreshFlag();
void SetMultiCoreSPara(int axis, SParaHandle sParaSrc);
void SetMultiCoreControlPara(int axis, ControlParaHandle ctrlParaSrc);

// 自定义SRIO读写接口
SRIORWStatus SRIOWriteDataLSU(unsigned char cRemoteDevID_8Bit,unsigned int uWrAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen);
SRIORWStatus SRIOReadDataLSU(unsigned char cRemoteDevID_8Bit, unsigned int uRdAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen);

#if CORE_NUM == 0
int ReadAD();
int ReadRuler();
#elif CORE_NUM == 7
int DAOut(Uint16 uChannelSel, const Uint32* pData);
void InsertCommitData(int count, int channel, double value);
#endif


#endif /* CONTROL_FUNCTION_H_ */
