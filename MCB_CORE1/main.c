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
#include "control/refcurve.h"
#include "control/encapsulation.h"
#include "csl_ipc.h"
#include "csl_ipcAux.h"
#include "src/intc/csl_intc.h"
#include "src/intc/csl_intcAux.h"
#include "ti/csl/csl_semAux.h"
#include "kern/macros.h"
#include "hw_soc_C6678.h"
#include "control/encapsulation.h"
#include "control/interrupt.h"
#include "control/msmAddr.h"
/**
 *
 * CORE 1
 * */
void IsrIPC();//中断初始化
static void readELF();
static void CoreTest();
void main(void){
    SEMDOWN(DNUM);//放下____
    WAIT(CSL_semIsFree(0));
    Global_Default_Init();//配置DSP参数和时钟
    MSMRemap();//地址映射
    initConfig();//各数据初始化
    //CoreTest();
    CurveValueSetAll();//参数初始化
    INTC_Init();
    if (0 != INTC_Open(91, 5, IsrIPC, NULL))
    {
        return;
    }
    CSL_intcEventEnable(91);
    SEMUP(DNUM);//抬起____
    while(1){
        ;
    }
}
static void readELF()
{
  ;
}
static void CoreTest()
{
    CommandConfig *temp = CURSLOTCONFIG(0);
    temp->runFinishFlag = 1;
    temp = CURSLOTCONFIG(1);
    temp->runFinishFlag = 1;
    return;
}
int i = 0;
int timeRecord[5000];
void IsrIPC(){

    IsrPrologue();//中断开场白，关闭使能获取信号量
    switch(DNUM) {
    case 1:
        IsrMotionControlCore1();
        break;
    case 2:
        IsrMotionControlCore2();
        break;
    case 3:
        IsrMotionControlCore3();
        break;
    case 4:
        IsrMotionControlCore4();
        break;
    case 5:
        IsrMotionControlCore5();
        break;
    case 6:
        IsrMotionControlCore6();
        break;
    default:
        break;
    }
    IsrEpilogue();//中断尾声，重新使能释放信号量
}



