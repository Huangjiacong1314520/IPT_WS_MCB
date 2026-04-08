/*
 * EDMA3.c
 *
 *  Created on: 2020-8-31
 *      Author: linxi
 */
#include "../drv_edma3.h"
#include "edma.h"
#include "../include/hw_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c6x.h>

#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/tistdtypes.h>
#include <ti/csl/csl_cpIntcAux.h>
#include "../drv_intc.h"
#include "../drv_cache.h"
#include "hw_soc_c6678.h"

static unsigned int BaseAddr=SOC_EDMA30CC_0_REGS_C667x;//SOC_EDMA31CC_0_REGS_C667x;




#if 0
int EDMA3SetParam(unsigned int chNum, EDMA3CCParam_st* pNewPaRAM)
{
	//if(EDMA3RequestChannel(BaseAddr,0,chNum,0,0))
    if(EDMA3RequestChannel(BaseAddr,0,chNum,0,0))
	{
        EDMA3ChannelToParamMap(BaseAddr,chNum,chNum);   //modify by zyb

		EDMA3CCPaRAMEntry param;
		memcpy(&param,pNewPaRAM,sizeof(EDMA3CCParam_st));
		EDMA3SetPaRAM(BaseAddr,chNum,&param);
		return 0;
	}
	return -1;
}

int EDMA3QdmaSetParam(unsigned int chNum,unsigned int paRAMId,EDMA3CCParam_st* pNewPaRAM)
{
	if(EDMA3RequestChannel(BaseAddr,1,chNum,0,0))
	{
		EDMA3CCPaRAMEntry param;
		memcpy(&param,pNewPaRAM,sizeof(EDMA3CCParam_st));
		EDMA3QdmaSetPaRAM(BaseAddr,chNum,paRAMId,&param);
		return 0;
	}
	return -1;
}

void EDMA3QdmaMapChToParam(unsigned int chNum,unsigned int *paRAMId)
{
	EDMA3MapQdmaChToPaRAM(BaseAddr,chNum,paRAMId);
}

void EDMA3QdmaSetTrigWord(unsigned int chNum,unsigned int trigWord)
{
	EDMA3SetQdmaTrigWord(BaseAddr,chNum,trigWord);
}

int EDMA3EnableInfoTransfer(unsigned int chNum,unsigned int trigMode)
{
	if(EDMA3EnableTransfer(BaseAddr,chNum,trigMode))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int EDMA3WaitForCompletion(unsigned int nCompCode)
{
    int nTimeOut = 100000;

    while(nCompCode != EDMA3GetIntrStatus(BaseAddr))
    {
        nTimeOut--;
        if(!nTimeOut)
        {
          //  break;
        }
    }

    return (0 == nTimeOut)?(-1):(0);
}
void EDMA3FreeResource(unsigned int chType, unsigned int chNum, unsigned int trigMode)
{
	EDMA3FreeChannel(BaseAddr,chType,chNum,trigMode,0,0);
}

#endif



int EDMA3_Init(unsigned char nCC, unsigned int nCh, unsigned int nParamId, unsigned int intcNum, unsigned int evtQNum, EDMA3CCParam_st Param)
{
	EDMA3SetRegionID(EDMA3_REGIONID);
	switch (nCC)
	{
		case EDMA3_CC0:
		{
			BaseAddr = SOC_EDMA30CC_0_REGS_C667x;
			break;
		}
		case EDMA3_CC1:
		{
			BaseAddr = SOC_EDMA31CC_0_REGS_C667x;
			break;
		}
		case EDMA3_CC2:
		{
			BaseAddr = SOC_EDMA32CC_0_REGS_C667x;
			break;
		}
		default:
		{
			BaseAddr = SOC_EDMA30CC_0_REGS_C667x;
			break;
		}
	}
	EDMA3Init(BaseAddr, evtQNum);
	if (EDMA3RequestChannel(BaseAddr, 0, nCh, intcNum, 0, evtQNum))
	{
		EDMA3ChannelToParamMap(BaseAddr, nCh, nParamId);
		EDMA3SetPaRAM(BaseAddr, nCh, (EDMA3CCPaRAMEntry*)(&Param));
	}
	else
	{
		return -1;
	}
	return 0;
}


#if 0
int EDMA3_InitParam(unsigned char nCh,unsigned int nSrcAddr, unsigned int nDstAddr,unsigned int nLen,EDMA3CCParam_st* pPaRAM)
{
    unsigned int nMaxACnt = 16*1024;
    unsigned int acnt = 0,bcnt = 1, ccnt = 1;
    unsigned int syncType   = EDMA3_SYNCTYPE_A;

    if(!pPaRAM || !nLen)
    {
        return -1;
    }

    memset(pPaRAM,0,sizeof(EDMA3CCParam_st));

    pPaRAM->srcAddr    = (unsigned int)(nSrcAddr);
    pPaRAM->destAddr   = (unsigned int)(nDstAddr);

    if(nLen > nMaxACnt)
    {
        acnt = nMaxACnt;
        bcnt = nLen/nMaxACnt;
        ccnt = 1;

    }
    else
    {
        acnt = nLen;
        bcnt = ccnt = 1;
    }

    pPaRAM->aCnt = (unsigned short)acnt;
    pPaRAM->bCnt = (unsigned short)bcnt;
    pPaRAM->cCnt = (unsigned short)ccnt;

      // 设置 SRC / DES 索引
    pPaRAM->srcBIdx = (short)acnt;
    pPaRAM->destBIdx = (short)acnt;

      if (syncType == EDMA3_SYNCTYPE_A)

      // A Sync 传输模式
     pPaRAM->srcCIdx = (short)acnt;
     pPaRAM->destCIdx = (short)acnt;


     pPaRAM->linkAddr = (unsigned short)0xFFFF;
     pPaRAM->bCntReload = (unsigned short)0;
     pPaRAM->opt = 0u;
      // Src 及 Dest 使用 INCR 模式
     pPaRAM->opt &= 0xFFFFFFFC;
      // 编程 TCC
     pPaRAM->opt |= (nCh<<12);

      // 使能 Intermediate & Final 传输完成中断
     pPaRAM->opt |= (1 << 0x00000015);
     pPaRAM->opt |= (1 << 0x00000014);

  //    if (syncType == EDMA3_SYNCTYPE_A)
     if(nLen < nMaxACnt)
      {
          //A Sync 传输模式
         pPaRAM->opt &= 0xFFFFFFFB;
      }
      else
      {
             // AB Sync 传输模式
         pPaRAM->opt |= (1 << 0x00000002);
        // pPaRAM->opt |= (1 << 0x00000003);
      }

     //no link


      return 0;

}

int EDMA3_SetChParam(unsigned char nCh,unsigned int nParamID,EDMA3CCParam_st* pPaRAM)
{
    if(!pPaRAM)
    {
        return -1;
    }
    EDMA3SetPaRAM(BaseAddr,nParamID,(EDMA3CCPaRAMEntry*)pPaRAM);
    return 0;
}

#endif

int EDMA3_StartChTrans(unsigned char nCh)
{
    if(EDMA3EnableTransfer(BaseAddr,nCh,EDMA3_TRIGMODE_MANUAL))
    {
        return 0;
    }
    else
    {
        return -1;
    }

}

unsigned int EDMA3_GetIntrStatus()
{
    return EDMA3GetIntrStatus(BaseAddr);
}

unsigned int EDMA3_GetHighIntrStatus()
{
	return EDMA3IntrStatusHighGet(BaseAddr);
}

void EDMA3_ClrIntr(unsigned int intcNum)
{
	EDMA3ClrIntr(BaseAddr, intcNum);
}

int EDMA3_UnInit(unsigned char nCh, unsigned int intcNum, unsigned int evtQNum)
{
	if (EDMA3FreeChannel(BaseAddr, 0, nCh, EDMA3_TRIGMODE_MANUAL, intcNum, 0, evtQNum))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


int EDMA3_WaitForTransComplete(unsigned int intcNum)
{
    unsigned int nCompCode = 1<< intcNum;

    int nTimeOut = 0xffffffff;
	if (intcNum < 32)
	{
		while (nCompCode != (nCompCode & EDMA3GetIntrStatus(BaseAddr)))
		{
			nTimeOut--;
			return -1;
		}
	}
	else
	{
		while (nCompCode != (nCompCode & EDMA3IntrStatusHighGet(BaseAddr)))
		{
			nTimeOut--;
			return -1;
		}
	}
    
    EDMA3_ClrIntr(intcNum);
    return 0;
}

