/*
 * API_cache.c
 *
 *  Created on: 2020-9-30
 *      Author: ly
 */
#include "../drv_cache.h"
#include "csl_cacheAux.h"

// CACHE definitions
#define CACHE_BASE          0x01840000
#define CACHE_L2CFG             (*( unsigned int* )( CACHE_BASE ))
#define CACHE_L1PCFG            (*( unsigned int* )( CACHE_BASE + 0x0020 ))
#define CACHE_L1DCFG            (*( unsigned int* )( CACHE_BASE + 0x0040 ))
#define L2WBINV                 (CACHE_BASE + 0x5004) // L2WBINV Control
#define L2INV                   (CACHE_BASE + 0x5008) // L2INV Control
#define L1PINV                  (CACHE_BASE + 0x5028) // L1PINV Control
#define L1DWBINV                (CACHE_BASE + 0x5044) // L1DWBINV Control
#define L1DINV                  (CACHE_BASE + 0x5048) // L1DINV Control



WaitSoft( int nloop )
{
    int i;

    // 1 sec ~ 40000 loop on P4 3.4GHz
    for( i = 0 ; i < nloop ; i++ )
    {
    }
}



void  CfgCache_L1_L2_Size(CacheL1Cfg L1PCfg, CacheL1Cfg L1DCfg, CacheL2Cfg L2Cfg)
{

    CACHE_L1PCFG = L1PCfg;           // L1P on, MAX size
    CACHE_L1DCFG = L1DCfg;           // L1D on, MAX size
    CACHE_L2CFG  = L2Cfg;           // L2 off, use as RAM
}

void Invalidate_Cache(unsigned char nType, void* pStatrt, unsigned int nbycnt, unsigned char blwatiFinsh)
{
	unsigned char blWait = 0;
	if (blwatiFinsh)
	{
		blWait = 1;
	}
	switch (nType)
	{
		case CACHE_L1D:
			CACHE_invL1d(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		case CACHE_L1P:
			CACHE_invL1p(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		case CACHE_L2:
			CACHE_invL2(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		default:
			break;
	}
}

void Writeback_Cache(unsigned char nType, void* pStatrt, unsigned int nbycnt, unsigned char blwatiFinsh)
{
	unsigned char blWait = 0;
	if (blwatiFinsh)
	{
		blWait = 1;
	}
	switch (nType)
	{
		case CACHE_L1D:
			CACHE_wbL1d(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		case CACHE_L2:
			CACHE_wbL2(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		default:
			break;
	}
}

void Invalid_Writeback_Cache(unsigned char nType, void* pStatrt, unsigned int nbycnt, unsigned char blwatiFinsh)
{
	unsigned char blWait = 0;
	if (blwatiFinsh)
	{
		blWait = 1;
	}
	switch (nType)
	{
		case CACHE_L1D:
			CACHE_wbInvL1d(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		case CACHE_L2:
			CACHE_wbInvL2(pStatrt, nbycnt, (CACHE_Wait)blWait);
			break;
		default:
			break;
	}
}

void Invalidata_Cache_All(unsigned char nType, unsigned char blwatiFinsh)
{
	unsigned char blWait = 0;
	if (blwatiFinsh)
	{
		blWait = 1;
	}
	switch (nType)
	{
		case CACHE_L1D:
			CACHE_invAllL1d((CACHE_Wait)blWait);
			break;
		case CACHE_L1P:
			CACHE_invAllL1p((CACHE_Wait)blWait);
			break;
		case CACHE_L2:
			CACHE_invAllL2((CACHE_Wait)blWait);
			break;
		default:
			break;
	}
}

