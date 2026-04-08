/*
 * interrupt.c
 *
 *  Created on: 2022Äê7ÔÂ3ÈÕ
 *      Author: lkx
 *      Modified:cxy
 */

#include "interrupt.h"
#include "../kern/macros.h"
#include "../peripheral/tempAcquisition.h"
static int  Delay(unsigned int nTimes);

void  IsrMotionControl(){

    static int isDelayFinished = 0;//延时结束

    switch(GetSystemCurState())//系统当前状态
    {
        case Silence:
            IsrPrologue();
            ReceiveControlCommand();
            IsrEpilogue(130);
            ParseControlCommand();
            if (MultiCoreShareData->CurState != Activate)
            {
                MultiCoreShareData->CurState = Silence;
            }
        break;
        case Activate:
            if (!isDelayFinished)
            {
                isDelayFinished = Delay(15000);
                break;
            }
            OgModeSet(0x2, 0xFFF);
            IsrPrologue();
            ReceiveControlCommand();
          //  CheckPeripheralState();
            isDelayFinished = 0;
            MultiCoreShareData->CurState = Default;
            NotifyOtherCores();//运行其它核的程序
            IsrEpilogue(130);
            ParseControlCommand();
        break;
        case Default:
            IsrPrologue();
            ReadSensorTopHalf();//传感器数据
            ReceiveControlCommand();
            CheckSensorDataTransfer(40);//是否传完
            ADDataProcess();
            RulerDataProcess();
            NotifyOtherCores();
            MultiCoreShareData->Phase[0] = Free;
            MultiCoreShareData->Phase[1] = Free;
            MultiCoreShareData->Phase[2] = Free;
            MultiCoreShareData->Phase[3] = Free;
            MultiCoreShareData->Phase[4] = Free;
            MultiCoreShareData->Phase[5] = Free;
            MultiCoreShareData->Phase[6] = Free;
            MultiCoreShareData->Phase[7] = Free;
            MultiCoreShareData->Phase[8] = Free;
            IsrEpilogue(130);
            ParseControlCommand();//解析控制指令
            SetallAxisRefreshFlag(0);
            ClearGlobalVar();
            if ((MultiCoreShareData->CurState != Initialize) && (MultiCoreShareData->CurState != Activate)
            {
                MultiCoreShareData->CurState = Default;
            }
        break;
        case Initialize:
            IsrPrologue();
            ReadSensorTopHalf();
            ReceiveControlCommand();
            InitMoveMode(Initialize);
            CheckSensorDataTransfer(40);
            ADDataProcess();
            RulerDataProcess();
            KickUnlock();
            CSL_IPC_genGEMInterrupt(1, 0);
            CSL_IPC_genGEMInterrupt(2, 0);
            CSL_IPC_genGEMInterrupt(3, 0);
            CSL_IPC_genGEMInterrupt(7, 0);
            KickLock();
            IsrEpilogue(130);
            if (CheckAllAxisFinish() == 0)
            {
                MoveFinishLock(0);
            }
            ParseControlCommand();
            TransferSysInfo();
        break;

        case SpecificTask:
            IsrPrologue();
            ReadSensorTopHalf();
            ReceiveControlCommand();

            InitMoveMode(SpecificTask);

            if (CheckAllAxisFinish() == 0)
            {
                RefreshParam();
            }
            if (CheckSParamCacheEmpty() == 0)
            {
                RequestNewSParam();
            }

            CheckSensorDataTransfer(40);
            ADDataProcess();
            RulerDataProcess();
            KickUnlock();
            CSL_IPC_genGEMInterrupt(1, 0);
            CSL_IPC_genGEMInterrupt(2, 0);
            CSL_IPC_genGEMInterrupt(3, 0);
            CSL_IPC_genGEMInterrupt(7, 0);
            KickLock();
            IsrEpilogue(130);
            ParseControlCommand();
            TransferSysInfo();
        break;
        case Stop:
            NotifyOtherCores();
            IsrEpilogue(130);
            ParseControlCommand();
            TransferSysInfo();
        break;
    }
    VxDooBellSend();
}
static int  Delay(unsigned int nTimes)
{
    static int counter = 0;
    if (++counter == nTimes)
    {
        counter = 0;
        return 1;
    }
    return 0;
}




