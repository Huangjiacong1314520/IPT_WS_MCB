/*
 * global.c
 *
 *  Created on: 2021Äê8ÔÂ22ÈÕ
 *      Author: lkx
 */
#include "global.h"
#include "msmAddr.h"

CommandPackageHandle pCommandPackage;
ShareData *MultiCoreShareData;

SensorDataHandle SensorData;

InteractionHandle VxRawConfig;

//struct Base base_array[SlotCount];

//double sparam[2][6];
#if CORE_NUM == 7
#ifdef  __cplusplus
#pragma DATA_SECTION("CommitData")
#pragma  DATA_ALIGN(128)
#else
#pragma  DATA_SECTION(CommitData,  "CommitData")
#pragma  DATA_ALIGN(CommitData, 128)
#endif
double   CommitData[900];
#endif

#if CORE_NUM == 0

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

#elif CORE_NUM == 7
#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma DATA_ALIGN(128)
#else
#pragma DATA_SECTION(AuroraBufferDA,  "AuroraData")
#pragma DATA_ALIGN(AuroraBufferDA,   128)
#endif
Uint8  AuroraBufferDA[256] = {0};

#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma  DATA_ALIGN(16)
#else
#pragma  DATA_SECTION(AuroraBufferIndexDA,  "AuroraData")
#pragma  DATA_ALIGN(AuroraBufferIndexDA,   16)
#endif
Uint8 AuroraBufferIndexDA[PacketBodyMax] = {0};
#endif



void initConfig(){
    VxRawConfig = (InteractionHandle)(MSM_MAP_BASEADDR);
    MultiCoreShareData = (ShareData *)(MSM_SHAREDATA_BASEADDR);
    SensorData = (SensorDataHandle)(MSM_SHAREDATA_BASEADDR + OFFSET(ShareData,sensorData));
//    for(i = 1; i <= AXIS_NUM; i++){
//        SParaSpecific[i-1] = (SParaHandle)(0x0C200000 + 0x00040000*i + 8);
//        SParaCurrentData[i-1] = (SParaHandle)(0x0C200000 + 0x00040000*i + 8 + sizeof(SParaStruct));
//        ControlPara[i-1] = (ControlParaHandle)(0x0C200000 + 0x00040000*i + 8 + sizeof(SParaStruct)*3);
//    }
    pCommandPackage = (CommandPackageHandle)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE*DNUM);

}
