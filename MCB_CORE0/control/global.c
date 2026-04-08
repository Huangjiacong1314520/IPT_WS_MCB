/*
 * global.c
 *
 *  Created on: 2021Äê8ÔÂ22ÈÕ
 *      Author: lkx
 */
#include "global.h"
#include "msmAddr.h"



#define TOTALCORENUM 6
#define TOTALAXISNUM 6
CommandPackage *pCommandPackageArray[CoreCount];
ShareData *MultiCoreShareData;
SParaStruct *pSParamCache[AxisCount] = {NULL};

SensorDataHandle SensorData;
//ControlParaHandle ControlPara[TOTALAXISNUM];
InteractionHandle VxRawConfig;
InteractionHandle VxLatchConfig;

struct VxCommitData *pReadyBuf = NULL;
struct VxCommitData *pCurrentBuf = NULL;


int IFClearZero_Ctrl=0;
unsigned int RulerState[8];



#pragma  DATA_SECTION(CommitBuf,  "CommitData")
#pragma  DATA_ALIGN(CommitBuf,8);
struct VxCommitData CommitBuf[2];

#pragma  DATA_SECTION(Sysinfo,  "SystemShareData")
#pragma  DATA_ALIGN(Sysinfo, 8)
SystemInfo   Sysinfo;

#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma DATA_ALIGN(128)
#else
#pragma DATA_SECTION(AuroraBufferAD,  "AuroraData")
#pragma DATA_ALIGN(AuroraBufferAD, 128)
#endif
Uint8  AuroraBufferAD[256] = {0};

#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma  DATA_ALIGN(16)
#else
#pragma  DATA_SECTION(AuroraBufferIndexAD,  "AuroraData")
#pragma  DATA_ALIGN(AuroraBufferIndexAD,   16)
#endif
Uint8  AuroraBufferIndexAD[PacketBodyMax] = {0};

#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma DATA_ALIGN(128)
#else
#pragma DATA_SECTION(AuroraBufferRuler,  "AuroraData")
#pragma DATA_ALIGN(AuroraBufferRuler,   128)
#endif
Uint8  AuroraBufferRuler[256] = {0};

#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma  DATA_ALIGN(16)
#else
#pragma  DATA_SECTION(AuroraBufferIndexRuler,  "AuroraData")
#pragma  DATA_ALIGN(AuroraBufferIndexRuler,   16)
#endif
Uint8  AuroraBufferIndexRuler[PacketBodyMax] = {0};




#pragma  DATA_SECTION(Interferometerbuf,  "Interferometer")
#pragma  DATA_ALIGN(Interferometerbuf,   8)
Uint8 Interferometerbuf[256] = {0};
//void initConfig(){
//    int i = 0;
//    InteractionConfig = (InteractionHandle)(0x0C000000);//
//    Config = (ConfigHandle)(0x0C200000 + 0x00040000*CORE_NUM);
//    SensorData = (SensorDataHandle)(0x0C200000 + 8);//
//    for(i = 1; i <= AXIS_NUM; i++){
//        SParaSpecific[i-1] = (SParaHandle)(0x0C200000 + 0x00040000*i + 8);
//        SParaCurrentData[i-1] = (SParaHandle)(0x0C200000 + 0x00040000*i + 8 + sizeof(SParaStruct));
//        ControlPara[i-1] = (ControlParaHandle)(0x0C200000 + 0x00040000*i + 8 + sizeof(SParaStruct)*3);
//    }
//    SParaSpecific[MOTOR_EP] = (SParaHandle)(0x0C200000 + 0x00040000*1 + sizeof(ConfigStruct_motion_core) + 8);
//    SParaCurrentData[MOTOR_EP] = (SParaHandle)(0x0C200000 + 0x00040000*1 + 8 + sizeof(ConfigStruct_motion_core) + sizeof(SParaStruct));
//    ControlPara[MOTOR_EP] = (ControlParaHandle)(0x0C200000 + 0x00040000*1 + 8 + sizeof(ConfigStruct_motion_core) + sizeof(SParaStruct)*3);
//    #if CORE_NUM == 0
//        memset((void*)0x0C000000, 0, 0x00400000);
//    #elif CORE_NUM == 7
//        memset((void*)0x83800000, 0, 0x00001C20);
//    #endif
//}

inline void initCommand(CoreNum corenum,Slot slotnum,Axis axis)
{
    CommandHandle CommandBaseAddr = &pCommandPackageArray[corenum]->CommandArray[slotnum];
    CommandBaseAddr->axis = axis;
   // CommandBaseAddr->step = Step1;
    CommandBaseAddr->task = Task1;
    CommandConfig *ConfigBaseAddr = (CommandConfig *)((char *)CommandBaseAddr + OFFSET(Command,config));

    MultiCoreShareData->AxisCommand[axis] = CommandBaseAddr;
    MultiCoreShareData->AxisCommandConfig[axis] = ConfigBaseAddr;
    MultiCoreShareData->AxisSCurvePara[axis] = (SParaHandle)((char *)ConfigBaseAddr + OFFSET(CommandConfig,sParaSpecific));
    MultiCoreShareData->AxisSCurveCurPoint[axis] = (SParaHandle)((char *)ConfigBaseAddr + OFFSET(CommandConfig,currentPointSPara));
    MultiCoreShareData->AxisSCurveNextPoint[axis] = (SParaHandle)((char *)ConfigBaseAddr + OFFSET(CommandConfig,nextPointSPara));
    MultiCoreShareData->AxisControlPara[axis] = (ControlParaHandle)((char *)ConfigBaseAddr + OFFSET(CommandConfig,controlPara));
}
//inline void ArrayStorgeAlloc(char *baseaddr)
//{
//    int Axisnum = GetAxisCount();
//    int Motornum = GetMotorCount();
//    char *StorgeBaseAddr = (char *)(baseaddr + sizeof(ShareData));
//    MultiCoreShareData->AxisSCurvePara = (SParaStruct  **)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(SParaStruct *)*Axisnum;
//    MultiCoreShareData->AxisSCurveCurPoint = (SParaStruct  **)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(SParaStruct *)*Axisnum;
//    MultiCoreShareData->AxisSCurveNextPoint = (SParaStruct  **)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(SParaStruct *)*Axisnum;
//    MultiCoreShareData->AxisControlPara = (ControlParaStruct  **)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(ControlParaStruct  *)*Axisnum;
//    MultiCoreShareData->AxisCommandConfig = (CommandConfig **)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(CommandConfig  *)*Axisnum;
//    MultiCoreShareData->AxisCommand = (Command **)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(Command *)*Axisnum;
//    MultiCoreShareData->MotorOutput = (double *)(StorgeBaseAddr);
//    StorgeBaseAddr += sizeof(double *)*MotorCount;
//    assert(StorgeBaseAddr <= (char *)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE));
//}
void initConfig(){
    int i;
    int Corenum = GetCoreCount();

    VxRawConfig = (InteractionHandle)(MSM_MAP_BASEADDR);
    VxLatchConfig = (InteractionHandle)(MSM_VXCMDLATCH_BASEADDR);
    pCurrentBuf = &CommitBuf[0];
    MultiCoreShareData = (ShareData *)(MSM_SHAREDATA_BASEADDR);
    SensorData = (SensorDataHandle)(MSM_SHAREDATA_BASEADDR + OFFSET(ShareData,sensorData));
    memset((void *)MSM_MAP_BASEADDR, 0, MSMTOTALSIZE);
   // ArrayStorgeAlloc((char *)MultiCoreShareData);
    for (i = 0;i < Corenum; ++i) {
        pCommandPackageArray[i] = (CommandPackage *)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE*(i+1));
        switch(i) {
        case CORE1:
            initCommand(CORE1,Slot1,MacroLogicalX);
            initCommand(CORE1,Slot2,MacroLogicalY);
            initCommand(CORE1,Slot3,MacroLogicalRz);
            break;
        case CORE2:

            initCommand(CORE2,Slot1,MicroLogicalX);
            initCommand(CORE2,Slot2,MicroLogicalY);
            initCommand(CORE2,Slot3,MicroLogicalRz);
            break;
        case CORE3:
            initCommand(CORE3,Slot1,MicroLogicalZ);
            initCommand(CORE3,Slot2,MicroLogicalRx);
            initCommand(CORE3,Slot3,MicroLogicalRy);
            break;
        case CORE4:
            break;
        case CORE5:
            break;
        case CORE6:
            break;
        default:
            break;
        }
    }
    memset(CommitBuf, 0, sizeof(struct VxCommitData)*2);
}
