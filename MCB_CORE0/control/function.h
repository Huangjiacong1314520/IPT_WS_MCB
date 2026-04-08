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
#include "diagnose.h"
void * MyMemSet(void *dst, int fill, size_t len);
void P2busCfg(unsigned char nP2busMode, unsigned char nStartAddr, unsigned char nReadNum);
void P2busRead(unsigned char *nP2busRecvData, unsigned char nReadNum);
double Controller(double Err_Now, double *U_, double *e_, double *Param, Uint16 n);

void VectorSet(double* A, int n, double value);
void Vector2Set(double* A, int row, int column, double value);

unsigned char PowerCalculate(unsigned int number);

void InsertCommitData(int count, int channel, double value);
double MaxDouble(double* data, int num);
void ParamInit();
void SetMultiCoreInterruptFlag();
void SetMultiCoreRunFlag(unsigned char);
void SetMultiCoreFreshFlag(unsigned char value);
void SetMultiCoreSPara(int axis, SParaHandle sParaSrc);
void SetMultiCoreControlPara(int axis, ControlParaHandle ctrlParaSrc);

void ReadInterferometer();
// 自定义SRIO读写接口
SRIORWStatus SRIOWriteDataLSU(unsigned char cRemoteDevID_8Bit,unsigned char nLSU,unsigned int uWrAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen);
SRIORWStatus SRIOReadDataLSU(unsigned char cRemoteDevID_8Bit,unsigned char nLSU,unsigned int uRdAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen);

#if CORE_NUM == 0
int ReadAD(unsigned char nSfpChannel);
int ReadRuler(unsigned char nSfpChannel);
int ReadRulerTopHalf(unsigned char nSfpChannel);
int ReadRulerBottomHalf(unsigned char nSfpChannel);
int ReadADTopHalf(unsigned char nSfpChannel);
int ReadADBottomHalf(unsigned char nSfpChannel);
int ReadInfTopHalf();
int ReadInfBottomHalf();
void ConvertDouble2DA(const double* fOut, Uint32* uOut, int len);
#elif CORE_NUM == 7
int DAOut(Uint16 uChannelSel, const Uint32* pData);
void InsertCommitData(int count, int channel, double value);
#endif


#endif /* CONTROL_FUNCTION_H_ */
