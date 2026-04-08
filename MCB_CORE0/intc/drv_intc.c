/*
 * intc.c
 *
 *  Created on: 2020年9月4日
 *      Author: zhuyb
 */
#include "../drv_intc.h"
#include "cslr_device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/src/intc/csl_intcAux.h>
#include <ti/csl/csl_cpIntcAux.h>

#define MAX_INTC_EVENT_CNT              (128)

CSL_IntcContext              g_intcContext;
CSL_IntcEventHandlerRecord   g_eventHandler[MAX_INTC_EVENT_CNT];
CSL_IntcHandle               g_IntcHandle[MAX_INTC_EVENT_CNT] = {NULL};
CSL_CPINTC_Handle         	 g_hnd[4]={0};
CSL_IntcGlobalEnableState    g_nState = 0;

int INTC_Init(void)
{
    memset(&g_intcContext,0,sizeof(CSL_IntcContext));
    memset(g_eventHandler,0,MAX_INTC_EVENT_CNT * sizeof(CSL_IntcEventHandlerRecord));

    g_intcContext.eventhandlerRecord = g_eventHandler;
    g_intcContext.numEvtEntries      = MAX_INTC_EVENT_CNT;

    if (CSL_intcInit(&g_intcContext) != CSL_SOK)
        return -1;


    if (CSL_intcGlobalNmiEnable() != CSL_SOK)
          return -1;

      /* Enable global interrupts */
      if (CSL_intcGlobalEnable(&g_nState) != CSL_SOK)
          return -1;

    return 0;
}

int INTC_Uninit(void)
{
    CSL_intcGlobalDisable(&g_nState);

    memset(&g_intcContext,0,sizeof(CSL_IntcContext));
    memset(g_eventHandler,0,sizeof(CSL_IntcEventHandlerRecord));

    return 0;
}
CSL_IntcObj intcObj;

int INTC_Open(int nEventID, int nVectID,funcIntcEventHandler func,void *pParam)
{


    if(nEventID > MAX_INTC_EVENT_CNT)
    {
        return -1;
    }

    memset(&intcObj,0,sizeof(CSL_IntcObj));

    CSL_IntcVectId VecID = (CSL_IntcVectId)nVectID;
    g_IntcHandle[nEventID] = CSL_intcOpen(&intcObj, nEventID, &VecID, NULL);//CSL_GEM_TINTLN

   if(NULL ==  g_IntcHandle[nEventID])
   {
      return -1;
   }


   /* 绑定中断服务函数和中断源*/
   g_eventHandler[nEventID].handler = (CSL_IntcEventHandler)func;
   g_eventHandler[nEventID].arg = (void *)pParam;

   CSL_intcPlugEventHandler(g_IntcHandle[nEventID], &g_eventHandler[nEventID]);

   /* Event Enable */
  // CSL_intcHwControl( g_IntcHandle[nEventID], CSL_INTC_CMD_EVTENABLE, NULL);

   return 0;
}

int INTC_Close(int nEventID)
{
    if(g_IntcHandle[nEventID])
     {
        CSL_intcHwControl( g_IntcHandle[nEventID], CSL_INTC_CMD_EVTDISABLE, NULL);
        CSL_intcClose(g_IntcHandle[nEventID]);
        g_IntcHandle[nEventID] = NULL;
     }

    return 0;
}

int INTC_CIC_Open(char nCIC)
{
	if((nCIC < 0)||(nCIC>3))
	{
		return -1;
	}
	g_hnd[nCIC] = CSL_CPINTC_open(nCIC);
	return 0;
}

int INTC_CIC_Init(char nCIC, int nSysInter, int nHostInter)
{
	if((nCIC < 0)||(nCIC>3))
	{
		return -1;
	}

	if((nHostInter < 0)||(nHostInter > 255))
	{
		return -1;
	}
	/* Disable all host interrupts. */
	CSL_CPINTC_disableAllHostInterrupt(g_hnd[nCIC]);

	/* Configure no nesting support in the CPINTC Module. */
	CSL_CPINTC_setNestingMode (g_hnd[nCIC], CPINTC_NO_NESTING);

	/* We now map System Interrupt xx to channel xx */
	CSL_CPINTC_mapSystemIntrToChannel (g_hnd[nCIC], nSysInter, nHostInter);

	/* We now enable system interrupt xx */
	CSL_CPINTC_enableSysInterrupt (g_hnd[nCIC], nSysInter);

	/* We enable host interrupts. */
	CSL_CPINTC_enableHostInterrupt (g_hnd[nCIC], nHostInter);

	/* Enable all host interrupts also. */
	CSL_CPINTC_enableAllHostInterrupt(g_hnd[nCIC]);

	return 0;
}
int INTC_CIC_ClearSysInter(char nCIC,int nSysInter,int nHostInter)
{
	if((nCIC < 0)||(nCIC>3))
	{
		return -1;
	}

	if((nHostInter < 0)||(nHostInter > 255))
	{
		return -1;
	}

	// 使主机中断失效
	CSL_CPINTC_disableHostInterrupt (g_hnd[nCIC], nHostInter);
	// 清除系统中断
	CSL_CPINTC_clearSysInterrupt (g_hnd[nCIC],nSysInter);
	// 使能主机中断
	CSL_CPINTC_enableHostInterrupt (g_hnd[nCIC], nHostInter);
	return 0;
}
int INTC_CIC_Colse(char nCIC)
{
	if((nCIC < 0)||(nCIC>3))
	{
		return -1;
	}
	memset(g_hnd,0,sizeof(CSL_CPINTC_Handle)*4);
	CSL_CPINTC_disableAllHostInterrupt(g_hnd[nCIC]);
	return 0;
}
