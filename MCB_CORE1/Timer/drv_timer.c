/*
 * Timer.c
 *
 *  Created on: 2020-9-2
 *      Author: linxi
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../drv_timer.h"
#include <ti/csl/csl_tmr.h>
#include <ti/csl/csl_tmrAux.h>
#include <ti/csl/src/intc/csl_intc.h>
#include "timer.h"


typedef void (*pTimer)(void* handle);
typedef struct _TIMER_INTERRUPT_CONFIG_
{
	pTimer  pTimerFun;
	int 	nTime; //ms
	void*   pParam;
}TimerCfg_st,*pTimerCfg_st;





#define MAX_TIMER_CNT       (16)
/* Maximum delay to wait for to detect spurious interrupts. */
#define  MAX_DELAY                  0xFFFF


CSL_TmrHandle               g_hTmr[MAX_TIMER_CNT] = {NULL};

/* INTC Objects */

CSL_TmrObj                  g_TmrObj[MAX_TIMER_CNT] = {0};

TimerCfg_st                 g_TimerCfg[MAX_TIMER_CNT] = {0};


unsigned int g_CoreClk = 1000;      //1000MHZ



/*
 * nCoreClk         CPU Clock MHz
 */
void Timer_SetCoreClk(unsigned int nCoreClk)
{
    g_CoreClk = nCoreClk;

    return;
}

/*
 * nTimer   定时器实例
 * nMs      延时时间ms
 */


int Timer_Delay(unsigned char nTimer, unsigned int nPerHi, unsigned int nPerLo)
{
    CSL_TmrEnamode   tmrTimeCountMode = CSL_TMR_ENAMODE_ENABLE;
    CSL_TmrHwStatusQuery Query;
    unsigned int nCnt = 0;
    CSL_TmrObj                  TmrObj;
    CSL_TmrHandle  hTmr;
    CSL_Status tmrstatus;
    CSL_TmrHwSetup               tmrhwSetup = CSL_TMR_HWSETUP_DEFAULTS;


    //UInt64 nPeriodValue = (UInt64)nMs*g_CoreClk*1000/6;
    //UInt64 nPeriodValue = (UInt64)nMs*g_CoreClk/6;//us

    /* Open the timer. */
    memset(&TmrObj, 0, sizeof(CSL_TmrObj));
    hTmr = CSL_tmrOpen(&TmrObj, nTimer, NULL, &tmrstatus);
    if (hTmr == NULL)
        return 0;


    CSL_tmrHwControl(hTmr, CSL_TMR_CMD_STOP64, NULL);
    /* Set the timer mode to 64bit GP Timer Mode and set the PRD registers */
    //tmrhwSetup.tmrTimerMode = CSL_TMR_TIMMODE_GPT;

    tmrhwSetup.tmrTimerPeriodLo = nPerLo;
    tmrhwSetup.tmrTimerPeriodHi = nPerHi;

  //  tmrhwSetup =  CSL_TMR_HWSETUP_DEFAULTS;
    CSL_tmrHwSetup(hTmr, &tmrhwSetup);

    /* Start the timer with one shot */
    CSL_tmrHwControl(hTmr, CSL_TMR_CMD_START64, (void *)&tmrTimeCountMode);

    Query = CSL_TMR_QUERY_COUNT_LO; //CSL_TMR_QUERY_COUNT_LO
    CSL_tmrGetHwStatus(hTmr,Query,&nCnt);
    while(nCnt)
    {
        CSL_tmrGetHwStatus(hTmr,Query,&nCnt);
    }

    CSL_tmrClose(hTmr);

    return 0;
}


int Timer_Init(void)
{

    CSL_tmrInit(NULL);

    return 0;
}

#if 0
int Timer_StopPeriod(unsigned char nTimer)
{
    unsigned char               nEvntID;

    if(nTimer >=  MAX_TIMER_CNT)
    {
        return -1;
    }

   if(g_hTmr[nTimer])
   {
       CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_RESET64, NULL);
       CSL_tmrClose(g_hTmr[nTimer]);
       g_hTmr[nTimer] = NULL;
   }

   if(nTimer < 8)
   {
     nEvntID = CSL_GEM_TINTLN;
   }
   else
   {
       nEvntID = CSL_GEM_TINT8L + (nTimer-8)*2;
   }

   INTC_Close(nEvntID);

   return 0;
}


int Timer_StartPeriod(unsigned char nTimer, int nVectID, TimerCfg_st TimerCfg)
{


    CSL_Status                   tmrstatus;
    CSL_TmrHwSetup               tmrhwSetup = CSL_TMR_HWSETUP_DEFAULTS;
    CSL_TmrEnamode               tmrTimeCountMode = CSL_TMR_ENAMODE_CONT;

    unsigned char               nEvntID;


    UInt64 nPeriodValue = (UInt64)TimerCfg.nTime*g_CoreClk*1000/6;


    if(0>Timer_StopPeriod(nTimer))
    {
        return -1;
    }
    /* Clear local data structures */
    memset(&g_TmrObj[nTimer], 0, sizeof(CSL_TmrObj));

    /* Open INTC */
    if(nTimer < 8)
    {
        nEvntID = CSL_GEM_TINTLN;
    }
    else
    {
        nEvntID = CSL_GEM_TINT8L + (nTimer-8)*2;
    }

    INTC_Open(nEvntID,nVectID,TimerCfg.pTimerFun,TimerCfg.pParam);
    g_TimerCfg[nTimer] = TimerCfg;


    /* Open the timer. */
    g_hTmr[nTimer] = CSL_tmrOpen(&g_TmrObj[nTimer], nTimer, NULL, &tmrstatus);
    if ( g_hTmr[nTimer] == NULL)
        return -1;

     CSL_tmrHwControl( g_hTmr[nTimer], CSL_TMR_CMD_STOP64, NULL);

    /* Set the timer mode to 64bit GP Timer Mode and set the PRD registers */
    tmrhwSetup.tmrTimerMode = CSL_TMR_TIMMODE_GPT;


    tmrhwSetup.tmrTimerPeriodLo = nPeriodValue&0xffffffff;
    tmrhwSetup.tmrTimerPeriodHi = nPeriodValue>>32;

    CSL_tmrHwSetup(g_hTmr[nTimer], &tmrhwSetup);

    /* Reset the Timer */
    CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_RESET64, NULL);

    /* Start the timer in SINGLE SHOT Mode. */
    CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_START64, (void *)&tmrTimeCountMode);


    return 0;


}

#endif

int Timer_Config(unsigned char nTimer, unsigned int nPerHi, unsigned int nPerLo)
{
	if (nTimer > 15)
	{
		return -1;
	}


    CSL_TmrObj                  TmrObj;
    CSL_TmrHandle  hTmr;
    CSL_Status tmrstatus;
    CSL_TmrHwSetup     tmrhwSetup = CSL_TMR_HWSETUP_DEFAULTS;

	if (g_hTmr[nTimer])
	{
		CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_RESET64, NULL);
		CSL_tmrClose(g_hTmr[nTimer]);
		g_hTmr[nTimer] = NULL;
	}


    /* Open the timer. */
    memset(&TmrObj, 0, sizeof(CSL_TmrObj));
    hTmr = CSL_tmrOpen(&TmrObj, nTimer, NULL, &tmrstatus);
    if (hTmr == NULL)
        return 0;

    g_hTmr[nTimer] = hTmr;

    CSL_tmrHwControl(hTmr, CSL_TMR_CMD_STOP64, NULL);
    /* Set the timer mode to 64bit GP Timer Mode and set the PRD registers */
    //tmrhwSetup.tmrTimerMode = CSL_TMR_TIMMODE_GPT;

    tmrhwSetup.tmrTimerPeriodLo = nPerLo;
    tmrhwSetup.tmrTimerPeriodHi = nPerHi;

  //  tmrhwSetup =  CSL_TMR_HWSETUP_DEFAULTS;
    CSL_tmrHwSetup(hTmr, &tmrhwSetup);


    return 0;
}

int Timer_Start(unsigned char nTimer, unsigned char EnaMode)
{
	if (nTimer > 15)
	{
		return -1;
	}

	if ((EnaMode < 1) || (EnaMode > 2))
	{
		return -1;
	}
    CSL_TmrEnamode   tmrTimeCountMode = (CSL_TmrEnamode)EnaMode;
    /* Start the timer with one shot */
    CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_START64, (void *)&tmrTimeCountMode);
    return 0;
}


int Timer_GetCount(unsigned char nTimer,unsigned int *pCntHi, unsigned int *pCntLo)
{
    //CSL_TmrHwStatusQuery Query;

    if(NULL ==  g_hTmr[nTimer])
    {
        *pCntHi = 0;
        *pCntLo = 0;
        return -1;
    }
    unsigned int BaseAddr= 0x02200000 + nTimer * 0x10000;
    *pCntHi =TimerCounterGet(BaseAddr, TMR_TIMER34);
    *pCntLo=TimerCounterGet(BaseAddr, TMR_TIMER12);


    return 0;
}

int Timer_ModifyCount(unsigned char nTimer, unsigned int nCntHi, unsigned int nCntLo)
{
	if (NULL == g_hTmr[nTimer])
	{
		return -1;
	}

	unsigned int BaseAddr = 0x02200000 + nTimer * 10000;
	TimerCounterSet(BaseAddr, TMR_TIMER34, nCntHi);
	TimerCounterSet(BaseAddr, TMR_TIMER12, nCntLo);

	return 0;
}


int Timer_Close(unsigned char nTimer)
{
    if(NULL ==  g_hTmr[nTimer])
    {
        return -1;
    }

    CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_STOP64, NULL);
    CSL_tmrHwControl(g_hTmr[nTimer], CSL_TMR_CMD_RESET64, NULL);
    CSL_tmrClose(g_hTmr[nTimer]);

    g_hTmr[nTimer] = NULL;
    return 0;
}


