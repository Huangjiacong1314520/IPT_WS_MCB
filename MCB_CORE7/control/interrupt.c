
/*
 * interrupt.c
 *
 *  Created on: 2021锟斤拷8锟斤拷22锟斤拷
 *      Author: lkx
 */

#include "interrupt.h"
#include "c6x.h"
#include "encapsulation.h"
#include <math.h>

double MotorOutPut[MotorCount] = {0};
static void AllchannelOuPutZero();//DA通道分配置零
static void AllcontrolOuPutZero();//多核共享数据输出置零
void ErrProtect(double LSXYerrorlimit, double LSRzerrorlimit, double SSXYZerrorlimit, double SSRxRyRzerrorlimit);//超限幅保护
void PositionProtect(double LSXposlimit, double LSYposlimit, double LSRzposlimit, double SSXposlimit, double SSYposlimit, double SSRzposlimit, double SSZposlimit, double SSRxposlimit, double SSRyposlimit);
void FFAcc();
void Cablecompensate();
void IsrMotionControl()
{
    IsrPrologue();

    if(MultiCoreShareData->CurState == Default)
    {
        AllcontrolOuPutZero();
        AllchannelOuPutZero();
        goto IsrEnd;
    }

//    if (MultiCoreShareData->CurState == SpecificTask ||
//        MultiCoreShareData->CurState == Default)
//    {
//        ErrProtect(0.002, 0.002, 0.0005, 0.005);
//    }
    GBcompensate(MotorOutPut);//补偿运算
    ProtectOut(MotorOutPut, 52428.7*10.0);//饱和

    DAChannelAssign(MotorOutPut[LongStrokeY2], Channel7);//对应输出通道
    DAChannelAssign(MotorOutPut[LongStrokeY1], Channel9);
    DAChannelAssign(MotorOutPut[LongStrokeX], Channel11);

    DAChannelAssign(MotorOutPut[ShortStrokeX], Channel6);
    DAChannelAssign(MotorOutPut[ShortStrokeY1], Channel5);
    DAChannelAssign(MotorOutPut[ShortStrokeY2], Channel4);

    DAChannelAssign(MotorOutPut[ShortStrokeZ1], Channel3);
    DAChannelAssign(MotorOutPut[ShortStrokeZ2], Channel2);
    DAChannelAssign(MotorOutPut[ShortStrokeZ3], Channel1);

    RefreshShareData(MotorOutPut,MotorCount);//更新多核共享数据
-
    DAOutput(0x4);//输出数据

IsrEnd:
    IsrEpilogue();
}

static void AllchannelOuPutZero()
{
    DAChannelAssign(0, Channel7);
    DAChannelAssign(0, Channel9);
    DAChannelAssign(0, Channel11);

    DAChannelAssign(0, Channel1);
    DAChannelAssign(0, Channel2);
    DAChannelAssign(0, Channel3);
    DAChannelAssign(0, Channel4);
    DAChannelAssign(0, Channel5);
    DAChannelAssign(0, Channel6);
    DAOutput(0x4);
}
static void AllcontrolOuPutZero()
{
    MultiCoreShareData->AxisControlPara[MacroLogicalX]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MacroLogicalY]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MacroLogicalRz]->controlOutput = 0;

    MultiCoreShareData->AxisControlPara[MicroLogicalX]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MicroLogicalY]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MicroLogicalRz]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MicroLogicalZ]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MicroLogicalRx]->controlOutput = 0;
    MultiCoreShareData->AxisControlPara[MicroLogicalRy]->controlOutput = 0;

    MultiCoreShareData->MotorOutput[0] = 0;
    MultiCoreShareData->MotorOutput[1] = 0;
    MultiCoreShareData->MotorOutput[2] = 0;
    MultiCoreShareData->MotorOutput[3] = 0;
    MultiCoreShareData->MotorOutput[4] = 0;
    MultiCoreShareData->MotorOutput[5] = 0;
    MultiCoreShareData->MotorOutput[6] = 0;
    MultiCoreShareData->MotorOutput[7] = 0;
    MultiCoreShareData->MotorOutput[8] = 0;
 //   MultiCoreShareData->MotorOutput[0] = 0;
}
void ErrProtect(double LSXYerrorlimit, double LSRzerrorlimit, double SSXYZerrorlimit, double SSRxRyRzerrorlimit)
{
    if (MultiCoreShareData->AxisControlPara[MacroLogicalX]->controlErr[0] > LSXYerrorlimit ||
        MultiCoreShareData->AxisControlPara[MacroLogicalX]->controlErr[0] < -LSXYerrorlimit)
    {
        MultiCoreShareData->AxisControlPara[MacroLogicalX]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }

    if (MultiCoreShareData->AxisControlPara[MacroLogicalY]->controlErr[0] > LSXYerrorlimit ||
        MultiCoreShareData->AxisControlPara[MacroLogicalY]->controlErr[0] < -LSXYerrorlimit)
    {
        MultiCoreShareData->AxisControlPara[MacroLogicalY]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }

    if (MultiCoreShareData->AxisControlPara[MacroLogicalRz]->controlErr[0] > LSRzerrorlimit ||
        MultiCoreShareData->AxisControlPara[MacroLogicalRz]->controlErr[0] < -LSRzerrorlimit)
    {
        MultiCoreShareData->AxisControlPara[MacroLogicalRz]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }

    if (MultiCoreShareData->AxisControlPara[MicroLogicalX]->controlErr[0] >  SSXYZerrorlimit ||
        MultiCoreShareData->AxisControlPara[MicroLogicalX]->controlErr[0] < -SSXYZerrorlimit    )
    {
        MultiCoreShareData->AxisControlPara[MicroLogicalX]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }

    if (MultiCoreShareData->AxisControlPara[MicroLogicalY]->controlErr[0] >  SSXYZerrorlimit ||
        MultiCoreShareData->AxisControlPara[MicroLogicalY]->controlErr[0] < -SSXYZerrorlimit    )
    {
        MultiCoreShareData->AxisControlPara[MicroLogicalY]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }
    if (MultiCoreShareData->AxisControlPara[MicroLogicalRz]->controlErr[0] >  SSRxRyRzerrorlimit ||
        MultiCoreShareData->AxisControlPara[MicroLogicalRz]->controlErr[0] < -SSRxRyRzerrorlimit    )
    {
        MultiCoreShareData->AxisControlPara[MicroLogicalRz]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }
    if (MultiCoreShareData->AxisControlPara[MicroLogicalZ]->controlErr[0] >  SSXYZerrorlimit ||
        MultiCoreShareData->AxisControlPara[MicroLogicalZ]->controlErr[0] < -SSXYZerrorlimit    )
    {
        MultiCoreShareData->AxisControlPara[MicroLogicalZ]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }
    if (MultiCoreShareData->AxisControlPara[MicroLogicalRx]->controlErr[0] >  SSRxRyRzerrorlimit ||
        MultiCoreShareData->AxisControlPara[MicroLogicalRx]->controlErr[0] < -SSRxRyRzerrorlimit    )
    {
        MultiCoreShareData->AxisControlPara[MicroLogicalRx]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }
    if (MultiCoreShareData->AxisControlPara[MicroLogicalRy]->controlErr[0] >  SSRxRyRzerrorlimit ||
        MultiCoreShareData->AxisControlPara[MicroLogicalRy]->controlErr[0] < -SSRxRyRzerrorlimit    )
    {
        MultiCoreShareData->AxisControlPara[MicroLogicalRy]->controlOutput = 0;
        MultiCoreShareData->CurState = Default;
    }
}

