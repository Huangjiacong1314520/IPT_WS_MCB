/*
 * interrupt.c
 *  core 1
 *  Created on: 2021年8月22日
 *      Author: lkx
 */

#include "interrupt.h"
#include "ti/csl/csl_semAux.h"
#include "../kern/macros.h"
double InitPos;
unsigned long long count;
double* SParaPtr;
int stepNum = 0;
double lastReference = 0;
double last1 = 0;
double last2 = 0;


/*
 *
 * CORE 1
 * */
//void IsrMotionControl(size_t axis){
//    double posErr = 0;
//    //Xzhou
//    double logicalPosition[2];
//    logicalPosition[0] = SensorData->ruler5*(0.5+SensorData->ruler7/0.632)+SensorData->ruler6*(0.5-SensorData->ruler7/0.632);     // 解耦
//    logicalPosition[1] = SensorData->ruler7;
//    double logicalAxisOut = 0;
//
//
//    Config[0]->controlPara.parameter[0] = -1.893063729055;/*den[0]*/
//    Config[0]->controlPara.parameter[1] = 0.893063729055;/*den[1]*/
//    Config[0]->controlPara.parameter[2] = 416346352.941176354885;/*num[0]*/
//    Config[0]->controlPara.parameter[3] = -827666521.373619079590;/*num[1]*/
//    Config[0]->controlPara.parameter[4] = 411321157.821918308735;/*num[2]*/
//
//
//    /*/////高带宽/////
//    Config[0]->controlPara.parameter[0] = -1.860022740732213;
//    Config[0]->controlPara.parameter[1] = 0.860022740732213;
//    Config[0]->controlPara.parameter[2] = 213704347.8260869;
//    Config[0]->controlPara.parameter[3] = -425873180.5389637;
//    Config[0]->controlPara.parameter[4] = 212169131.8503658;*/
//
//
//    if(1 == Config[axis]->runFlag){
//        if(1 == Config[axis]->refreshFlag){
//            stepNum++;
//            Config[axis]->refreshFlag = 0;
//            Config[axis]->runFinishFlag = 0;
//            count = 0;
//            last1 = 0;
//            last2 = 0;
//            if(1 == stepNum){
//                InitPos = logicalPosition[axis];    //当前测量值
//            }else{
//                InitPos = lastReference;        // 上一条曲线最后一点
//            }
//
//            Config[0]->currentPointSPara.S = InitPos;
//            Config[0]->currentPointSPara.V = 0;
//            Config[0]->currentPointSPara.A = 0;
//            Config[0]->currentPointSPara.J = 0;
//            Config[0]->currentPointSPara.C = 0;
//            Config[0]->currentPointSPara.P = 0;
//        }else{
//            Config[axis]->currentPointSPara.S = Config[axis]->nextPointSPara.S;
//            Config[axis]->currentPointSPara.V = Config[axis]->nextPointSPara.V;
//            Config[axis]->currentPointSPara.A = Config[axis]->nextPointSPara.A;
//            Config[axis]->currentPointSPara.J = Config[axis]->nextPointSPara.J;
//            Config[axis]->currentPointSPara.C = Config[axis]->nextPointSPara.C;
//            Config[axis]->currentPointSPara.P = Config[axis]->nextPointSPara.P;
//        }
//
//        // 控制量计算
//        posErr = Config[axis]->currentPointSPara.S - logicalPosition[axis];
//        logicalAxisOut = Controller(posErr, Config[axis]->controlPara.controlU,
//                                            Config[axis]->controlPara.controlErr, Config[axis]->controlPara.parameter, 2);
//        // 对DA进行赋值
//        Config[axis]->controlPara.controlOutput = logicalAxisOut;
//    }else{
//        stepNum = 0;
//        count = 0;
//        InitPos = logicalPosition[axis];    //当前测量值
//        Config[axis]->controlPara.controlOutput = 0;
//        VectorSet(Config[axis]->controlPara.controlU, 5, 0);
//        VectorSet(Config[axis]->controlPara.controlErr, 5, 0);
//    }
//}
//
//void calculateRefCurve(size_t axis){
//    double waveNum = 0;
//    double posref[6] = {0, 0, 0, 0, 0, 0};
//    SParaPtr = (double*)(&(Config[axis]->sParaSpecific));
//
//    if(1 == Config[axis]->runFlag){
//        RefCurveCreator(&waveNum, (double*)posref, (double*)SParaPtr, InitPos, SParaPtr[0], 1, count,axis);
//        count++;
//        if(count > waveNum+5000){
//            if(count < waveNum+10000){
//                Config[axis]->runFinishFlag = 1;
//            }else{
//                Config[axis]->runFinishFlag = 0;
//            }
//            count = waveNum+10000;
//            lastReference = posref[0];
//            InitPos = lastReference;
//            if (CSL_semIsFree(11)) {
//                SEMDOWN(11);
//            }
//        }
//        Config[axis]->nextPointSPara.S = posref[0];
//        Config[axis]->nextPointSPara.V = posref[1];
//        Config[axis]->nextPointSPara.A = posref[2];
//        Config[axis]->nextPointSPara.J = posref[3];
//        Config[axis]->nextPointSPara.C = posref[4];
//        Config[axis]->nextPointSPara.P = posref[5];
//    }else{
//        Config[axis]->nextPointSPara.S = InitPos;
//        Config[axis]->nextPointSPara.V = 0;
//        Config[axis]->nextPointSPara.A = 0;
//        Config[axis]->nextPointSPara.J = 0;
//        Config[axis]->nextPointSPara.C = 0;
//        Config[axis]->nextPointSPara.P = 0;
//        if (!CSL_semIsFree(11)){
//            SEMUP(11);
//        }
//    }
//}

void IsrVxWorks(){

}






