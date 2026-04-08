/*
 * refcurve.h
 *
 *  Created on: 2021年8月22日
 *      Author: lkx
 */

#ifndef CONTROL_REFCURVE_H_
#define CONTROL_REFCURVE_H_


#include "global.h"
#include "function.h"

extern double RefValueLast[SlotCount][6];
extern double RefValue[SlotCount][6];
extern int StateLast[SlotCount];
extern double SpeedLowConst[SlotCount];
extern double StayTime[SlotCount];
extern double SwitchDt[SlotCount];
extern double TNodes[SlotCount][32];
/*extern double RefValueLast[6];                       // 生成S曲线用到的历史数据
extern double RefValue[6];                           // S曲线实时数据
extern int StateLast;                                // 当前S曲线阶段
extern double SpeedLowConst;                         // 低速常量（当S曲线无法生成时使用）
extern double StayTime;                              // 保持时间
extern double SwitchDt;*/

double Sine_DatCreator(int count,double freq,double Amp,double phase);
void S3_DataCreator(double* realValue, double* value, double* lastValue, double* para,double originPos,double targetPos,unsigned long long count);
void S4_DataCreator(double* realValue, double* value, double* lastValue, double* para, int stateLast,
                    double originPos,double targetPos,unsigned long long count, int* tempindex,Slot slot);
void S5_DataCreator(double* realValue, double* value, double* lastValue, double* para, int stateLast,
                    double originPos,double targetPos,unsigned long long count,Slot slot);
void RampCreator(double* realValue, double* value, double originPos, double targetPos, double speed, unsigned long long count);
void RefCurveCreator(int order, double *waveNum, double* _realValue, double* _para, double originPos,
                     double targetPos, int times, unsigned long long int count,int *index,Slot slot);
void DataCpy(double* dst, double* src, int len);
void CurveValueSet(int axis);
void CurveValueSetAll();
int S5CheckData(double* para, double* waveNum,Slot slot);
int S4CheckData(double* para, double* waveNum,Slot slot);
double Abs(double a);
double Pow(double v, int n);
int Round(double v);
void GetParameter();
void ControllerOutProtect(double* posErr, double* FOut);

#endif /* CONTROL_REFCURVE_H_ */
