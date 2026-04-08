/*
 * encapsulation.c
 *
 *  Created on: 2022年5月23日
 *      Author: Administrator
 */

#include "encapsulation.h"
#include "../kern/spin_lock.h"
#include "refcurve.h"
static int MotionStep[3] = {0};
static double InitPos[3] = {0};
static unsigned long long count[3]= {0};
static double lastReference[3] = {0};
/*
 *Param:void
 *Comment: 中断开场白，关闭使能获取信号量
 * return:void
 * */
void IsrPrologue()
{
    SEMDOWN(DNUM);
    CSL_intcEventDisable(91);
    CSL_IPC_clearGEMInterruptSource(CORE_NUM, 0);
}
/*
 *Param:void
 *Comment: 中断尾声，重新使能释放信号量
 * return:void
 * */
void IsrEpilogue()
{
//    CACHE_wbInvL1d(pCommandPackage, sizeof(CommandPackage), CACHE_WAIT);
//    _mfence();
//    _mfence();
    SEMUP(DNUM);
    CSL_intcEventClear(91);
    CSL_intcEventEnable(91);
}
/*
 *Param:逻辑轴
 *Comment:根据参数返回相应的测量解耦位置
 * return:当前位置
 * */
double AcquireCurPoint(Axis axis) // 解耦
{
    double ssx123;
    double ssx124;
    double ssx134;
    double ssx234;
    double ssy123;
    double ssy124;
    double ssy134;
    double ssy234;
    double ssrz123;
    double ssrz124;
    double ssrz134;
    double ssrz234;
    double ssz123;
    double ssz124;
    double ssz134;
    double ssz234;
    double ssrx123;
    double ssrx124;
    double ssrx134;
    double ssrx234;
    double ssry123;
    double ssry124;
    double ssry134;
    double ssry234;

    switch(axis){
    case MacroLogicalX:
        return SensorData->ruler1; // 行程【+-0.060】
    case MacroLogicalY:
        return (SensorData->ruler2 * 0.5) +(SensorData->ruler3 * 0.5); // 行程【+-0.060】
    case MacroLogicalRz:
        return (SensorData->ruler2 * 0.75414781297) -(SensorData->ruler3 * 0.75414781297) -0.122e-03;
    case MicroLogicalX:
//        return (SensorData->adChannel7 * 0.455634735218010) -(SensorData->adChannel5 * 1.0) +(SensorData->adChannel3 * 0.455634735218010);
         ssx123 = (SensorData->adChannel7 * 0.455634735218010) -(SensorData->adChannel5 * 1.0) +(SensorData->adChannel3 * 0.455634735218010);
         ssx124 = (SensorData->adChannel7 * 0.0) -(SensorData->adChannel5 * 0.554204014079268) +(SensorData->adChannel1 * 0.445795985920732);
         ssx134 = -(SensorData->adChannel7 * 0.566435336312484) -(SensorData->adChannel3 * 0.566435336312484) +(SensorData->adChannel1 * 1.0);
         ssx234 = -(SensorData->adChannel5 * 0.554204014079268) +(SensorData->adChannel3 * 0.0) +(SensorData->adChannel1 * 0.445795985920732);
        return (ssx123 +ssx124 +ssx134 +ssx234)/4 + (0.0002); // 行程【+-0.0008】
    case MicroLogicalY:
//        return (SensorData->adChannel7 * 0.463493414847883) -(SensorData->adChannel5 * 0.0) -(SensorData->adChannel3 * 0.536506585152117);
         ssy123 = (SensorData->adChannel7 * 0.463493414847883) -(SensorData->adChannel5 * 0.0) -(SensorData->adChannel3 * 0.536506585152117);
         ssy124 = (SensorData->adChannel7 * 1.0) -(SensorData->adChannel5 * 0.524921529449275) -(SensorData->adChannel1 * 0.524921529449275);
         ssy134 = (SensorData->adChannel7 * 0.463493414847883) -(SensorData->adChannel3 * 0.536506585152117) +(SensorData->adChannel1 * 0.0);
         ssy234 = (SensorData->adChannel5 * 0.453484969140939) -(SensorData->adChannel3 * 1.0) +(SensorData->adChannel1 * 0.453484969140939);
        return (ssy123 +ssy124 +ssy134 +ssy234)/4 + (-0.00005); // 行程【+-0.0008】
    case MicroLogicalRz:
//        return (SensorData->adChannel7 * 3.114509394164500) -(SensorData->adChannel5 * 0.0) +(SensorData->adChannel3 * 3.114509394164500);
         ssrz123 = (SensorData->adChannel7 * 3.114509394164500) -(SensorData->adChannel5 * 0.0) +(SensorData->adChannel3 * 3.114509394164500);
         ssrz124 = (SensorData->adChannel7 * 0.0) +(SensorData->adChannel5 * 3.047256231170819) +(SensorData->adChannel1 * 3.047256231170819);
         ssrz134 = (SensorData->adChannel7 * 3.114509394164500) +(SensorData->adChannel3 * 3.114509394164500) +(SensorData->adChannel1 * 0.0);
         ssrz234 = (SensorData->adChannel5 * 3.047256231170819) +(SensorData->adChannel3 * 0.0) +(SensorData->adChannel1 * 3.047256231170819);
        return (ssrz123 +ssrz124 +ssrz134 +ssrz234)/4 + (0.0006); // 【行程+-0.003】
    case MicroLogicalZ:
//        return (SensorData->adChannel8 * 0.490494296577947) + (SensorData->adChannel6 * 0.011886743696704) + (SensorData->adChannel4 * 0.497618959725350);
         ssz123 = (SensorData->adChannel8 * 0.490494296577947) + (SensorData->adChannel6 * 0.011886743696704) + (SensorData->adChannel4 * 0.497618959725350);
         ssz124 = -(SensorData->adChannel8 * 0.112502795795124) + (SensorData->adChannel6 * 0.509505703422053) + (SensorData->adChannel2 * 0.602997092373071);
         ssz134 = (SensorData->adChannel8 * 0.504898233057482) + (SensorData->adChannel4 * 0.509505703422053) - (SensorData->adChannel2 * 0.014403936479535);
         ssz234 = (SensorData->adChannel6 * 0.416663590387242) + (SensorData->adChannel4 * 0.092842113034811) + (SensorData->adChannel2 * 0.490494296577947);
         return (ssz123 +ssz124 +ssz134 +ssz234)/4 +(0.0); // 行程【+-0.0006】
    case MicroLogicalRx:
//        return (SensorData->adChannel8 * 0.0) + (SensorData->adChannel6 * 4.854368932038836) - (SensorData->adChannel4 * 4.854368932038835) - (0.001); // 行程【+-0.0075】
         ssrx123 = (SensorData->adChannel8 * 0.0) + (SensorData->adChannel6 * 4.854368932038836) - (SensorData->adChannel4 * 4.854368932038835);
         ssrx124 = (SensorData->adChannel8 * 5.882352941176471) + (SensorData->adChannel6 * 0.0) - (SensorData->adChannel2 * 5.882352941176471);
         ssrx134 = (SensorData->adChannel8 * 5.882352941176471) + (SensorData->adChannel4 * 0.0) - (SensorData->adChannel2 * 5.882352941176471);
         ssrx234 = (SensorData->adChannel6 * 4.854368932038836) - (SensorData->adChannel4 * 4.854368932038836) + (SensorData->adChannel2 * 0.0);
         return (ssrx123 +ssrx124 +ssrx134 +ssrx234)/4 +(-0.00045); // 行程【+-0.004】
    case MicroLogicalRy:
//        return -(SensorData->adChannel8 * 3.802281368821293) + (SensorData->adChannel6 * 3.783823692273617) + (SensorData->adChannel4 * 0.018457676547676) -(0.004); // 行程【+-0.005】
         ssry123 = -(SensorData->adChannel8 * 3.802281368821293) + (SensorData->adChannel6 * 3.783823692273617) + (SensorData->adChannel4 * 0.018457676547676);
         ssry124 = -(SensorData->adChannel8 * 3.824647729814359) + (SensorData->adChannel6 * 3.802281368821292) + (SensorData->adChannel2 * 0.022366360993066);
         ssry134 = (SensorData->adChannel8 * 0.782822634757325) + (SensorData->adChannel4 * 3.802281368821292) - (SensorData->adChannel2 * 4.585104003578618);
         ssry234 = (SensorData->adChannel6 * 0.646018679168666) + (SensorData->adChannel4 * 3.156262689652626) - (SensorData->adChannel2 * 3.802281368821292);
         return (ssry123 +ssry124 +ssry134 +ssry234)/4 +(-0.0038); // 行程【+-0.004】
    default:
        return 0;
    }

}
static void RefreshCurPoints(Axis axis,double S,double V,double A,double J,double C,double P);
static void RefreshNextPoints(Axis axis,double S,double V,double A,double J,double C,double P);
/*
 *Param:槽号，逻辑轴
 *Comment:获取轴的参考位置用来计算误差
 * return:参考位置
 * */
double AcquireTargetPoint(Slot slotnum,Axis axis)
{
    double targetpoint = 0.0;
    CommandConfig *temp = CURSLOTCONFIG(slotnum);
    if (1 == temp->refreshFlag) {
        temp->refreshFlag = 0;
        temp->runFinishFlag = 0;
        count[slotnum] = 0;
        MotionStep[slotnum]++;
        if (MotionStep[slotnum] == 1)
            InitPos[slotnum] = AcquireCurPoint(axis);
        else
            InitPos[slotnum] = lastReference[slotnum];
        targetpoint = InitPos[slotnum];
        RefreshCurPoints(axis, InitPos[slotnum], 0, 0, 0, 0, 0);
    }
    else {
        targetpoint = temp->nextPointSPara.S;
        RefreshCurPoints(axis, temp->nextPointSPara.S, temp->nextPointSPara.V,temp->nextPointSPara.A ,
                               temp->nextPointSPara.J, temp->nextPointSPara.C, temp->nextPointSPara.P);
    }
    return targetpoint;
}
/*
 *Param:槽号，逻辑轴
 *Comment:计算逻辑轴控制器输出
 * return:void
 * */
void FeedBackController(Slot slotnum,Axis axis)
{
    CommandConfig *temp = CURSLOTCONFIG(slotnum);
//    struct test abcd = array[slotnum];
    double curPoint = AcquireCurPoint(axis);
    double targetPoint = AcquireTargetPoint(slotnum, axis);
    double posErr = targetPoint - curPoint;
    double logicalAxisOut = Controller(posErr, temp->controlPara.controlU, temp->controlPara.controlErr,
                                               temp->controlPara.parameter, temp->controlPara.order);
    temp->controlPara.controlOutput = logicalAxisOut;
    return;
}
/*
 *Param:槽号，逻辑轴
 *Comment:计算逻辑轴下一时刻的参考位置
 * return:void
 * */
void CalculateNextPoint(Slot slotnum,Axis axis,int *index, unsigned long long int delay,unsigned long long int proberdelay)
{

    CommandConfig *temp = CURSLOTCONFIG(slotnum);

    double waveNum = 0;
    double posref[6] = {0,0,0,0,0,0};
    double *SParaPtr  = (double *)(&(temp->sParaSpecific));
    double SParatemp[6] = {0};
    memcpy(SParatemp, SParaPtr, sizeof(double)*6);
    RefCurveCreator(4,&waveNum, posref, SParatemp, InitPos[slotnum], SParatemp[0], 1, count[slotnum], index, slotnum);

    if (count[slotnum] >= waveNum + delay + proberdelay)
    {

        count[slotnum] = waveNum + delay + proberdelay;
        temp->runFinishFlag = 1;
        lastReference[slotnum] = posref[0];
        InitPos[slotnum] = lastReference[slotnum];
    }

    MovePhaseCheck(*index, axis, count[slotnum], waveNum, delay);

    count[slotnum]++;

    RefreshNextPoints(axis, posref[0], posref[1], posref[2], posref[3], posref[4], posref[5]);

}
/*
 *Param:槽号，逻辑轴
 *Comment:当停止控制时清理必要的全局变量
 * return:void
 * */
void Clear(Slot slotnum,Axis axis)
{
    CommandConfig *temp = CURSLOTCONFIG(slotnum);
    MotionStep[slotnum] = 0;
    count[slotnum] = 0;
    InitPos[slotnum] = AcquireCurPoint(axis);
    temp->controlPara.controlOutput = 0;
    temp->runFinishFlag = 0;
    VectorSet(temp->controlPara.controlU, 5, 0);
    VectorSet(temp->controlPara.controlErr, 5, 0);
    memset(&temp->nextPointSPara,0,sizeof(SParaStruct));
}
/*
 *Param:逻辑轴，S曲线参数
 *Comment:刷新当前位置，辅助函数。
 * return:void
 * */
static void RefreshCurPoints(Axis axis,double S,double V,double A,double J,double C,double P)
{
    SParaHandle temp = GetAxisSCurveCurPoint(axis);
    temp->S = S;
    temp->V = V;
    temp->A = A;
    temp->J = J;
    temp->C = C;
    temp->P = P;
}
static void RefreshNextPoints(Axis axis,double S,double V,double A,double J,double C,double P)
{
    SParaHandle temp = GetAxisSCurveNextPoint(axis);
    temp->S = S;
    temp->V = V;
    temp->A = A;
    temp->J = J;
    temp->C = C;
    temp->P = P;
}
void SetAxisRefreshFlag(Axis axis,unsigned char refreshflag)
{
    CommandConfig *temp = MultiCoreShareData->AxisCommandConfig[axis];
    temp->refreshFlag = refreshflag;
}
