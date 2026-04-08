/*
 * main.c
 *
 *  Created on: 2021年8月12日
 *      Author: lkx
 */
/**
 * CORE 7
 * */
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
#include "csl_ipc.h"
#include "csl_ipcAux.h"
#include "src/intc/csl_intc.h"
#include "src/intc/csl_intcAux.h"
#include "kern/macros.h"
#include "control/msmAddr.h"
#include "control/encapsulation.h"
#include "control/function.h"
void IsrIPC();
int AxisCalculateFinished();
static void DAOutTest();
void main(void){
    SEMDOWN(DNUM);
//    WAIT(CSL_semIsFree(0));
    Global_Default_Init();
    MSMRemap();
    initConfig();
    //DAOutTest();
    INTC_Init();
    if (0 != INTC_Open(91, 5, IsrIPC, NULL))
    {
        return ;
    }
    CSL_intcEventEnable(91);
    INTCENABLE(5);
    SEMUP(DNUM);
    while(1){
        ;
    }
}
static void DAOutTest()
{
    unsigned int fout[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    DAOut(0xFFF, fout);
    unsigned char oldbuf[256];
    memcpy(oldbuf, AuroraBufferDA, 256);
    DAChannelAssign(1, Channel1);
    DAChannelAssign(2, Channel2);
    DAChannelAssign(3, Channel3);
    DAChannelAssign(4, Channel4);
    DAChannelAssign(5, Channel5);
    DAChannelAssign(6, Channel6);
    DAChannelAssign(7, Channel7);
    DAChannelAssign(8, Channel8);
    DAChannelAssign(9, Channel9);
    DAChannelAssign(10, Channel10);
    DAChannelAssign(11, Channel11);
    DAChannelAssign(12, Channel12);
    DAOutput(0x2);
    unsigned char newbuf[256];
    memcpy(newbuf, AuroraBufferDA, 256);
    assert(memcmp(newbuf, oldbuf, 256) == 0);
    DAChannelAssign(1, Channel1);
    DAChannelAssign(2, Channel2);
    DAChannelAssign(3, Channel3);
    DAChannelAssign(4, Channel4);
    DAChannelAssign(0, Channel5);
    DAChannelAssign(0, Channel6);
    DAChannelAssign(0, Channel7);
    DAChannelAssign(0, Channel8);
    DAChannelAssign(0, Channel9);
    DAOutput(0x2);
    memcpy(newbuf, AuroraBufferDA, 256);
    unsigned int temp[12] ={1,2,3,4,0,0,0,0,0,0,0,0};
    DAOut(0x1FF,temp);
    memcpy(oldbuf, AuroraBufferDA, 256);
    assert(memcmp(newbuf, oldbuf, 256) == 0);

    DAChannelAssign(1, Channel1);
    DAChannelAssign(2, Channel2);
    DAChannelAssign(3, Channel3);
    DAChannelAssign(4, Channel4);
    DAChannelAssign(0, Channel5);
    DAChannelAssign(0, Channel6);
    DAChannelAssign(0, Channel7);
    DAChannelAssign(0, Channel9);
    DAChannelAssign(0, Channel11);
    DAOutput(0x2);
    memcpy(newbuf, AuroraBufferDA, 256);
    DAOut(0x57F,temp);
    memcpy(oldbuf, AuroraBufferDA, 256);
    assert(memcmp(newbuf, oldbuf, 256) == 0);
}
/**
 * 修改这里 每增删核
 *
 *  */
int SrcCores[] = {1, 2, 3, 4, 5, 6};
int CountCores = 6;


int start = 0;
int stop = 0;
int i = 0;
int j = 0;

void IsrIPC()
{

    IsrMotionControl();
}


int AxisCalculateFinished(){
    int numFinished = 0;
    int flagFinished = 0;
    int timeCount = 100;
    int i = 0;

    // 判断是否所有轴计算完成
    for(i = 0; i < CountCores; i++){
        flagFinished = flagFinished | (0x01<<SrcCores[i]);
    }
    while(timeCount > 0){
        for(i = 0; i < CountCores; i++){
            if(CSL_IPC_isGEMInterruptSourceSet(CORE_NUM, SrcCores[i])){
                numFinished = numFinished | (0x01<<SrcCores[i]);
            }
        }
        if(numFinished == flagFinished){
            break;
        }
        delay_us(1);
        timeCount--;
    }
//    printf("flag:%d, get:%d\n", flagFinished, numFinished);
    // 清空标志
    for(i = 0; i < 7; i++){
        CSL_IPC_clearGEMInterruptSource(CORE_NUM, i);
    }
    if(timeCount > 0){
        return 1;
    }else{
        return 0;
    }
}

