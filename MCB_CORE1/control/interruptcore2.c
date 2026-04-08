/*
 * interruptcore2.c
 *
 *  Created on: 2022Äê5ÔÂ27ÈÕ
 *      Author: Administrator
 */

#include "interrupt.h"
static void AssignControlParam(Axis axis);
static void Step1Function(Slot slot,CommandHandle cmd);
static void Step2Function(Slot slot,CommandHandle cmd);
void IsrMotionControlCore2()
{
    int i;

    for (i = 0;i < 3; ++i)
    {
        CommandHandle temp = &pCommandPackage->CommandArray[i];
        if (MultiCoreShareData->CurState == Default)
        {
            Clear((Slot)i, temp->axis);
            continue;
        }
        switch (temp->task)
        {
            case Task1:
                AssignControlParam(temp->axis);
                Step1Function((Slot)i,temp);
            break;
            case Task2:
                AssignControlParam(temp->axis);
                Step2Function((Slot)i,temp);
            break;
            case Task3:
            break;
            default:
            break;
        }
    }
}

static void AssignControlParam(Axis axis)
{
    ControlParaHandle temp = GetAxisControlParam(axis);

    switch(axis)
    {
        case MicroLogicalX:
            temp->parameter[0] = -2.867008005351;/*den[1]*/
            temp->parameter[1] = 2.742326449059;/*den[2]*/
            temp->parameter[2] = -0.875318443709;/*den[3]*/
            temp->parameter[3] = 934631127.960563182831;/*num[0]*/
            temp->parameter[4] = -921637879.714275956154;/*num[1]*/
            temp->parameter[5] = -934597200.537983655930;/*num[2]*/
            temp->parameter[6] = 921671807.136855483055;/*num[3]*/
            temp->order = 3;
        break;
        case MicroLogicalY:
            temp->parameter[0] = -2.867008005351;/*den[1]*/
            temp->parameter[1] = 2.742326449059;/*den[2]*/
            temp->parameter[2] = -0.875318443709;/*den[3]*/
            temp->parameter[3] = 934631127.960563182831;/*num[0]*/
            temp->parameter[4] = -921637879.714275956154;/*num[1]*/
            temp->parameter[5] = -934597200.537983655930;/*num[2]*/
            temp->parameter[6] = 921671807.136855483055;/*num[3]*/
            temp->order = 3;
        break;
        case MicroLogicalRz:
            temp->parameter[0] = -2.867008005351;/*den[1]*/
            temp->parameter[1] = 2.742326449059;/*den[2]*/
            temp->parameter[2] = -0.875318443709;/*den[3]*/
            temp->parameter[3] = 17028511.266971569508;/*num[0]*/
            temp->parameter[4] = -16791780.788457274437;/*num[1]*/
            temp->parameter[5] = -17027893.126316532493;/*num[2]*/
            temp->parameter[6] = 16792398.929112307727;/*num[3]*/
            temp->order = 3;
        break;
    }
}
static void Step1Function(Slot slot,CommandHandle cmd)
{
    int index = 0;
    unsigned long long int delay = 5000;
    unsigned long long int proberdelay = 0;

    SetAxisSCurveParam(MicroLogicalX,  0, 0.001, 0.1,  100, 250000, 1);
    SetAxisSCurveParam(MicroLogicalY,  0, 0.001, 0.1,  100, 250000, 1);
    SetAxisSCurveParam(MicroLogicalRz, 0, 0.002, 0.05, 10,  5000,   1);

    switch (cmd->axis)
    {
        case MicroLogicalX:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MicroLogicalY:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MicroLogicalRz:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        default:
        break;
    }
}
static void Step2Function(Slot slot,CommandHandle cmd)
{
    int index = 0;
    unsigned long long int delay = 0;
    unsigned long long int proberdelay = 0;

//    SetAxisSCurveParam(MicroLogicalX,  0, 0.001, 0.1,  100, 250000, 1);
//    SetAxisSCurveParam(MicroLogicalY,  0, 0.001, 0.1,  100, 250000, 1);
    SetAxisSCurveParam(MicroLogicalRz, 0, 0.002, 0.05, 10,  5000,   1);

    switch (cmd->axis)
    {
        case MicroLogicalX:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MicroLogicalY:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MicroLogicalRz:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        default:
        break;
    }
}
