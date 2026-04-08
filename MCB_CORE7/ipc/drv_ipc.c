/*
 * ipc.c
 *
 *  Created on: 2020-9-20
 *      Author: linxi
 */
#include <stdio.h>
#include <string.h>
#include "../drv_ipc.h"
#include "../drv_intc.h"

// BOOT and CONFIG dsp system modules Definitions
#define CHIP_LEVEL_REG  0x02620000
// Boot cfg registers
#define KICK0			*(unsigned int*)(CHIP_LEVEL_REG + 0x0038)
#define KICK1			*(unsigned int*)(CHIP_LEVEL_REG + 0x003C)
#define KICK0_UNLOCK (0x83E70B13)
#define KICK1_UNLOCK (0x95A4F1E0)
#define KICK_LOCK    0





 /* IPCGR Info */
unsigned int iIPCGRInfo[8] = {
	CHIP_LEVEL_REG + 0x240,
	CHIP_LEVEL_REG + 0x244,
	CHIP_LEVEL_REG + 0x248,
	CHIP_LEVEL_REG + 0x24C,
	CHIP_LEVEL_REG + 0x250,
	CHIP_LEVEL_REG + 0x254,
	CHIP_LEVEL_REG + 0x258,
	CHIP_LEVEL_REG + 0x25C
};
/* IPCAR Info */
unsigned int iIPCARInfo[8] = {
	CHIP_LEVEL_REG + 0x280,
	CHIP_LEVEL_REG + 0x284,
	CHIP_LEVEL_REG + 0x288,
	CHIP_LEVEL_REG + 0x28C,
	CHIP_LEVEL_REG + 0x290,
	CHIP_LEVEL_REG + 0x294,
	CHIP_LEVEL_REG + 0x298,
	CHIP_LEVEL_REG + 0x29C
};



void IPC_SendInterruptToCoreX(unsigned char nNextCore, unsigned int nInfo)
{
	// Unlock Config
	KICK0 = KICK0_UNLOCK;
	KICK1 = KICK1_UNLOCK;

	*(volatile unsigned int *)iIPCGRInfo[nNextCore] = nInfo;

	*(volatile unsigned int *)iIPCGRInfo[nNextCore] |= 1;
	// Unlock Config
	KICK0 = KICK0_UNLOCK;
	KICK1 = KICK1_UNLOCK;
}

void IPC_GetCoreSoureInfo(unsigned char nCore, unsigned int * pInfo)
{
	*pInfo = *(volatile unsigned int*)iIPCGRInfo[nCore];
}

void IPC_ClearSourceInfo(unsigned char nCore)
{
	*(volatile unsigned int *)iIPCARInfo[nCore] = *(volatile unsigned int*)iIPCGRInfo[nCore];
}
