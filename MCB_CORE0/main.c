    /*
 * main.c
 *
 *  Created on: 2021年8月12日
 *      Author: lkx
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interfacedrive.h"
#include "char_decimal1.h"
#include "TestDriver.h"
#include <assert.h>
#include "drv_srio.h"
#include "srio/srio.h"
#include "cslr_cgem.h"
#include "csl_cacheAux.h"
#include "control/global.h"
#include "control/interrupt.h"
#include "src/intc/csl_intc.h"
#include "src/intc/csl_intcAux.h"
#include "csl_ipc.h"
#include "csl_ipcAux.h"
#include "kern/macros.h"
#include "hw_soc_C6678.h"
#include "control/connector.h"
#include "ti/csl/csl_xmcAux.h"
#include "control/msmAddr.h"
#include "peripheral/tempAcquisition.h"
#include "control/list.h"
/**
 *
 * CORE 0
 * */
CSL_IPCRegs *ipcroot = (CSL_IPCRegs *)(0x02620000+0x200);
void DisableDDR3CachePrefetch();
static void IsrDoorbell(void* hand);
int InitSrioIntc();
extern int WriteRegCommon(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannelSel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint8 uChannel, const Uint32* pData, AuroraPacket* pPkt );
extern int ReadRegCommon(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannelSel, Uint8  cTransactionType, Uint8  uChannel, AuroraPacket* pPkt);
void SetRulerMode();

int SRIOREADY = 0;
//static void XMCtest();
//static int MSMtest();
static void Functiontest();
//static void Rs485test();
//static void RulerTest();
static void Sriotest();
static void OffsetTest();

static void SystemReset();
static void initSystemSrio();
static void WaitOtherCores();
static void TdReset();

void main(void){
  //  Functiontest();
   // OffsetTest();
    SystemReset();
   // XMCtest();
    initSystemSrio();
   // delay_ms(1000);
   // Sriotest();
   // TdReset();

    InitSrioIntc();
   //  Sriotest();
    WaitOtherCores();

    INTCENABLE(4);
    while(1){
        ;
    }
}
static void Sriotest()
{
//    int i = 0;
//    for (i = 0;i < 100;++i) {
//        TransferSysInfo();
//    }
//
//    for (i = 0;i < 100;++i) {
//        TransferSysInfo();
//    }
//
//    for (i = 0;i < 100;++i) {
//        TransferSysInfo();
//    }
//
//    for (i = 0;i < 100;++i) {
//        TransferSysInfo();
//    }
//
//    for (i = 0;i < 100;++i) {
//        TransferSysInfo();
//    }
//    char *target = (char *)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE*6);
//    double temp;
//
//    while(1) {
//        SRIOReadDataLSU(0x70, 0x0, PADDR(target), SRIO_PACKET_TYPE_NRead, &temp, 8);
//        delay_ms(2);
//    }

}
static void SystemReset()
{
    delay_ms(50);
    *((volatile Uint32*) (0x2620000 + 0x38)) = 1;          //kick0
    *((volatile Uint32*) (0x2620000 + 0x3C)) = 1;          //kick1
    SRIOSoftReset();
    Global_Default_Init();
    DisableDDR3CachePrefetch();
    MSMRemap();
    initConfig();
}
static void initSystemSrio()
{
    const unsigned char cLocalDevID = 0x30;
    SRIOInitialize(cLocalDevID, SRIO_NO_LOOPBACK);
    P2busCfg(0x0, 0x21, 0x8);
    SRIOClearAllIntcStatus();
    SRIODoorbellRouteCtl(0);
    {
        int i = 0;
        for (i = 0; i < 7; i++)
        {
           SRIODoorbellIntcRouteSet(i, 0);
        }

        delay_ms(20);
    }
    //初始化与主卡以及P2BUS通信
    unsigned char buf[256] = {0};
    SRIOWriteDataLSU(0x10,0x0,0xd0000040, SRIO_PACKET_TYPE_STREAMWRITE,((Uint8*)buf),256);
    SRIO_Maintenance(0x2,0x0,0x10,0xd0000000,(unsigned int)buf,SRIO_PACKET_TYPE_MAINTENANCE_WRITE);
   // P2busRead(buf, 1);

}
static void TdReset()
{
    SEMDOWN(0);
    Td_init(0x2,0xFFF,0x7, 9600);
    Td_init(0x2,0xFFF,0xB, 9600);
    Td_init(0x2,0xFFF,0xC, 9600);

//    TDWrite(0x4, 0x7,0xD8,0x1, 0x7);  // 0x7为115200波特率；0xD8为波特率设置寄存器
//    Td_init(0x4, 0xFF, 0x7, 115200);
//    Td_init(0x4, 0xFF, 0x8, 115200);
//    Td_init(0x4, 0xFF, 0x9, 115200);
//    delay_ms(5000);
    //上电之后的第一次运行首先收集环境信息。
    TdReadTopHalf(0x2,0x7, 0x1,6,PT100);
    TdReadTopHalf(0x2,0xB,0x1,8,voltage);
    TdReadTopHalf(0x2,0xC,0x1,8,voltage);
    char *baseaddr = ((char *)&Sysinfo + OFFSET(SystemInfo,EnvData));
    double *temp = (double *)(baseaddr + OFFSET(EnvironmentData,temperature1));
    TDReadBottomHalf(0x2,0x7,6,temp);
    temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage1));
    TDReadBottomHalf(0x2,0xB,8,temp);
    temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage9));
    TDReadBottomHalf(0x2,0xC,8,temp);
    while(1) {
        TdReadTopHalf(0x4,0x7, 0x1,6,PT100);
        TdReadTopHalf(0x4,11,0x1,8,voltage);
        TdReadTopHalf(0x4,12,0x1,8,voltage);
//        TDReadBottomHalf(0x4,0x7,6,temp);
//        TDReadBottomHalf(0x4,11,8,temp);
//        TDReadBottomHalf(0x4,12,8,temp);
    }
//    delay_ms(5000);
}
static void WaitOtherCores()
{
    SEMDOWN(0);
    WAIT(CORE1INIT);
    WAIT(CORE2INIT);
    WAIT(CORE3INIT);
    WAIT(CORE4INIT);
    WAIT(CORE5INIT);
    WAIT(CORE6INIT);
    WAIT(CORE7INIT);
    delay_ms(20);
    SRIOClearDoorbellIntcStatus(1);
    SEMUP(0);
}

static void Functiontest()
{
    SParaStruct param = {1,2,3,4,5,6};
    SParaStruct storge;
    unsigned long long int begin = GetCurTime();
    int i;
    for (i = 0;i < 10000;++i) {
        QueuePush(NULL,NULL);
        QueuePush(NULL,NULL);
        DeleteQueue();
    }
    unsigned long long int end = GetCurTime();
    printf("consume %lld\n",(end-begin)/10000);
}
#define NAME(x) #x
static void OffsetTest()
{
//    printf("%s offset is 0x%x\n",NAME(StateFlag),OFFSET(InteractionStruct,vxFlag) + OFFSET(struct VxFlag,StateFlag));
//    printf("%s offset is 0x%x\n",NAME(CmdrefreshFlag),OFFSET(InteractionStruct,vxFlag) + OFFSET(struct VxFlag,CmdrefreshFlag));
//    printf("%s offset is 0x%x\n",NAME(IfmrefreshFlag),OFFSET(InteractionStruct,vxFlag) + OFFSET(struct VxFlag,IfmrefreshFlag));
//    printf("%s offset is 0x%x\n",NAME(PosSampleFreq),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,PosSampleFreq));
//    printf("%s offset is 0x%x\n",NAME(PosSampleDelayTime),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,PosSampleDelayTime));
//    printf("%s offset is 0x%x\n",NAME(WsStepScan),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,WsStepScan));
//    printf("%s offset is 0x%x\n",NAME(ProberEnable),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,ProberEnable));
//    printf("%s offset is 0x%x\n",NAME(ProberInterval),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,ProberInterval));
//    printf("%s offset is 0x%x\n",NAME(ProberTimes),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,ProberTimes));
//
//    printf("%s offset is 0x%x\n",NAME(SensorMode),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,SensorMode));
//    printf("%s offset is 0x%x\n",NAME(RzFeedbackMode),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,RzFeedbackMode));
//    printf("%s offset is 0x%x\n",NAME(ControllerMode),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,ControlCmd) + OFFSET(struct ControlCommand,ControllerMode));
//    printf("%s offset is 0x%x\n",NAME(IfmPressureSensorInit),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,IfmCmd) + OFFSET(struct IfmCommand,IfmPressureSensorInit));
//    printf("%s offset is 0x%x\n",NAME(IfmPosClear),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,IfmCmd) + OFFSET(struct IfmCommand,IfmPosClear));
//    printf("%s offset is 0x%x\n",NAME(IfmPosDecouple),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,IfmCmd) + OFFSET(struct IfmCommand,IfmPosDecouple));
//    printf("%s offset is 0x%x\n",NAME(PowerSourceEnable),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,EleCmd) + OFFSET(struct EleCommand,PowerSourceEnable));
//    printf("%s offset is 0x%x\n",NAME(MotorActuatorEnable),OFFSET(InteractionStruct,vxCmd) + OFFSET(struct VxCommand,EleCmd) + OFFSET(struct EleCommand,MotorActuatorEnable));
//    printf("%s offset is 0x%x\n",NAME(sParaSpecific),OFFSET(InteractionStruct,sParaSpecific[0]));
    printf("%s offset is 0x%x\n",NAME(SensorMode),OFFSET(struct VxCommitData,SensorMode));
    printf("%s offset is 0x%x\n",NAME(RzFBMode),OFFSET(struct VxCommitData,RzFBMode));
    printf("%s offset is 0x%x\n",NAME(LogicalAxisMeaurement),OFFSET(struct VxCommitData,LogicalAxisMeaurement[0]));
    printf("%s offset is 0x%x\n",NAME(BasicSensorData),OFFSET(struct VxCommitData,BasicSensorData[0]));
    printf("%s offset is 0x%x\n",NAME(IfmDecoupleFeedback),OFFSET(struct VxCommitData,IfmDecoupleFeedback[0]));
    printf("%s offset is 0x%x\n",NAME(Channel),OFFSET(struct VxCommitData,Channel[0]));
}
int InitSrioIntc(){
    INTC_Init();
    INTC_CIC_Open(0);

    if (0 != INTC_Open(20, (int)4, IsrDoorbell, NULL)){
        return -1;
    }
    CSL_intcEventEnable(20);
    return 0;
}

int j = 0;
unsigned  long int start = 0;
unsigned  long int stop = 0;

static void IsrDoorbell(void* hand){
        CSL_intcEventDisable(20);
        unsigned int nStatus = SRIOGetDoorbellStatus();
        unsigned char dbIntcStatus = PowerCalculate(nStatus);
        if (nStatus != 2)
            goto IsrEnd;

        IsrMotionControl();

IsrEnd:
        SRIOClearDoorbellIntcStatus(dbIntcStatus);
        CSL_intcEventClear(20);
        CSL_intcEventEnable(20);
}


void DisableDDR3CachePrefetch(){
    CSL_CgemRegs *C66xCorePacRegs;
    C66xCorePacRegs = (CSL_CgemRegs *)CSL_CGEM0_5_REG_BASE_ADDRESS_REGS;

    CACHE_invAllL1p(CACHE_WAIT);
    CACHE_wbInvAllL1d(CACHE_WAIT);
    CACHE_wbInvAllL2(CACHE_WAIT);
    _mfence();
    _mfence();

    // 配置 DDR3 内存为不可被缓存和不可预取
    int i = 0;
    for(i = 16; i < 256; i++){
        C66xCorePacRegs->MAR[i] = 0;
    }
}
