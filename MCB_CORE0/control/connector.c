/*
 * interface.c
 *
 *  Created on: 2022Äê5ÔÂ24ÈÕ
 *      Author: Administrator
 */
#include "connector.h"
#include "list.h"
#include <c6x.h>
#include <ti/csl/csl_xmcAux.h>
#include <ti/csl/csl_xmc.h>
#include "msmAddr.h"
#include "../peripheral/tempAcquisition.h"
//static int isResetFinished = 0;
static int isRefreshFinished = 0;
static int count = 0;
static unsigned long long int startTime = 0;
static uint16_t srio_isr_info = 0x3000;
static int isEnableTransfer = 0;
//static int isFirst = 0;
void ClearGlobalVar()
{
   // isResetFinished = 0;
    isEnableTransfer = 0;
    isRefreshFinished = 0;
 //   isFirst = 0;
    srio_isr_info = 0x3000;
    pCurrentBuf = &CommitBuf[0];
    count = 0;
    if (MultiCoreShareData->CurState == Default)
        DeleteQueue();
    if (!CSL_semIsFree(0))
        SEMUP(0);
//    if (!CSL_semIsFree(20))
//        SEMUP(20);
}

//void IsrPrologue(State curstate)
//{
////    static int timecount = 0;
////    if (timecount == 0) {
////        TdReadTopHalf(0x4, 0x7, 0x1,6,PT100);
////        TdReadTopHalf(0x4,0x8,0x1,8,voltage);
////        TdReadTopHalf(0x4,0x9,0x1,8,voltage);
////    }
////    if (++timecount == 5200){
////        char *baseaddr = ((char *)&Sysinfo + OFFSET(SystemInfo,EnvData));
////        double *temp = (double *)(baseaddr + OFFSET(EnvironmentData,temperature1));
////        TDReadBottomHalf(0x4,0x7,6,temp);
////        temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage1));
////        TDReadBottomHalf(0x4,0x8,8,temp);
////        temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage9));
////        TDReadBottomHalf(0x4,0x9,8,temp);
////        timecount = 0;
////    }
////    ReadRulerTopHalf(0x2);
////    ReadADTopHalf(0x1);
////    ReadInfTopHalf();
//    switch(curstate){
//    case Silence:
//        break;
//    case Activate:
//        break;
//    case Default:
//    case Initialize:
//    case SpecificTask:
//        //    if (timecount == 0) {
//        //        TdReadTopHalf(0x4, 0x7, 0x1,6,PT100);
//        //        TdReadTopHalf(0x4,0x8,0x1,8,voltage);
//        //        TdReadTopHalf(0x4,0x9,0x1,8,voltage);
//        //    }
//        //    if (++timecount == 5200){
//        //        char *baseaddr = ((char *)&Sysinfo + OFFSET(SystemInfo,EnvData));
//        //        double *temp = (double *)(baseaddr + OFFSET(EnvironmentData,temperature1));
//        //        TDReadBottomHalf(0x4,0x7,6,temp);
//        //        temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage1));
//        //        TDReadBottomHalf(0x4,0x8,8,temp);
//        //        temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage9));
//        //        TDReadBottomHalf(0x4,0x9,8,temp);
//        //        timecount = 0;
//        //    }
//        //    ReadRulerTopHalf(0x2);
//        //    ReadADTopHalf(0x1);
//        break;
//    case Stop:
//        break;
//    }
//    startTime= GetCurTime();
//
//}
void IsrPrologue()
{
    startTime= GetCurTime();
    MultiCoreShareData->ErrorCode = 0;
}
void ReadSensorTopHalf()
{
    //    if (timecount == 0) {
    //        TdReadTopHalf(0x4, 0x7, 0x1,6,PT100);
    //        TdReadTopHalf(0x4,0x8,0x1,8,voltage);
    //        TdReadTopHalf(0x4,0x9,0x1,8,voltage);
    //    }
    //    if (++timecount == 5200){
    //        char *baseaddr = ((char *)&Sysinfo + OFFSET(SystemInfo,EnvData));
    //        double *temp = (double *)(baseaddr + OFFSET(EnvironmentData,temperature1));
    //        TDReadBottomHalf(0x4,0x7,6,temp);
    //        temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage1));
    //        TDReadBottomHalf(0x4,0x8,8,temp);
    //        temp = (double *)(baseaddr + OFFSET(EnvironmentData,voltage9));
    //        TDReadBottomHalf(0x4,0x9,8,temp);
    //        timecount = 0;
    //    }
        int r;
        r = ReadRulerTopHalf(0x2);
        r = ReadADTopHalf(0x1);
}
#define DSPCORESPEED 1000000000u
void CheckSensorDataTransfer(unsigned long long us)
{
    unsigned long long curTime;
    unsigned delayCycles;
    unsigned int dspCoreSpeed_HZ= 1000000000;
    delayCycles =((unsigned long long)us*dspCoreSpeed_HZ/1000000);
    do{
        curTime = GetCurTime();
    }while((curTime - startTime) < delayCycles);
    ReadRulerBottomHalf(0x2);
    ReadADBottomHalf(0x1);
    //ReadInfBottomHalf();
}

void IsrEpilogue(unsigned long long us)
{
    unsigned long long curTime;
    unsigned delayCycles;
    unsigned int dspCoreSpeed_HZ= 1000000000;
    delayCycles =((unsigned long long)us*dspCoreSpeed_HZ/1000000);
    do{
        curTime = GetCurTime();
    }while((curTime - startTime) < delayCycles);
    if (!CSL_semIsFree(7))
        SetErrorCode(CalculateTIMEOUT);
}
#undef DSPCORESPEED
void NotifyOtherCores()
{
    KickUnlock();
    CSL_IPC_genGEMInterrupt(1, 0);
    CSL_IPC_genGEMInterrupt(2, 0);
    CSL_IPC_genGEMInterrupt(3, 0);
    CSL_IPC_genGEMInterrupt(7, 0);
    KickLock();
}
void RulerDataProcess()
{
    double rulertemp = SensorData->ruler1;
    SensorData->ruler1 = rulertemp *5e-9;

    rulertemp = SensorData->ruler2;
    SensorData->ruler2 = rulertemp *5e-9;

    rulertemp = SensorData->ruler3;
    SensorData->ruler3 = rulertemp *5e-9;
}
void ADDataProcess()
{
    //ZH4
    double temp = SensorData->adChannel1;
    SensorData->adChannel1 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZV4
    temp = SensorData->adChannel2;
    SensorData->adChannel2 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZH3
    temp = SensorData->adChannel3;
    SensorData->adChannel3 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZV3
    temp = SensorData->adChannel4;
    SensorData->adChannel4 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZH2
    temp = SensorData->adChannel5;
    SensorData->adChannel5 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZV2
    temp = SensorData->adChannel6;
    SensorData->adChannel6 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZH1
    temp = SensorData->adChannel7;
    SensorData->adChannel7 = (temp/10.0*3.0 +0.3)*1e-3;
    //ZV1
    temp = SensorData->adChannel8;
    SensorData->adChannel8 = (temp/10.0*3.0 +0.3)*1e-3;

}
void SetAxisRunflag(Axis axis,unsigned char runflag)
{
    CommandConfig *temp = MultiCoreShareData->AxisCommandConfig[axis];
    temp->runFlag = runflag;
}
void SetallAxisRunflag(unsigned char runflag)
{
    int i;
    int Axisnum = GetAxisCount();
    for(i = 0;i < Axisnum;++i) {
        SetAxisRunflag((Axis)i, runflag);
    }
}
void SetAxisRefreshFlag(Axis axis,unsigned char refreshflag)
{
    CommandConfig *temp = MultiCoreShareData->AxisCommandConfig[axis];
    temp->refreshFlag = refreshflag;
}
void SetallAxisRefreshFlag(unsigned char refreshflag)
{
    int i;
    int Axisnum = GetAxisCount();
    for(i = 0;i < Axisnum;++i) {
        SetAxisRefreshFlag((Axis)i, refreshflag);
    }
}
int CheckSParamCacheEmpty()
{
//    int i;
//    for (i = 0;i < AxisCount;++i) {
//        if (pSParamCache[i] != NULL)
//            return -1;
//    }
    struct ListNode *temp = NULL;
    if ((temp = GetHead()) == NULL)
        return 0;
    if (temp->next == NULL)
        return 0;
    else
        return -1;
}
void RequestNewSParam()
{
    if (CSL_semIsFree(0)) {
       // SRIO_SendDoorBell(2, 0, 0x10, 0x3080);
        srio_isr_info |= 0x04;
        SEMDOWN(0);
    }
}
void RefreshParam()
{
    struct ListNode *temp = NULL;
    temp = GetHead();
    if (temp && temp->next) {
        QueuePop();
        temp = GetHead();
        int i;
        for (i = 0;i < AxisCount;++i) {
            memcpy(MultiCoreShareData->AxisSCurvePara[i],&temp->param[i],sizeof(SParaStruct));
            memcpy(&MultiCoreShareData->ControlCmd,&temp->cmd,sizeof(struct ControlCommand));
            SetAxisRefreshFlag((Axis)i,1);
        }
    }
}

void ReceiveControlCommand()
{
//    if (InteractionConfig->runFlag == 1) {
//        MultiCoreShareData->runFlag = 1;
//        SetallAxisRunflag(1);
//        if (!isRefreshFinished) {
//            isRefreshFinished = 1;
//            SetallAxisRefreshFlag(1);
//        }
//
//    }else {
//        MultiCoreShareData->runFlag = 0;
//        SetallAxisRunflag(0);
//        SetallAxisRefreshFlag(0);
//        ClearGlobalVar();
//        SwitchallAxisStep(Step1);
//    }
//    int i;
//    int Axisnum = GetAxisCount();
//    if (InteractionConfig->refreshFlag == 1) {
//        InteractionConfig->refreshFlag = 0x0;
//        for(i = 0; i < 6; i++){
//            pSParamCache[i] = &InteractionConfig->sParaSpecific[i];
//        }
//        if (!CSL_semIsFree(0)) {
//            SEMUP(0);
//        }
//    }
    if (GetVxRawRefreshFlag() == 1) {
        memcpy(VxLatchConfig, VxRawConfig, sizeof(InteractionStruct));
        VxRawConfig->vxFlag.CmdrefreshFlag = 0;
    }
    else {
        VxLatchConfig->vxFlag.CmdrefreshFlag = 0;
    }
}
void ParseControlCommand()
{
   // int i;
    if (GetVxLatchRefreshFlag() == 1) {
        MultiCoreShareData->CurState = GetVxLatchStateFlag();
        if (MultiCoreShareData->CurState== Initialize) {
            if (GetHead() == NULL)
                QueuePush(NULL,NULL);
        }
        else if (MultiCoreShareData->CurState == SpecificTask) {
//            for(i = 0; i < AxisCount; i++){
//                pSParamCache[i] = &VxLatchConfig->sParaSpecific[i];
//            }
//            Addtail(&VxLatchConfig->sParaSpecific[MicroLogicalZ],(struct ProberReg*)&VxLatchConfig->vxCmd.ControlCmd);
            QueuePush(VxLatchConfig->sParaSpecific, &VxLatchConfig->vxCmd.ControlCmd);
            if (!CSL_semIsFree(0)){
                SEMUP(0);
            }
        }
        else {
            ;
        }
    }

}
void InitMoveMode(State curstate)
{
    if(!isRefreshFinished){
        isRefreshFinished = 1;
        SetallAxisRefreshFlag(1);
    }
    if (curstate == Initialize)
        SwitchallAxisTask(Task1);
    else
        SwitchallAxisTask(Task2);
}
//static void NotifyVx(unsigned int destaddr,char *buf,unsigned int size);
void CommitData()
{


//    if (MultiCoreShareData->runFlag == 0) {
//        ClearGlobalVar();
//        return;
//    }

////...................................................
//    double ssz123 = (SensorData->adChannel8 * 0.490494296577947) + (SensorData->adChannel6 * 0.011886743696704) + (SensorData->adChannel4 * 0.497618959725350);
//    double ssz124 = -(SensorData->adChannel8 * 0.112502795795124) + (SensorData->adChannel6 * 0.509505703422053) + (SensorData->adChannel2 * 0.602997092373071);
//    double ssz134 = (SensorData->adChannel8 * 0.504898233057482) + (SensorData->adChannel4 * 0.509505703422053) - (SensorData->adChannel2 * 0.014403936479535);
//    double ssz234 = (SensorData->adChannel6 * 0.416663590387242) + (SensorData->adChannel4 * 0.092842113034811) + (SensorData->adChannel2 * 0.490494296577947);
//
//    double ssrx123 = (SensorData->adChannel8 * 0.0) + (SensorData->adChannel6 * 4.854368932038836) - (SensorData->adChannel4 * 4.854368932038835);
//    double ssrx124 = (SensorData->adChannel8 * 5.882352941176471) + (SensorData->adChannel6 * 0.0) - (SensorData->adChannel2 * 5.882352941176471);
//    double ssrx134 = (SensorData->adChannel8 * 5.882352941176471) + (SensorData->adChannel4 * 0.0) - (SensorData->adChannel2 * 5.882352941176471);
//    double ssrx234 = (SensorData->adChannel6 * 4.854368932038836) - (SensorData->adChannel4 * 4.854368932038836) + (SensorData->adChannel2 * 0.0);
//
//    double ssry123 = -(SensorData->adChannel8 * 3.802281368821293) + (SensorData->adChannel6 * 3.783823692273617) + (SensorData->adChannel4 * 0.018457676547676);
//    double ssry124 = -(SensorData->adChannel8 * 3.824647729814359) + (SensorData->adChannel6 * 3.802281368821292) + (SensorData->adChannel2 * 0.022366360993066);
//    double ssry134 = (SensorData->adChannel8 * 0.782822634757325) + (SensorData->adChannel4 * 3.802281368821292) - (SensorData->adChannel2 * 4.585104003578618);
//    double ssry234 = (SensorData->adChannel6 * 0.646018679168666) + (SensorData->adChannel4 * 3.156262689652626) - (SensorData->adChannel2 * 3.802281368821292);
//
//    double ssz = (ssz123 +ssz124 +ssz134 +ssz234)/4;
//    double ssrx = (ssrx123 +ssrx124 +ssrx134 +ssrx234)/4;
//    double ssry = (ssry123 +ssry124 +ssry134 +ssry234)/4;
//
//    //............... defined ............... //
//    SensorData->interfer1 = (SensorData->ruler2 * 0.5) +(SensorData->ruler3 * 0.5); // ls Y
//    SensorData->interfer2 = (SensorData->ruler2 * 0.75414781297) -(SensorData->ruler3 * 0.75414781297) +1.04e-04; // ls Rz

//    CommandConfig *cz = GetAxisCommandConfig(MicroLogicalZ);
//    CommandConfig *crx = GetAxisCommandConfig(MicroLogicalRx);
//    CommandConfig *cry = GetAxisCommandConfig(MicroLogicalRy);
//    CommandConfig *crz = GetAxisCommandConfig(MicroLogicalRz);
//    CommandConfig *cx = GetAxisCommandConfig(MicroLogicalX);
//    CommandConfig *cy = GetAxisCommandConfig(MicroLogicalY);
//    CommandConfig *clsx = GetAxisCommandConfig(MacroLogicalX);
//    CommandConfig *clsy = GetAxisCommandConfig(MacroLogicalY);
//    CommandConfig *clsrz = GetAxisCommandConfig(MacroLogicalRz);

    // .................. output .............................. //
//    InsertCommitData(count, 0, MultiCoreShareData->AxisControlPara[MacroLogicalX]->controlErr[0]);
//    InsertCommitData(count, 1, MultiCoreShareData->AxisControlPara[MacroLogicalY]->controlErr[0]);
//    InsertCommitData(count, 2, MultiCoreShareData->AxisControlPara[MacroLogicalRz]->controlErr[0]);
//    InsertCommitData(count, 3, MultiCoreShareData->AxisControlPara[MicroLogicalX]->controlErr[0]);
//    InsertCommitData(count, 4, MultiCoreShareData->AxisControlPara[MicroLogicalY]->controlErr[0]);
//    InsertCommitData(count, 5, MultiCoreShareData->AxisControlPara[MicroLogicalRz]->controlErr[0]);
//    InsertCommitData(count, 6, MultiCoreShareData->AxisControlPara[MicroLogicalZ]->controlErr[0]);
//    InsertCommitData(count, 7, MultiCoreShareData->AxisControlPara[MicroLogicalRx]->controlErr[0]);
//    InsertCommitData(count, 8, MultiCoreShareData->AxisControlPara[MicroLogicalRy]->controlErr[0]);


//    InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
//    InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY));
//    InsertCommitData(count, 2, GetAxisReferenceCurve(MacroLogicalRz));
//    InsertCommitData(count, 3, GetAxisReferenceCurve(MicroLogicalX));
//    InsertCommitData(count, 4, GetAxisReferenceCurve(MicroLogicalY));
//    InsertCommitData(count, 5, GetAxisReferenceCurve(MicroLogicalRz));
//    InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalZ));
//    InsertCommitData(count, 7, GetAxisReferenceCurve(MicroLogicalRx));
//    InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalRy));

//        InsertCommitData(count, 0, MultiCoreShareData->MotorOutput[0]);
//        InsertCommitData(count, 1, MultiCoreShareData->MotorOutput[1]);
//        InsertCommitData(count, 2, MultiCoreShareData->MotorOutput[2]);
//        InsertCommitData(count, 3, MultiCoreShareData->MotorOutput[3]);
//        InsertCommitData(count, 4, MultiCoreShareData->MotorOutput[4]);
//        InsertCommitData(count, 5, MultiCoreShareData->MotorOutput[5]);
//        InsertCommitData(count, 6, MultiCoreShareData->MotorOutput[6]);
//        InsertCommitData(count, 7, MultiCoreShareData->MotorOutput[7]);
//        InsertCommitData(count, 8, MultiCoreShareData->MotorOutput[8]);

//     InsertCommitData(count, 0, MultiCoreShareData->sensorData.adChannel1);
//     InsertCommitData(count, 1, MultiCoreShareData->sensorData.adChannel2);
//     InsertCommitData(count, 2, MultiCoreShareData->sensorData.adChannel3);
//     InsertCommitData(count, 3, MultiCoreShareData->sensorData.adChannel4);
//     InsertCommitData(count, 4, MultiCoreShareData->MotorOutput[8]);
//     InsertCommitData(count, 5, MultiCoreShareData->MotorOutput[7]);
//     InsertCommitData(count, 6, MultiCoreShareData->MotorOutput[6]);
//     InsertCommitData(count, 7, MultiCoreShareData->MotorOutput[5]);
//     InsertCommitData(count, 8, MultiCoreShareData->MotorOutput[4]);

    //NotifyVx();
}
//static void NotifyVx()
//{
//    int i;
//    ++count;
//    uint16_t srio_isr_data = 0x3000;
//    int DataFlag[16];
//    if (count == 100) {
//        count = 0;
//        int nLoopCount = 8000/256;
//        for (i = 0;  i < nLoopCount; i++){
//            SRIOWriteDataLSU(0x10,0x1,0xd0000040+ 256*i, SRIO_PACKET_TYPE_STREAMWRITE,  ((Uint8*)CommitDataBuf) + 256*i,  256);//
//
//        }
//        SRIOWriteDataLSU(0x10,0x1,0xd0000040 + 256*i, SRIO_PACKET_TYPE_STREAMWRITE,  ((Uint8*)CommitDataBuf )+  256*i,    8000 - 256*nLoopCount);
//        srio_isr_data |= 0x0001;
//        SRIOWriteDataLSU(0x10,0x1,0xd0000000, SRIO_PACKET_TYPE_STREAMWRITE, (Uint8*)DataFlag, sizeof(DataFlag));
//    }
//    if ((srio_isr_data & 0x00FF) != 0) {
//         SRIO_SendDoorBell(2, 0, 0x10, srio_isr_data);
//         srio_isr_data = 0x3000;
//    }
//
//
//}
//static void VxStreamTransfer()
//{
//
//}
void SwitchAxisTask(Axis axis,Task task)
{
    Command *temp = GetAxisCommand(axis);
    temp->task = task;
}
void SwitchallAxisTask(Task task)
{
    int i;
    for(i = 0;i < AxisCount;++i) {
        SwitchAxisTask((Axis)i, task);
    }
}

int CheckAxisFinish(Axis axis)
{
    CommandConfig *temp = MultiCoreShareData->AxisCommandConfig[axis];
    return temp->runFinishFlag == 1;
}
int CheckAllAxisFinish()
{
    int i;
    int Axisnum = GetAxisCount();
    for (i = 0;i < Axisnum;++i) {
        if (!CheckAxisFinish((Axis)i))
                return -1;
    }
  //  MoveFinishLock(20);
    return 0;
}
#define BYTECOUNT 0x800
#define MAINBUFADDR 0xD0001000u
#define AUXBUFADDR  0xD0101000u
//double temp = 1;
void TransferSysInfo()
{
    static int TransferSum = sizeof(struct VxCommitData)/BYTECOUNT;
    static int TailSize = sizeof(struct VxCommitData)%BYTECOUNT;
    static int TransferCount = 0;
    static unsigned int destaddr = 0;
    static uint16_t srio_isr_data = 0x0;

        //double *target = (double *)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE*6);
//        char *target = (char *)(MSM_SHAREDATA_BASEADDR + COREMEMSIZE*6);
//
//        SRIOReadDataLSU(0x70, 0x0, PADDR(target), SRIO_PACKET_TYPE_NRead, &temp, 8);




//      InsertCommitData(count, 0, GetAxisControlError(MacroLogicalX));
//      InsertCommitData(count, 1, GetAxisControlError(MacroLogicalY));
//      InsertCommitData(count, 2, GetAxisControlError(MacroLogicalRz));
//      InsertCommitData(count, 3, GetAxisControlError(MicroLogicalX));
//      InsertCommitData(count, 4, GetAxisControlError(MicroLogicalY));
//      InsertCommitData(count, 5, GetAxisControlError(MicroLogicalRz));
//      InsertCommitData(count, 6, GetAxisControlError(MicroLogicalZ));
//      InsertCommitData(count, 7, GetAxisControlError(MicroLogicalRx));
//      InsertCommitData(count, 8, GetAxisControlError(MicroLogicalRy));

//    InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
//    InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY));
//    InsertCommitData(count, 2, GetAxisReferenceCurve(MicroLogicalRz));
//    InsertCommitData(count, 3, GetAxisControlError(MacroLogicalX));
//    InsertCommitData(count, 4, GetAxisControlError(MacroLogicalY));
//    InsertCommitData(count, 5, GetAxisControlError(MacroLogicalRz));
//    InsertCommitData(count, 6, GetAxisControlParam(MacroLogicalX)->controlOutput);
//    InsertCommitData(count, 7, GetAxisControlParam(MacroLogicalY)->controlOutput);
//    InsertCommitData(count, 8, GetAxisControlParam(MacroLogicalRz)->controlOutput);
//    InsertCommitData(count, 6, MultiCoreShareData->MotorOutput[0]);
//    InsertCommitData(count, 7, MultiCoreShareData->MotorOutput[1]);
//    InsertCommitData(count, 8, MultiCoreShareData->MotorOutput[2]);


//    double lsrz = (SensorData->ruler2 * 0.75414781297) -(SensorData->ruler3 * 0.75414781297) +1.0e-03;


//        InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
//        InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY));
//        InsertCommitData(count, 2, GetAxisReferenceCurve(MacroLogicalRz));
//        InsertCommitData(count, 3, GetAxisReferenceCurve(MicroLogicalX));
//        InsertCommitData(count, 4, GetAxisReferenceCurve(MicroLogicalY));
//        InsertCommitData(count, 5, GetAxisReferenceCurve(MicroLogicalRz));
//        InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalZ));
//        InsertCommitData(count, 7, GetAxisReferenceCurve(MicroLogicalRx));
//        InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalRy));

        InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
        InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY));
        InsertCommitData(count, 2, GetAxisReferenceCurve(MacroLogicalRz));
        InsertCommitData(count, 3, MultiCoreShareData->Phase[0]);
        InsertCommitData(count, 4, MultiCoreShareData->Phase[1]);
        InsertCommitData(count, 5, GetAxisReferenceCurve(MicroLogicalRz));
        InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalZ));
        InsertCommitData(count, 7, GetAxisReferenceCurve(MicroLogicalRx));
        InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalRy));

//        InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
//        InsertCommitData(count, 1, MultiCoreShareData->MotorOutput[0]);
//        InsertCommitData(count, 2, GetAxisReferenceCurve(MacroLogicalY));
//        InsertCommitData(count, 3, MultiCoreShareData->MotorOutput[1]);
//        InsertCommitData(count, 4, GetAxisReferenceCurve(MicroLogicalX));
//        InsertCommitData(count, 5, MultiCoreShareData->MotorOutput[3]);
//        InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalY));
//        InsertCommitData(count, 7, MultiCoreShareData->MotorOutput[4]);
//
//
//        if (MultiCoreShareData->CurState == SpecificTask) {
//            int *p = (int *)(0xD0000000);
//            InsertCommitData(count, 8, (double)(*p));
//        }
//        else {
//            InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalZ));
//        }
//        InsertCommitData(count, 9, MultiCoreShareData->MotorOutput[6]);

//        InsertCommitData(count, 3, MultiCoreShareData->Phase[MacroLogicalX]);
//        InsertCommitData(count, 4, MultiCoreShareData->Phase[MacroLogicalY]);
//        InsertCommitData(count, 5, MultiCoreShareData->Phase[MicroLogicalZ]);

//      InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX)-GetAxisControlError(MacroLogicalX));
//      InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY)-GetAxisControlError(MacroLogicalY));
//      InsertCommitData(count, 2, GetAxisReferenceCurve(MacroLogicalRz)-GetAxisControlError(MacroLogicalRz));
//      InsertCommitData(count, 3, GetAxisReferenceCurve(MicroLogicalRz)-GetAxisControlError(MicroLogicalRz));
//      InsertCommitData(count, 4, GetAxisReferenceCurve(MicroLogicalZ)-GetAxisControlError(MicroLogicalZ));
//      InsertCommitData(count, 5, GetAxisReferenceCurve(MicroLogicalRx)-GetAxisControlError(MicroLogicalRx));
//      InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalRy)-GetAxisControlError(MicroLogicalRy));
//      InsertCommitData(count, 7, GetAxisReferenceCurve(MicroLogicalX)-GetAxisControlError(MicroLogicalX));
//      InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalY)-GetAxisControlError(MicroLogicalY));

//        InsertCommitData(count, 0, GetAxisReferenceCurve(MicroLogicalX));
//        InsertCommitData(count, 1, GetAxisReferenceCurve(MicroLogicalX)-GetAxisControlError(MicroLogicalX));
//        InsertCommitData(count, 2, MultiCoreShareData->MotorOutput[MicroLogicalX]);
//        InsertCommitData(count, 3, GetAxisReferenceCurve(MicroLogicalY));
//        InsertCommitData(count, 4, GetAxisReferenceCurve(MicroLogicalY)-GetAxisControlError(MicroLogicalY));
//        InsertCommitData(count, 5, MultiCoreShareData->MotorOutput[MicroLogicalY]);
//        InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalRz));
//        InsertCommitData(count, 7, GetAxisReferenceCurve(MicroLogicalRz)-GetAxisControlError(MicroLogicalRz));
//        InsertCommitData(count, 8, MultiCoreShareData->MotorOutput[MicroLogicalRz]);

//    InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
//    InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY));
//    InsertCommitData(count, 2, MultiCoreShareData->ControlCmd.ProberTimes);
//    InsertCommitData(count, 3, MultiCoreShareData->ControlCmd.ProberInterval);
//    InsertCommitData(count, 4, MultiCoreShareData->ControlCmd.ProberTimes*MultiCoreShareData->ControlCmd.ProberInterval);
//    InsertCommitData(count, 5, GetAxisReferenceCurve(MicroLogicalRz));
//    InsertCommitData(count, 6, GetAxisReferenceCurve(MicroLogicalZ));
//    InsertCommitData(count, 7, GetAxisReferenceCurve(MicroLogicalRx));
//    InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalRy));

//       InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX));
//       InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY));
//       InsertCommitData(count, 2, GetAxisReferenceCurve(MacroLogicalRz));
//       InsertCommitData(count, 3, GetAxisControlParam(MacroLogicalX)->controlOutput);
//       InsertCommitData(count, 4, GetAxisControlParam(MacroLogicalY)->controlOutput);
//       InsertCommitData(count, 5, GetAxisControlParam(MacroLogicalRz)->controlOutput);
//       InsertCommitData(count, 6, GetAxisControlError(MacroLogicalX));
//       InsertCommitData(count, 7, GetAxisControlError(MacroLogicalY));
//       InsertCommitData(count, 8, GetAxisControlError(MacroLogicalRz));

//       InsertCommitData(count, 0, GetAxisReferenceCurve(MicroLogicalX));
//       InsertCommitData(count, 1, GetAxisReferenceCurve(MicroLogicalY));
//       InsertCommitData(count, 2, GetAxisReferenceCurve(MicroLogicalZ));
//       InsertCommitData(count, 3, GetAxisControlParam(MicroLogicalX)->controlOutput);
//       InsertCommitData(count, 4, GetAxisControlParam(MicroLogicalY)->controlOutput);
//       InsertCommitData(count, 5, GetAxisControlParam(MicroLogicalZ)->controlOutput);
//       InsertCommitData(count, 6, GetAxisControlError(MicroLogicalX));
//       InsertCommitData(count, 7, GetAxisControlError(MicroLogicalY));
//       InsertCommitData(count, 8, GetAxisControlError(MicroLogicalZ));

//      InsertCommitData(count, 0, GetAxisReferenceCurve(MacroLogicalX)-GetAxisControlError(MacroLogicalX));
//      InsertCommitData(count, 1, GetAxisReferenceCurve(MacroLogicalY)-GetAxisControlError(MacroLogicalY));
//      InsertCommitData(count, 2, GetAxisReferenceCurve(MicroLogicalZ)-GetAxisControlError(MicroLogicalZ));
//      InsertCommitData(count, 3, MultiCoreShareData->Phase[MacroLogicalX]);
//      InsertCommitData(count, 4, MultiCoreShareData->Phase[MacroLogicalY]);
//      InsertCommitData(count, 5, MultiCoreShareData->Phase[MicroLogicalZ]);
//      InsertCommitData(count, 6, GetAxisReferenceCurve(MacroLogicalX));
//      InsertCommitData(count, 7, GetAxisReferenceCurve(MacroLogicalY));
//      InsertCommitData(count, 8, GetAxisReferenceCurve(MicroLogicalZ));

    if (++count == 100) {
        isEnableTransfer = 1;
        TransferCount = 0;
        pReadyBuf = pCurrentBuf;
        srio_isr_data = ((unsigned int)pCurrentBuf == (unsigned int)&CommitBuf[0])?0x1:0x2;
        destaddr = ((unsigned int)pCurrentBuf == (unsigned int)&CommitBuf[0])?MAINBUFADDR:AUXBUFADDR;
        pCurrentBuf = ((unsigned int)pCurrentBuf == (unsigned int)&CommitBuf[0])?&CommitBuf[1]:&CommitBuf[0];
        count = 0;
    }
    if (isEnableTransfer) {
        SRIOWriteDataLSU(0x10,0x1,destaddr+TransferCount*BYTECOUNT,SRIO_PACKET_TYPE_STREAMWRITE,(unsigned char *)pReadyBuf+TransferCount*BYTECOUNT,BYTECOUNT);
        if (++TransferCount == TransferSum) {
            if (TailSize != 0)
                SRIOWriteDataLSU(0x10,0x1,destaddr+TransferCount*BYTECOUNT,SRIO_PACKET_TYPE_STREAMWRITE,(unsigned char *)pReadyBuf+TransferCount*BYTECOUNT,TailSize);
            isEnableTransfer = 0;
            srio_isr_info |= srio_isr_data;
//            r = SRIO_SendDoorBell(2, 0x1, 0x10, srio_isr_data);
        }
    }
}
void VxDooBellSend()
{
    if(srio_isr_info != 0x3000) {
        SRIO_SendDoorBell(2, 0x1, 0x10, srio_isr_info);
        srio_isr_info = 0x3000;
    }

}
static int ReadPeripheralReg(unsigned char nSFPchannel,unsigned int RegAddr)
{
    unsigned char buf[256];
    MyMemSet(buf, 0, 256);
    unsigned int destaddr = nSFPchannel*0x10000 + 0x80100000;
    destaddr += RegAddr*0x100;
    int r;
    r = SRIOReadDataLSU(0xFF, 0x0, destaddr, SRIO_PACKET_TYPE_NRead, buf, 256);
    if (r < 0)
        return r;
    unsigned int *pbuf = (unsigned int *)(buf+4);
    if (*pbuf == 0)
        return -1;
    return 0;
}
int CheckPeripheralState()
{
    int r;
    if ((r = ReadPeripheralReg(0x1, 0x31)) < 0)
        SetErrorCode(ADoffline);
    if ((r = ReadPeripheralReg(0x2,0x31)) < 0)
        SetErrorCode(Ruleroffline);
    if (ReadPeripheralReg(0x4, 0x31) < 0)
        SetErrorCode(DAoffline);
    if (r < 0)
        MultiCoreShareData->CurState = Silence;
    return 0;
}
void MoveFinishLock(unsigned char lock)
{
    if (CSL_semIsFree(lock)) {
        SEMDOWN(lock);
        srio_isr_info |= 0x4;
    }

}
int CheckAllAxisWithoutZ()
{
    int i;
    int Axisnum = GetAxisCount();
    for (i = 0;i < Axisnum;++i) {
        if (i == MicroLogicalZ)
            continue;
        if (!CheckAxisFinish((Axis)i))
                return -1;
    }
    return 0;
}
//void RefreshParamWithoutZ()
//{
//    int i;
//    for (i = 0;i < AxisCount;++i) {
//        if (i == MicroLogicalZ)
//            continue;
//        if (pSParamCache[i]) {
//                memcpy(MultiCoreShareData->AxisSCurvePara[i],pSParamCache[i],sizeof(SParaStruct));
//               // SetAxisSCurveParamWithoutS((Axis)i,0.055,1,200,50000,200000000);
//                SetAxisRefreshFlag((Axis)i,1);
//                pSParamCache[i] = NULL;
//        }
//   }
//}
void RefreshZParam()
{
    struct ListNode *temp = NULL;
    if ((temp = GetHead()) != NULL) {
        memcpy(MultiCoreShareData->AxisSCurvePara[MicroLogicalZ],&temp->param[MicroLogicalZ],sizeof(SParaStruct));
        SetAxisRefreshFlag(MicroLogicalZ, 1);
        if (temp->next != NULL) {
            unsigned long long int mode = temp->next->cmd.WsStepScan;
 //           unsigned long long int prober = temp->next->cmd.ProberEnable;
            if ((mode == WSCOARSESCAN ||
                 mode == WSGERNALSCAN || mode == WSFINESCAN)) {
                memcpy(&temp->param[MicroLogicalZ],&temp->next->param[MicroLogicalZ],sizeof(SParaStruct));
            }
        }
   }
}
void ZPrefetchNext()
{
    struct ListNode *temp;
    temp = GetHead();
    if (temp && temp->next) {
        temp = temp->next;
        unsigned long long int mode = temp->cmd.WsStepScan;
        unsigned long long int prober = temp->cmd.ProberEnable;
        if (prober != 0 && (mode == WSCOARSESCAN ||
            mode == WSGERNALSCAN || mode == WSFINESCAN)) {
            memcpy(MultiCoreShareData->AxisSCurvePara[MicroLogicalZ],&temp->param[MicroLogicalZ],sizeof(SParaStruct));
            SetAxisRefreshFlag(MicroLogicalZ, 1);
        }

    }
}
//int isSpecialSwitch()
//{
//   unsigned long long int mode = GetMoveMode();
//   unsigned long long int prober = GetWhichProber();
//   if ()) {
//       return 0;
//   }
//   else
//       return -1;
//}
int ParseMoveMode()
{
    struct ListNode *temp = NULL;
    if ((temp = GetHead()) == NULL)
        return -1;
    unsigned long long int mode = temp->cmd.WsStepScan;
//    unsigned long long int prober = temp->cmd.ProberEnable;
    if ((mode == WSCOARSESCAN ||
        mode == WSGERNALSCAN || mode == WSFINESCAN))
        return 0;
    else
        return -1;
}
