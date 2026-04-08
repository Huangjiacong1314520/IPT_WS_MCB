/*
 * interruptcore1.c
 *
 *  Created on: 2022年5月27日
 *      Author: Administrator
 */
#include "interrupt.h"
//#define SPECIFYAXIS
static void AssignControlParam(Axis axis);//确定是哪个轴方向，设置传函参数
static void Step1Function(Slot slot,CommandHandle cmd);
static void Step2Function(Slot slot,CommandHandle cmd);

void IsrMotionControlCore1()
{
    int i;

    for (i = 0;i < 3; ++i)
    {
        CommandHandle temp = &pCommandPackage->CommandArray[i];
        if (MultiCoreShareData->CurState == Default)
        {
            Clear((Slot)i, temp->axis);//清全局变量
            continue;
        }
        switch (temp->task)
        {
            case Task1:
                AssignControlParam(temp->axis);
                Step1Function((Slot)i,temp);//两种参数
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
        case MacroLogicalX:
           temp->parameter[0] = -2.867008005350596;/*den[1]*/
           temp->parameter[1] = 2.742326449059381;/*den[2]*/
           temp->parameter[2] = -0.875318443708785;/*den[3]*/
           temp->parameter[3] = 5.922716821750089e+07;/*num[0]*/
           temp->parameter[4] = -5.840379172537204e+07;/*num[1]*/
           temp->parameter[5] = -5.922501825148308e+07;/*num[2]*/
           temp->parameter[6] = 5.840594169138984e+07;/*num[3]*/
           temp->order = 3;
        break;
        case MacroLogicalY:
            temp->parameter[0] = -2.867008005350596;/*den[1]*/
            temp->parameter[1] = 2.742326449059381;/*den[2]*/
            temp->parameter[2] = -0.875318443708785;/*den[3]*/
            temp->parameter[3] = 8.973813366288015e+07;/*num[0]*/
            temp->parameter[4] = -8.849059352329099e+07;/*num[1]*/
            temp->parameter[5] = -8.973487613861075e+07;/*num[2]*/
            temp->parameter[6] = 8.849385104756040e+07;/*num[3]*/
            temp->order = 3;
        break;
        case MacroLogicalRz:
            temp->parameter[0] = -2.867008005350596;/*den[1]*/
            temp->parameter[1] = 2.742326449059381;/*den[2]*/
            temp->parameter[2] = -0.875318443708785;/*den[3]*/
            temp->parameter[3] = 4.922181535473680e+06;/*num[0]*/
            temp->parameter[4] = -4.853753334560604e+06;/*num[1]*/
            temp->parameter[5] = -4.922002858637471e+06;/*num[2]*/
            temp->parameter[6] = 4.853932011396813e+06;/*num[3]*/
            temp->order = 3;
        break;
    }
}

static void Step1Function(Slot slot,CommandHandle cmd)
{
    int index = 0;
    unsigned long long int delay = 5000;
    unsigned long long int proberdelay = 0;

    SetAxisSCurveParam(MacroLogicalX, 0, 0.02, 0.5, 200, 150000, 1);//设置每个轴向的位置及五阶导数
    SetAxisSCurveParam(MacroLogicalY, 0,0.02,0.5, 200,150000,1);
    SetAxisSCurveParam(MacroLogicalRz, 0, 0.001, 0.05, 10, 5000, 1);

    switch (cmd->axis)
    {
        case MacroLogicalX:
            FeedBackController((Slot)slot,cmd->axis);//解耦并反馈控制
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);//预测下次的位置
        break;
        case MacroLogicalY:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MacroLogicalRz:
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

    SetAxisSCurveParam(MacroLogicalX, 0, 0.001, 0.5, 200, 150000, 1);
    SetAxisSCurveParam(MacroLogicalY, 0, 0.001, 0.5, 200, 150000, 1);
    SetAxisSCurveParam(MacroLogicalRz, 0, 0.002, 0.05, 10, 5000, 1);

    switch (cmd->axis)
    {
        case MacroLogicalX:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.01;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MacroLogicalY:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.01;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        case MacroLogicalRz:
            FeedBackController((Slot)slot,cmd->axis);
            SpeedLowConst[slot] = 0.001;
            CalculateNextPoint((Slot)slot,cmd->axis,&index,delay,proberdelay);
        break;
        default:
        break;
    }
}




