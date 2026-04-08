/*
 * global.h
 *
 *  Created on: 2021年8月22日
 *      Author: lkx
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../interfacedrive.h"
#include "../TestDriver.h"
#include <assert.h>
#include "../drv_srio.h"
#include "../srio/srio.h"
#include "c6x.h"
#include "../srio/srio_cfg.h"
#include "csl_cacheAux.h"
#include "../kern/macros.h"
#include "msmAddr.h"
#define CORE_NUM    1

#define AXIS_NUM    7
#define MACRO_X     0
#define MICRO_X     1
#define MICRO_Y     2
#define MICRO_Rz    3

#define AXIS_X      0
#define MOTOR_EP    6


#define MICRO_Z     4
#define MICRO_Rx    5
#define MICRO_Ry    6

#define MAX_ORDER   5                   // 预定控制器最高阶数

typedef struct{
    double S;               // 位移
    double V;               // 速度
    double A;               // 加速度
    double J;               // 加速度一阶导
    double C;               // 加速度二阶导
    double P;               // 加速度三阶导
}SParaStruct;


typedef struct{
    double ruler1;          // 光栅尺通道1数据
    double ruler2;          // 光栅尺通道2数据
    double ruler3;          // 光栅尺通道3数据
    double ruler4;          // 光栅尺通道4数据
    double ruler5;          // 光栅尺通道5数据
    double ruler6;          // 光栅尺通道6数据
    double ruler7;          // 光栅尺通道7数据
    double ruler8;          // 光栅尺通道8数据
    double adChannel1;      // AD通道1数据
    double adChannel2;      // AD通道2数据
    double adChannel3;      // AD通道3数据
    double adChannel4;      // AD通道4数据
    double adChannel5;      // AD通道5数据
    double adChannel6;      // AD通道6数据
    double adChannel7;      // AD通道7数据
    double adChannel8;      // AD通道8数据
    double interfer1;       // 干涉仪通道1数据
    double interfer2;       // 干涉仪通道2数据
    double interfer3;       // 干涉仪通道3数据
    double interfer4;       // 干涉仪通道4数据
    double interfer5;       // 干涉仪通道5数据
    double interfer6;       // 干涉仪通道6数据
}SensorDataStruct;
typedef struct{
    double temperature1;   //TD4015通道1
    double temperature2;   //TD4015通道2
    double temperature3;   //TD4015通道3
    double temperature4;   //TD4015通道4
    double temperature5;   //TD4015通道5
    double temperature6;   //TD4015通道6

    double voltage1;
    double voltage2;
    double voltage3;
    double voltage4;
    double voltage5;
    double voltage6;
    double voltage7;
    double voltage8;
    double voltage9;
    double voltage10;
    double voltage11;
    double voltage12;
    double voltage13;
    double voltage14;
    double voltage15;
    double voltage16;
}EnvironmentData;

typedef struct{
    int order;                              // 阶次
    double controlOutput;                   // 控制量输出
    double parameter[2*MAX_ORDER + 1];      // 参数
    double controlErr[MAX_ORDER];           // 误差
    double controlU[MAX_ORDER];             // 控制量
}ControlParaStruct;


#if CORE_NUM == 0
typedef struct{
    unsigned char runFlagLast;              // 上一次的运动标志
    unsigned char runFlag;                  // 运动标志
    unsigned char refreshFlag;              // 参数更新标志
    unsigned char runFinishFlag;            // 运动完成标志
    SensorDataStruct sensorData;            // 传感器数据
}ConfigStruct;
#elif CORE_NUM == 7
typedef struct{
    unsigned char runFlag;                  // 运动标志，开始控制置位
}ConfigStruct;
#endif
//typedef struct{
//    unsigned char runFlag;                  // 运动标志，开始控制置位
//    unsigned char runFinishFlag;            // 运动完成标志，到达指定点之后置位
//    unsigned char refreshFlag;              // 参数更新标志，用于更新起始位置，首次更新置位
//    SParaStruct sParaSpecific;              // S曲线规划参数，CORE0指定
//    SParaStruct currentPointSPara;          // 当前的运动各阶参数，用于上传或者前馈
//    SParaStruct nextPointSPara;             // 下一个运动点的各阶参数，下一个点的运动参数
//    ControlParaStruct controlPara;          // 控制器参数，控制量计算
//}ConfigStruct;

typedef struct{
    unsigned char runFlag;                  // 运动标志，开始控制置位
    unsigned char runFinishFlag;
    unsigned char refreshFlag;              // 参数更新标志，用于更新起始位置，首次更新置位
    SParaStruct sParaSpecific;              // S曲线规划参数，CORE0指定
    SParaStruct currentPointSPara;          // 当前的运动各阶参数，用于上传或者前馈
    SParaStruct nextPointSPara;             // 下一个运动点的各阶参数，下一个点的运动参数
    ControlParaStruct controlPara;          // 控制器参数，控制量计算
}CommandConfig;

//typedef struct{
//    double arr[10]
//}Base;

typedef enum{
    MacroLogicalX = 0,
    MacroLogicalY,
    MacroLogicalRz,

    MicroLogicalX,
    MicroLogicalY,
    MicroLogicalRz,
    MicroLogicalZ,
    MicroLogicalRx,
    MicroLogicalRy,

    AxisCount//永远放在最后一个勿动
}Axis;
typedef enum{
    Task1 = 0,
    Task2,
    Task3,
    Task4,

    TaskCount
}Task;
typedef struct{
    Axis axis;
    Task task;
    CommandConfig config;
}Command;
typedef struct{
    Command CommandArray[3];
}CommandPackage;
typedef enum{
    CORE1 = 0,
    CORE2,
    CORE3,
    CORE4,
    CORE5,
    CORE6,

    CoreCount
}CoreNum;
typedef enum{
    Slot1 = 0,
    Slot2,
    Slot3,

    SlotCount
}Slot;
typedef enum{
    LongStrokeX = 0,
    LongStrokeY1,
    LongStrokeY2,
    ShortStrokeX,
    ShortStrokeY1,
    ShortStrokeY2,
    ShortStrokeZ1,
    ShortStrokeZ2,
    ShortStrokeZ3,

    MotorCount
}Motor;

typedef enum{
    Silence = 0,
    Activate,
    Default,
    Initialize,
    Stop,
    SpecificTask

}State;

typedef unsigned long long int uint64;
struct IfmCommand{
    uint64 IfmPressureSensorInit;
    uint64 IfmPosClear;
    uint64 IfmPosDecouple;
};
struct ControlCommand{
    uint64 PosSampleFreq;
    uint64 PosSampleDelayTime;
    uint64 WsStepScan;
    uint64 ProberEnable;
    uint64 ProberInterval;
    uint64 ProberTimes;
    uint64 ed_delay;
    uint64 ed_freq;
    uint64 ed_times;
    uint64 SensorMode;
    uint64 RzFeedbackMode;
    uint64 ControllerMode;
};
struct EleCommand{
    uint64 PowerSourceEnable;
    uint64 MotorActuatorEnable;
};
struct VxCommand{
    struct IfmCommand  IfmCmd;
    struct ControlCommand ControlCmd;
    struct EleCommand  EleCmd;
};
struct VxFlag{
    uint64 StateFlag;
    uint64 CmdrefreshFlag;
    uint64 IfmrefreshFlag;
};
typedef struct{
    struct VxFlag vxFlag;
    struct VxCommand vxCmd;
    SParaStruct sParaSpecific[AxisCount];
}InteractionStruct;
typedef enum{
    Free = 0,
    Acc ,
    Uni,
    Dec,
    Loc
}MovePhase;
typedef struct{
    State CurState;
    SensorDataStruct sensorData;            // 传感器数据
    EnvironmentData  EnvData;               //
    SParaStruct  *AxisSCurvePara[AxisCount];
    SParaStruct  *AxisSCurveCurPoint[AxisCount];
    SParaStruct  *AxisSCurveNextPoint[AxisCount];
    ControlParaStruct *AxisControlPara[AxisCount];
    CommandConfig *AxisCommandConfig[AxisCount];
    Command *AxisCommand[AxisCount];
    double MotorOutput[MotorCount];
    unsigned long long int ErrorCode;
    MovePhase Phase[AxisCount];
    struct ControlCommand ControlCmd;
}ShareData;

typedef ShareData *ShareHandle;
typedef SParaStruct* SParaHandle;
typedef SensorDataStruct* SensorDataHandle;
typedef ControlParaStruct* ControlParaHandle;
typedef InteractionStruct* InteractionHandle;
typedef CommandPackage *CommandPackageHandle;
typedef Command *CommandHandle;

extern ShareData *MultiCoreShareData;
extern SParaHandle SParaSpecific[AXIS_NUM];             // S曲线参数指定核间变量
extern SParaHandle SParaCurrentData[AXIS_NUM];          // S曲线参数当前值核间变量
extern SensorDataHandle SensorData;                     // 传感器数据核间变量
extern ControlParaHandle ControlPara[AXIS_NUM];         // 控制器参数核间变量
extern InteractionHandle VxRawConfig;             // VxWorks交互变量
extern CommandPackageHandle pCommandPackage;

#define CURSLOTCONFIG(slotnum) (CommandConfig *)(&(pCommandPackage->CommandArray[slotnum].config))


inline void SetScurveParam(Axis axis,double S,double V,double A,
                                     double J,double C,double P)
{
    SParaSpecific[axis]->S = S;
    SParaSpecific[axis]->V = V;
    SParaSpecific[axis]->A = A;
    SParaSpecific[axis]->J = J;
    SParaSpecific[axis]->C = C;
    SParaSpecific[axis]->P = P;
}
inline void SetScurveParamWithoutS(Axis axis,double V,double A,
                                   double  J,double C,double P)
{
    SParaSpecific[axis]->V = V;
    SParaSpecific[axis]->A = A;
    SParaSpecific[axis]->J = J;
    SParaSpecific[axis]->C = C;
    SParaSpecific[axis]->P = P;
}

inline SParaHandle GetAxisSCurveParam(Axis axis)
{
    return MultiCoreShareData->AxisSCurvePara[axis];
}
inline SParaHandle GetAxisSCurveCurPoint(Axis axis)
{
    return MultiCoreShareData->AxisSCurveCurPoint[axis];
}
inline SParaHandle GetAxisSCurveNextPoint(Axis axis)
{
    return MultiCoreShareData->AxisSCurveNextPoint[axis];
}
inline ControlParaHandle GetAxisControlParam(Axis axis)
{
    return MultiCoreShareData->AxisControlPara[axis];
}
inline CommandConfig *GetAxisCommandConfig(Axis axis)
{
    return MultiCoreShareData->AxisCommandConfig[axis];
}

inline void SetAxisSCurveParam(Axis axis,double S,double V,double A,
                                         double J,double C,double P)
{
    SParaHandle temp = GetAxisSCurveParam(axis);
    temp->S = S;
    temp->V = V;
    temp->A = A;
    temp->J = J;
    temp->C = C;
    temp->P = P;
}

inline void SetAxisSCurveParamWithoutS(Axis axis,double V,double A,
                                       double J,double C,double P)
{
    SParaHandle temp = GetAxisSCurveParam(axis);
    temp->V = V;
    temp->A = A;
    temp->J = J;
    temp->C = C;
    temp->P = P;
}
inline State GetSystemCurState()
{
    return MultiCoreShareData->CurState;
}
inline void SetAxisControllerParam(Axis axis,double *param,int n)
{
   ControlParaHandle temp = GetAxisControlParam(axis);
   memcpy(temp->parameter, param, n*8);
}
inline CommandPackage *GetCurCoreCmdPackage()
{
    return (CommandPackage *)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE*DNUM);
}
#if CORE_NUM == 0
extern Uint8 AuroraBufferAD[256];
extern Uint8 AuroraBufferIndexAD[64];
extern Uint8 AuroraBufferRuler[256];
extern Uint8 AuroraBufferIndexRuler[64];
#elif CORE_NUM == 7
extern Uint8 AuroraBufferDA[256];
extern Uint8 AuroraBufferIndexDA[64];
#endif

/*
 * 与VxWorks通信
 */

#if CORE_NUM == 7
extern double CommitData[900];
#endif
/*
 * 函数声明
 */
void initConfig();
#endif /* GLOBAL_H_ */
