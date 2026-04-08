/*
 *   SRIO 设备抽象层函数
 *
 */

#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/tistdtypes.h>
#include <ti/csl/csl_cpIntcAux.h>
#include <ti/csl/csl_pscAux.h>
#include "srio.h"
#include "../include/hw_srio.h"
#include "../drv_edma3.h"
#include "hw_soc_c6678.h"
// 外设驱动库
#include "../include/hw_types.h"
//#include "srio_debug.h"
#include "../drv_psc.h"
#include <stdio.h>
#include <stdlib.h>
#include <c6x.h>

#include "string.h"
#include "srio_cfg.h"



/****************************************************************************/
/*                                                                          */
/*              宏定义                                                      */
/*                                                                          */
/****************************************************************************/
#define MAX_RAM_COUNT       (2)             //FPGA RAM个数
#define BUFFER_SIZE (4*1024)

#define FPGA_BLOCK_FLAG(block)       (unsigned int )(0xf03000+block*0x10)
#define FPGA_BLOCK_LEN(block)        (unsigned int )(0xf03008 + block*0x10)

#define VGA_BLOCK_STATE              (unsigned int )(0xf03040)

/****************************************************************************/
/*                                                                          */
/*              全局变量定义                                                      */
/*                                                                          */
/****************************************************************************/
static unsigned char g_UpBlockIndex = 0;      //RapidIO 读取camlink输入视频数据RAM块索引，共2块乒乓使用，每块4K
static unsigned char g_DnBlockIndex = 0;      //RapidIO RAM块索引

// 测试数据
unsigned char SRIOSrcBuf[BUFFER_SIZE+512] = {0};

unsigned char SRIODstBuf[BUFFER_SIZE+512] = {0};

unsigned int SRIOFpgaBuf[2] = {0x00000000,0x000a0000};
unsigned int SRIOFpgaFlag[2] = {0xf030a0, 0xf030a8};
static volatile unsigned int g_UpBlockState[2] = {0};
static volatile unsigned int g_DnBlockState[2] = {0};
volatile unsigned int g_BlockLen[2] = {0};

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                      */
/*                                                                          */
/****************************************************************************/
extern void  SRIO_InitErrManageRegs(void);


/****************************************************************************/
/*                                                                          */
/*              SRIO Direct I/O 传输                                        */
/*                                                                          */
/****************************************************************************/

void SRIODirectIOTransfer(unsigned char LSU, SRIOLSUConfig *config)
{
	config->Status.TransactionIndex =  HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU)) & 0xF;

	config->Status.Context =  (HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU)) & 0x10) >> 4;

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG0(LSU)) = config->Address.RapidIOAddress.MSB;
    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG1(LSU)) = config->Address.RapidIOAddress.LSB;//64b aligned
    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG2(LSU)) = config->Address.DSPAddress;

    unsigned int ByteCount;
    if(config->ByteCount >= 1024 * 1024)
    {
    	ByteCount = 1024 * 1024;
    }
    else
    {
    	ByteCount = config->ByteCount;
    }

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG3(LSU)) = ((config->DoorBell.Enable & 0x01)    << 31) |  ByteCount   ;//最大1MB,但不要超过256字节

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG4(LSU)) = ((config->ID.Dest         & 0xFFFF)  << 16) |
                                                  ((config->ID.Src          & 0xF)     << 12) |
                                                  ((config->ID.Size         & 0x2)     << 10) |
                                                  ((config->ID.Port         & 0x2)     <<  8) |
                                                  ((config->Priority        & 0xF)     <<  4) |
                                                  ((config->Xambs           & 0x2)     <<  2) |
                                                  ((config->SupGoodInt      & 0x1)     <<  1) |
                                                  ((config->IntReq          & 0x1)     <<  0);

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG5(LSU)) = ((config->DoorBell.Info   & 0xFFFF)  << 16) |
                                                  ((config->HopCount        & 0xFF)    <<  8) |
                                                  ((config->PacketType      & 0xFF)    <<  0);

    config->ByteCount -= ByteCount;
    config->Address.DSPAddress += ByteCount;

    unsigned long long RapidIOAddress;
    RapidIOAddress = _itoll(config->Address.RapidIOAddress.MSB, config->Address.RapidIOAddress.LSB) + ByteCount;
    config->Address.RapidIOAddress.MSB = _hill(RapidIOAddress);
    config->Address.RapidIOAddress.LSB = _loll(RapidIOAddress);
}

unsigned char SRIOLSUStateIndexTable[8][9]=
{
/* LSU0 */ {0, 1, 2, 3, 4, 5, 6, 7, 8},
/* LSU1 */ {9, 10, 11, 12, 13, 14},
/* LSU2 */ {15, 16, 17, 18, 19},
/* LSU3 */ {20, 21, 22, 23},
/* LSU4 */ {24, 25, 26, 27, 28, 29, 30, 31, 32},
/* LSU5 */ {33, 34, 35, 36, 37, 38},
/* LSU6 */ {39, 40, 41, 42, 43},
/* LSU7 */ {44, 45, 46, 47}
};


void SRIOSerDesPLLSet(unsigned int config)
{
    HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGPLL) = config;
}

unsigned int SRIOSerDesPLLStatus()
{
    return (HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_STS));
}

void SRIOSerDesTxSet(unsigned char index, unsigned int config)
{
    if(index == 0)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX0) = config;
    }
    else if(index == 1)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX1) = config;
    }
    else if(index == 2)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX2) = config;
    }
    else if(index == 3)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX3) = config;
    }
}

unsigned int SRIOSerDesTxStatus(unsigned char index)
{
    if(index == 0)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX0);
    }
    else if(index == 1)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX1);
    }
    else if(index == 2)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX2);
    }
    else if(index == 3)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGTX3);
    }

    return 0;
}

void SRIOSerDesRxSet(unsigned char index, unsigned int config)
{
    if(index == 0)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX0) = config;
    }
    else if(index == 1)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX1) = config;
    }
    else if(index == 2)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX2) = config;
    }
    else if(index == 3)
    {
        HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX3) = config;
    }
}

unsigned int SRIOSerDesRxStatus(unsigned char index)
{
    if(index == 0)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX0);
    }
    else if(index == 1)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX1);
    }
    else if(index == 2)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX2);
    }
    else if(index == 3)
    {
        return HWREG(SOC_DSC_BASE_REGS + SOC_DSC_SRIO_SERDES_CFGRX3);
    }

    return 0;
}


SRIORWStatus SRIOLSUStatusGet(unsigned char LSU, SRIOLSUConfig *config)
{
	unsigned int CompletionCode;

#if 0
	//do
	{
		//CompletionCode = (HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(StateIndex / 8)) >> ((StateIndex & 7) * 4)) & 0xF;
	    if (0 == LSU )
	        CompletionCode = (HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU)) & 0xF);
	    else if (1 == LSU)
	        CompletionCode = ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU)) & 0xF0) >>4);
	    else if (2 == LSU)
	        CompletionCode = ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU-1)) & 0xF0000000) >>28);
	    else if (3 == LSU)
	        CompletionCode = ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU-1)) & 0xF0000) >>16);
	    else if (4 == LSU)
	        CompletionCode = (HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU-1)) & 0xF) ;
	    else if (5 == LSU)
	        CompletionCode = ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU-1)) & 0xF0)>>4);
	    else if (6 == LSU)
	        CompletionCode =  ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU-2)) & 0xF0000000) >>28);
	    else if (7 == LSU)
	        CompletionCode =  ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(LSU-2)) & 0xF0000) >>16);
	}
	//while((CompletionCode & 1) != config->Status.Context);
#else
	unsigned int StateIndex = SRIOLSUStateIndexTable[LSU][config->Status.TransactionIndex];
	int  RetryCount = 0x2;
	do
	{
	        CompletionCode = (HWREG(SOC_SRIO_0_REGS + SRIO_LSU_Status(StateIndex / 8)) >> ((StateIndex & 7) * 4)) & 0xF;
	        RetryCount--;
	 }
	  while(((CompletionCode & 1) != config->Status.Context) && RetryCount>0);
#endif

	return (SRIORWStatus)(CompletionCode >> 1);
}

unsigned int SRIOLSUFullCheck(unsigned char LSU)
{
	return (((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU)) & 0x40000000) >> 30));
}

unsigned int SRIOLSUBusyCheck(unsigned char LSU)
{
    return ((HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU))& 0x80000000)   >> 31);
}
//注意：Busy位和Full位是要一起被读取的，因为如果CPU读取了Full位并获取了LSU寄存器的使用权，busy位马上就会被置1，试图查看busy位之后就会马上导致busy位被置1。所以Busy位和Full位一定要一起读取。
unsigned char  SRIOLSUFullAndBusyCheck(unsigned char LSU)
{
    unsigned int value = HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU));
    unsigned char busyStatus  = (value & 0x80000000)  >> 31;
    unsigned char  fullStatus   = (value & 0x40000000)  >> 30;
    return (busyStatus) | (fullStatus << 4);
}
//void SRIOLSUFlush(unsigned char LSU)
//{
//    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU))  |= 0x01;
//}
//void SRIOLSURestart(unsigned char LSU)
//{
//    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(LSU))  |= 0x02;
//}
void SRIOFlowControl(unsigned char index, unsigned short destID)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_FlowCntl(index)) = (0 << 16) | (destID);
}
/****************************************************************************/
/*                                                                          */
/*              SRIO 使能及禁用                                             */
/*                                                                          */
/****************************************************************************/
void SRIOGlobalEnable()
{
    HWREG(SOC_SRIO_0_REGS + SRIO_GBL_EN) = 1;
    while(0x1  != (HWREG(SOC_SRIO_0_REGS + SRIO_GBL_EN_STAT) & 0x1));
}

void SRIOGlobalDisable()
{
    HWREG(SOC_SRIO_0_REGS + SRIO_GBL_EN) = 0;
    while(HWREG(SOC_SRIO_0_REGS + SRIO_GBL_EN_STAT) & 0x1);
}


void SRIOBlockEnable(unsigned char block)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_BLK_EN(block)) = 1;
    while(0x01 != (HWREG(SOC_SRIO_0_REGS + SRIO_BLK_EN_STAT(block)) & 0x1) );
}

void SRIOBlockDisable(unsigned char block)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_BLK_EN(block)) = 0;
}
unsigned int IsSRIOBlockEnabled(unsigned char block)
{
    return HWREG(SOC_SRIO_0_REGS + SRIO_BLK_EN(block)) ;
}
/****************************************************************************/
/*                                                                          */
/*              SRIO 外设配置                                               */
/*                                                                          */
/****************************************************************************/
void SRIOBootCompleteSet(unsigned char val)
{
	if(val == SRIO_Enable)
	{
		HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL) |= (1 << 24);
	}
	else if(val == SRIO_Disable)
	{
		HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL) &= ~(1 << 24);
	}
}

void SRIOModeSet(unsigned char Lane, unsigned char val)
{
	if(val == SRIO_Loopback)
	{
		HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL1) |= (1 << (4 + Lane));
	}
	else if(val == SRIO_Normal)
	{
		HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL1) &= ~(1 << (4 + Lane));
	}
}

void SRIOAutomaticPriorityPromotionEnable()
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL) &= ~(1 << 21);
}

void SRIOAutomaticPriorityPromotionDisable()
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL) |= (1 << 21);
}

void SRIOPrescalarSelectSet(unsigned char val)
{
    unsigned int savePinmux = 0;

    savePinmux = (HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL) &~(0x000000F0));

    HWREG(SOC_SRIO_0_REGS + SRIO_PER_SET_CNTL) = ((val << 4) | savePinmux);
}

/****************************************************************************/
/*                                                                          */
/*              SRIO 中断                                                   */
/*                                                                          */
/****************************************************************************/
void SRIODoorBellInterruptGenerate(unsigned char Port, unsigned char LSU,
		                               unsigned char IDSize, unsigned short DestinationID,
		                               unsigned short Info)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG0(LSU)) = 0;
    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG1(LSU)) = 0;
    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG2(LSU)) = 0;

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG3(LSU)) = 0x80000004;

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG4(LSU)) = (DestinationID  << 16) |
                                                  ((IDSize & 0x2) << 10) |
                                                  ((Port   & 0x2) <<  8);

    HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG5(LSU)) = (Info           << 16) |
                                                  (SRIO_DoorBell  <<  0);
}

void SRIOLSUPendingInterruptClear(unsigned char LSU, unsigned char id)
{
	if(LSU == 0)
	{
	    HWREG(SOC_SRIO_0_REGS + SRIO_LSU0_ICCR) |= (1 << id);
	}
	else if(LSU == 1)
	{
	    HWREG(SOC_SRIO_0_REGS + SRIO_LSU1_ICCR) |= (1 << id);
	}
}

unsigned int SRIODoorBellInterruptGet(unsigned char DoorBellNo)
{
    return HWREG(SOC_SRIO_0_REGS + SRIO_DoorBell_ICSR(DoorBellNo));
}

//DoorBellNo只能是0-3,  DoorBell选择0-15(位), 1为清除, 0不起作用
void SRIODoorBellInterruptClear(unsigned char DoorBellNo, unsigned char DoorBell)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_DoorBell_ICCR(DoorBellNo)) |= (1 << DoorBell);
}
void SRIOErrRstEventInterruptClear()
{
    HWREG(SOC_SRIO_0_REGS + SRIO_ERR_RST_EVNT_ICCR) = 0xFFFFFFFF;
}
void SRIODoorBellInterruptRoutingControl(unsigned char value)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_InterruptControl) = value;
}
/*本函数为指定门铃寄存器发送路由中断 到指定目的地.
*     routes the doorbell interrupts for the specified doorbell  register to a specific destination.
*     Route the Doorbell bits 'DoorBellBit' for Doorbell 'DoorBellNo' to destination 'InterruptDestination'
*/
void SRIODoorBellInterruptConditionRoutingSet(unsigned char DoorBellNo, unsigned char DoorBellBit, unsigned char InterruptDestination)
{
	if(DoorBellBit < 8) //处理0-7的中断请求
	{
		HWREG(SOC_SRIO_0_REGS + SRIO_DoorBell_ICRR1(DoorBellNo)) |= (InterruptDestination << (DoorBellBit * 4));
	}
	else if(DoorBellBit >= 8 && DoorBellBit < 16)//处理8-15的中断请求
	{
		HWREG(SOC_SRIO_0_REGS + SRIO_DoorBell_ICRR2(DoorBellNo)) |= (InterruptDestination << ((DoorBellBit - 8) * 4));;
	}
}

/****************************************************************************/
/*                                                                          */
/*              SRIO ID                                                     */
/*                                                                          */
/****************************************************************************/
//void SRIODeviceInfoSet(unsigned short VendorID, unsigned int Revision)//按SRIO参考文档上写的
//{
//	HWREG(SOC_SRIO_0_REGS + SRIO_DeviceID)   =  (0x009D << (0x00000010u)) |(VendorID); //((ID << 16) | VendorID);
//	HWREG(SOC_SRIO_0_REGS + SRIO_DeviceINFO) = Revision; //The lower 4b should match the 4b from the JTAG Variant field of   the DeviceID register. 这里??
//}
void SRIODeviceInfoSet(unsigned short ID, unsigned short VendorID, unsigned int Revision)//按创龙的代码来的
{
    HWREG(SOC_SRIO_0_REGS + SRIO_DeviceID)   = ((ID << 16) | VendorID);
    HWREG(SOC_SRIO_0_REGS + SRIO_DeviceINFO) = Revision; //The lower 4b should match the 4b from the JTAG Variant field of   the DeviceID register. 这里??
}

void SRIOAssemblyInfoSet(unsigned short ID, unsigned short VendorID, unsigned short Revision, unsigned short ExtendedFeaturesPtr)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_AssemblyID)   = ((ID << 16) | VendorID);
	HWREG(SOC_SRIO_0_REGS + SRIO_AssemblyINFO) = ((Revision << 16) | ExtendedFeaturesPtr);
}

unsigned long long SRIODeviceInfoGet()
{
	return (_itoll(HWREG(SOC_SRIO_0_REGS + SRIO_DeviceINFO), HWREG(SOC_SRIO_0_REGS + SRIO_DeviceID)));
}

unsigned long long SRIOAssemblyInfoGet()
{
	return 	(_itoll(HWREG(SOC_SRIO_0_REGS + SRIO_AssemblyINFO), HWREG(SOC_SRIO_0_REGS + SRIO_AssemblyID)));
}

void SRIOHostDeviceIDSet(unsigned short ID)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_HOSTDeviceID) = ID;
}

void SRIOComponentTagSet(unsigned int value)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_ComponentTag) = value;
}

unsigned short SRIOHostDeviceIDGet()
{
	return HWREG(SOC_SRIO_0_REGS + SRIO_HOSTDeviceID);
}

unsigned int SRIOComponentTagGet()
{
	return HWREG(SOC_SRIO_0_REGS + SRIO_ComponentTag);
}

void SRIOPortWriteTargetDeviceID(unsigned char MSB, unsigned char LSB, unsigned char cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PortWriteTargetDeviceID) = ((MSB << 24) | (LSB << 16) | ((cfg & 0x1) << 15));
}

/****************************************************************************/
/*                                                                          */
/*              SRIO CSR / CAR                                              */
/*                                                                          */
/****************************************************************************/
void SRIOProcessingElementFeaturesSet(unsigned int cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PEFeature) = cfg;
}

void SRIOSourceOperationsSet(unsigned int cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_SourceOP) = cfg;
}

void SRIODestinationOperationsSet(unsigned int cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_DestinationOP) = cfg;
}

void SRIODeviceIDSet(unsigned char BaseID, unsigned short VendorIDEntity)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_BaseDeviceID) = ((BaseID << 16) | VendorIDEntity);
}

void SRIOInputPortEnable(unsigned char Port)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PortControl(Port)) |= (1 << 21);
}

void SRIOOutputPortEnable(unsigned char Port)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PortControl(Port)) |= (1 << 22);
}

void SRIOSetSPCtl(unsigned char Port ,unsigned int cfg)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_PortControl(Port)) = cfg;
}
void SRIOSetSPCtl2(unsigned char Port ,unsigned int cfg)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_SPCTL2(Port)) = cfg;
}
unsigned int  SRIOGetSPCtl2(unsigned char Port)
{
    return HWREG(SOC_SRIO_0_REGS + SRIO_SPCTL2(Port));
}

void SRIOPortLinkTimeoutSet(unsigned int val)
{
    HWREG(SOC_SRIO_0_REGS + SRIO_PortLinkTimeout) &= 0xF;
	HWREG(SOC_SRIO_0_REGS + SRIO_PortLinkTimeout)  |= (val << 8);
}

void SRIOPortGeneralSet(unsigned char Host, unsigned char Master, unsigned char Discovered)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PortGeneralControl) = (((Host       & 0x1) << 31) |
                                                        ((Master     & 0x1) << 30) |
                                                        ((Discovered & 0x1) << 29));
}

/****************************************************************************/
/*                                                                          */
/*              SRIO TLM                                                    */
/*                                                                          */
/****************************************************************************/
//This register controls how the Base Routing Register (BRR) is applied to 入站 packets.
void SRIOTLMPortBaseRoutingSet(unsigned char Port, unsigned char BRR, unsigned char Enable, unsigned char RouteMaintenanceRequestToLLM, unsigned char Private)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_TLM_SP_BRR_Control(Port, BRR)) = (((Enable                       & 0x1) << 31) |
                                                                   ((RouteMaintenanceRequestToLLM & 0x1) << 26) |
                                                                   ((Private                      & 0x1) << 24));
}

//这个寄存器提供了入站destID进行比较的模式，以及哪些位参与比较。
void SRIOTLMPortBaseRoutingPatternMatchSet(unsigned char Port, unsigned char BRR, unsigned short Pattern, unsigned short Match)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_TLM_SP_BRR_Pattern_Match(Port, BRR)) = ((Pattern << 16) | Match);
}

/****************************************************************************/
/*                                                                          */
/*              SRIO PLM                                                    */
/*                                                                          */
/****************************************************************************/
//cfg 设置为 0是不合法的
void SRIOPLMPortSilenceTimerSet(unsigned char Port, unsigned char cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_SilenceTimer(Port)) = ((cfg & 0xF) << 28);
}
//cfg设置为0,或者1都是非法的, 默认7, 最多只有0,2端口需要设
void SRIOPLMPortDiscoveryTimerSet(unsigned char Port, unsigned char cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_DiscoveryTimer(Port)) = ((cfg & 0xF) << 28);
}

void SRIOPLMPortForeceReLink(unsigned char Port)
{
    unsigned int nValue = 0;
    nValue = (HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_IMP_SPEC_Ctrl(Port)) &~(0x04000000));

    HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_IMP_SPEC_Ctrl(Port)) = (nValue | 0x04000000);

}

void SRIOPMMSoftRestPort(unsigned char nPort)
{
    unsigned int nValue = 0;
    nValue = (HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_IMP_SPEC_Ctrl(nPort)) &~(0x02000000));

    HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_IMP_SPEC_Ctrl(nPort)) = (nValue | 0x02000000);

}
void SRIOPLMPathModeControl(unsigned char Port, unsigned char Mode)
{
    unsigned int savePinmux = 0;
    //此寄存器只有0`2字段是可写, 000 对应Mode0, 001对应Mode1, 010-Mode2,011-Mode3, 100-Mode4
    savePinmux = (HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_PathModeControl(Port)) &~(0x00000007));

    HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_PathModeControl(Port)) = (Mode | savePinmux);
}

/****************************************************************************/
/*                                                                          */
/*              SRIO   port-write reception capture 0-3 CSR          3.17.70                                           */
/*                                                                          */
/****************************************************************************/
void SRIOPortWriteRxCapture(unsigned char Port, unsigned int cfg)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PortWriteRxCapture(Port)) = cfg;
}

void SRIORegisterResetControlClear()
{
	HWREG(SOC_SRIO_0_REGS + SRIO_RegisterResetControl) |= 1;
}

void SRIODataDtreamingLogicalLayerControl(unsigned char MTU)
{
	Uint32 tmp = (HWREG(SOC_SRIO_0_REGS + SRIO_DataDtreamingLogicalLayerControl)&0xFFFFFF00) ;
	HWREG(SOC_SRIO_0_REGS + SRIO_DataDtreamingLogicalLayerControl) = tmp | MTU;
}

void SRIOServerClockPortIPPrescalar(unsigned char Div)
{
	Uint32 tmp = (HWREG(SOC_SRIO_0_REGS + SRIO_ServerClockPortIPPrescalar)&0xFFFFFF00) ;
	HWREG(SOC_SRIO_0_REGS + SRIO_ServerClockPortIPPrescalar) = tmp | Div;
}

void SRIOPeripheralEnable()
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PeripheralControl) |= (1 << 2);
}

void SRIOPeripheralDisable()
{
	HWREG(SOC_SRIO_0_REGS + SRIO_PeripheralControl) &= ~(1 << 2);
}

unsigned char SRIOPortOKCheck(unsigned char Port)
{
	return ((HWREG(SOC_SRIO_0_REGS + SRIO_PortErrorStatus(Port)) & 0x2) >> 1);
}



/****************************************************************************/
/*                                                                          */
/*              SRIO 初始化                                                 */
/*                                                                          */
/****************************************************************************/
SRIOInitStatus  SRIOInit(unsigned char uMode, unsigned char DSPID, SRIOLoopbackMode  loopbackMode)
{
    int i = 0;
    unsigned int retryCount = 0xFFFF;

    {
            // 使能外设, 已在Global_Default_Init()里面调用
            //PSCModuleControl(HW_PSC_SRIO, PSC_MDCTL_NEXT_ENABLE, PSC_POWERDOMAIN_SRIO, PSC_PDCTL_NEXT_ON);

            //{{{--- RapidIO外设内部各个单元
            //          RapidIO外设内部单元支持独立的掉电控制，关闭不用的模块可以降低功耗
            //          由BLK_EN控制具体见“SRIO User Guide”中“2.3.10 Reset and Powerdown“ 章节
            // 禁用 SRIO 及 SRIO Block
            for(i = 1; i <= 9; i++)
            {
                SRIOBlockDisable(i);
            }
            SRIOBlockDisable(0);
            SRIOGlobalDisable();

            // 设置 PER_SET_CNTL 寄存器中的 BOOT_COMPLETE 为 0
            // 以便可以修改 SRIO 所有寄存器包括只读（Read Only）寄存器
            SRIOBootCompleteSet(SRIO_Disable);

            // 使能 SRIO 及 SRIO Block
            SRIOGlobalEnable();

            for(i = 0; i <= 9; i++)
            {
                SRIOBlockEnable(i);//5`8对应port0~3
            }
           ///---}}}
    }

        // 配置 SRIO Lane 工作模式
    if (SRIO_DIGITAL_LOOPBACK != loopbackMode )
    {
        for (i = 0; i < 4; i++)
              SRIOModeSet(i, SRIO_Normal);
    }
    else
    {
        for (i = 0; i < 4; i++)
             SRIOModeSet(i, SRIO_Loopback);
    }
        // 使能自动优先级提升
        SRIOAutomaticPriorityPromotionEnable();


        //{{{--- SRIO SerDes 时钟配置，参考SRIO User Guide 中
        //          3.1.2 SerDes Macro Configuration Register
        //          3.2 SerDes Receive/Transmit Channel Configuration Registers

        // 解锁关键寄存器
        KickUnlock();

        // 配置 SRIO SerDes 时钟（250Mhz  ）
        SRIOSerDesPLLSet(0x29);//0x29

        //设置参考时钟.  参数prescalar_Clock的计算方法如下，其中[]的意思是取最近的整数
        //prescalarClock=[refclk÷10]
        SRIOServerClockPortIPPrescalar(0x19);

        // 设置 SRIO VBUS 预分频: 由于输入时钟为250MHz，根据文档sprugw1b.pdf中的Table3-19，得出预分频设置为223.7-447.4最恰当
        // 注意：预分频对时钟影响不大，一般都可以将该参数默认设置为0
        SRIOPrescalarSelectSet(4);

        // 配置 SRIO SerDes 发送 / 接收
        /* Configure the SRIO SERDES Receive/Transfer Configuration. */
        if (SRIO_SERDES_LOOPBACK == loopbackMode )
        {
            SRIOSerDesRxSet(0, 0x1C40485);
            SRIOSerDesRxSet(1, 0x1C40485);
            SRIOSerDesRxSet(2, 0x1C40495);
            SRIOSerDesRxSet(3, 0x1C40495);

            SRIOSerDesTxSet(0, 0x0780785);
            SRIOSerDesTxSet(1, 0x0780785);
            SRIOSerDesTxSet(2, 0x0780795);
            SRIOSerDesTxSet(3, 0x0780795);
        }
        else
        {
            SRIOSerDesRxSet(0, 0x00440485);  //设置laneA  5G
            SRIOSerDesRxSet(1, 0x00440485);  //设置laneB
            SRIOSerDesRxSet(2, 0x00440495); //2.5G
            SRIOSerDesRxSet(3, 0x00440495);
#if  0
                    //临时测试
            SRIOSerDesRxSet(0, 0x00468485);  //设置laneA  5G
            SRIOSerDesRxSet(1, 0x00468485);  //设置laneB
            SRIOSerDesRxSet(2, 0x00468495); //2.5G
            SRIOSerDesRxSet(3, 0x00468495);
#endif

            SRIOSerDesTxSet(0, 0x00180785);  //设置laneA
            SRIOSerDesTxSet(1, 0x00180785);  //设置laneB
            SRIOSerDesTxSet(2, 0x00180795);
            SRIOSerDesTxSet(3, 0x00180795);
        }


        //使能3.125G,  5G,  2.5G波特率
       SRIOSetSPCtl2(0,   1<<18);//18  -  5G
       SRIOSetSPCtl2(1,   1<<18);
       SRIOSetSPCtl2(2,   1<<22);//22 - 2.5G ; 20 - 3.125G
       SRIOSetSPCtl2(3,   1<<22);

        // 等待 SRIO SerDes 锁定
        while (!(SRIOSerDesPLLStatus() & 0x1))
        {
            retryCount--;
          if (!retryCount)
          {
              printf("PLL lock failed.\r\n");
              return SRIO_INIT_PLL_LOCKFAIL;
          }
        }
    //---}}}
        //Capability Registers (CAR) and Command and Status Registers (CSR).
        // 设置设备信息, 厂家ID  0x30, 写SRIO的CSR/CAR寄存器
       SRIODeviceInfoSet(DSPID,  DEVICE_VENDOR_ID, DEVICE_REVISION);//这里改成DEVICE_TYPE_ID也不影响和FPGA通信

       // 设置组织信息
       SRIOAssemblyInfoSet(DEVICE_ASSEMBLY_ID, DEVICE_ASSEMBLY_VENDOR_ID, DEVICE_ASSEMBLY_REVISION, DEVICE_ASSEMBLY_INFO);

       // PE 特性配置，器件支持 8- and 16-bit destIDs 支持 34 bit地址等
       // 具体请查看 CSR/CAR 寄存器中的 PE_FEAT 寄存器
        SRIOProcessingElementFeaturesSet(0x20000199);

        // 配置源及目标操作，使其支持读写操作流操作原子操作等
        SRIOSourceOperationsSet(0x0004FDF4);
        SRIODestinationOperationsSet(0x0000FC04);

        // 设置 DSP Base_ID
        SRIODeviceIDSet(DSPID, 0xABCD);
        //SRIOHostDeviceIDSet(DSPID);

        // Write Target Device ID , 即FPGA的器件ID
        //if((unsigned int)ID_SMALL_LARGE)
        //{
        //    SRIOPortWriteTargetDeviceID((unsigned char)(FPGA_DEVICE_ID_16BIT>>8), \
        //        (unsigned char)FPGA_DEVICE_ID_16BIT, SRIO_ID_16Bit);
        //}
        //else
        //{
        //    SRIOPortWriteTargetDeviceID(0x00, \
        //        (unsigned char)FPGAID, SRIO_ID_8Bit);
        //}

        // 配置 TLM 基本路由信息3.17.26, , 只和FPGA交流不需要配
        //相关的寄存器一共有4组，SP0 --- SP4, 分别对应一个端口, 也就是portNum, 每个portNum下又有4组BRR
        {//暂时好像都可以, 至少没影响环回
    #if 1
            SRIOTLMPortBaseRoutingSet(SRIO_Port0 , 1, SRIO_Enable, SRIO_Enable, SRIO_Disable);
            SRIOTLMPortBaseRoutingSet(SRIO_Port2 , 1, SRIO_Enable, SRIO_Enable, SRIO_Disable);
            SRIOTLMPortBaseRoutingSet(SRIO_Port3 , 1, SRIO_Enable, SRIO_Enable, SRIO_Disable);
    #else
            SRIOTLMPortBaseRoutingSet(SRIO_Port0 , 1, SRIO_Disable, SRIO_Enable, SRIO_Enable);
          //  SRIOTLMPortBaseRoutingSet(SRIO_Port1 , 1, SRIO_Disable, SRIO_Enable, SRIO_Enable);
            SRIOTLMPortBaseRoutingSet(SRIO_Port2 , 1, SRIO_Disable, SRIO_Enable, SRIO_Enable);
            SRIOTLMPortBaseRoutingSet(SRIO_Port3 , 1, SRIO_Disable, SRIO_Enable, SRIO_Enable);
    #endif
        }
        //TODO  这里改成其他设备的ID
        SRIOTLMPortBaseRoutingPatternMatchSet(SRIO_Port0, 1, FPGA_DEVICE_ID_8BIT, 0xFF);
        SRIOTLMPortBaseRoutingPatternMatchSet(SRIO_Port2, 1,  0x11, 0xFF);//手册中搜索RIO_DEVICEID_REG,
     //   SRIOTLMPortBaseRoutingPatternMatchSet(SRIO_Port3, 1,  0x70, 0xFFFF);//TODO
     //   SRIOTLMPortBaseRoutingPatternMatchSet(SRIO_Port2, 1, 0x40, 0xFFFF);

       // 配置端口 PLM
           // 配置 PLM 端口 Silence Timer
           SRIOPLMPortSilenceTimerSet(SRIO_Port0, 0x2);
           SRIOPLMPortSilenceTimerSet(SRIO_Port2, 0x2);
           SRIOPLMPortSilenceTimerSet(SRIO_Port3, 0x2);
           SRIOComponentTagSet(DSPID);//配置组件tag寄存器
       // 使能端口,   可配置0-3
          SRIOSetSPCtl( SRIO_Port0 ,    (1 << 22) | (1 << 21) );
          SRIOSetSPCtl( SRIO_Port2 ,    (1 << 22) | (1 << 21)   );
          SRIOSetSPCtl( SRIO_Port3 ,    (1 << 22) | (1 << 21)  );
    // 配置 PLM 端口 Discovery Timer
        //SRIOPLMPortDiscoveryTimerSet(SRIO_Port0, 0x7);//TODO
        //SRIOPLMPortDiscoveryTimerSet(SRIO_Port2, 0x7);//TODO
        SRIOPortWriteRxCapture(SRIO_Port0, 0);     //重设端口写异常捕获
        SRIOPortWriteRxCapture(SRIO_Port2, 0);   //  重设端口写异常捕获
        SRIOPortWriteRxCapture(SRIO_Port3, 0);   //  重设端口写异常捕获

        // 配置端口连接超时,设置为0x0时则不使能超时 3.14.18
        SRIOPortLinkTimeoutSet(0x20);//2021.7.24  js modified , orgin 0xFFFFFF, 0x8e00是50us
        // 配置端口响应超时
        //SRIOPortRespTimeoutSet(0xFFFFFF);// 默认就是， 不用设置
        // 作为主设备，MASTER_ENABLE
        // 主设备应当负责系统的探测，初始化，维护  3.14.20
        SRIOPortGeneralSet(SRIO_Disable, SRIO_Enable, SRIO_Disable);

    // 清除 Sticky Register 位, 允许SELF_RST和 PWDN_PORT 重置以清除sticky register bits in addition to the normal configuration registers.
        SRIORegisterResetControlClear();

        // 设置端口写目标设备 ID.
        SRIOPortWriteTargetDeviceID(0, 0x0, SRIO_ID_8Bit);// 和0x1B934的寄存器有关联RIO_EM_DEV_PW_EN

        // 设置数据流最大传输单元（MTU）, 64代表256
        //SRIODataDtreamingLogicalLayerControl(64);

        // 配置链路模式, 可配4个
        //这个特定于实现的per-Path寄存器表示通道资源的数量，并控制将这些资源分配给端口。
        SRIOPLMPathModeControl(SRIO_Port0 , uMode);
        SRIOPLMPathModeControl(SRIO_Port2 , uMode);
        SRIOPLMPathModeControl(SRIO_Port3 , uMode);

//        for (i = 0; i < 10; i++)
//            SRIOFlowControl(i, 0x10*(i+1));



     // 设置 LLM Port IP 预分频, to handle different clock frequencies.
        //SRIOServerClockPortIPPrescalar(0x19);
        // 使能外设
        SRIOPeripheralEnable();

        // 锁定关键寄存器
         KickLock();

        // 让DSP SRIO数据使用大端模式，与FPGA匹配
        //SRIOBytesSWAPSet(0xA0053800);

        // 配置完成
        SRIOBootCompleteSet(SRIO_Enable);


        // 此代码检查端口是否可运行。
        for (i= 0; i < 4; i++)
        {
                retryCount = 0xFFFFF;
                if (1 == i)    continue;
                while(  SRIOPortOKCheck(SRIO_Port0 + i) != TRUE)
                {
                    retryCount--;
                    if (!retryCount)
                    {
                        //SRIOPLMPortForeceReLink(SRIO_Port0 + i);
                        //SRIOPMMSoftRestPort(SRIO_Port0 + i);

                        // printf("port %d check port ok failed. \r\n", SRIO_Port0 + i);
                        //break;
                        return  (SRIOInitStatus)(SRIO_INIT_PORT_0_NOTOK +  i) ;
                    }
                }
          }

    #if 0
        i = 100000;
        SRIOPLMPortForeceReLink(0);
        while(i--){};
    #endif
        ////尝试发送数据包之前, 检查错误条件，清除SP(n)_ERR_STAT和SP(n)_ERR_DET寄存器中的错误状态位
     //       SRIO_EnableErrorCapture(SRIO_Enable);

     //       SRIOClearAllIntcStatus();
        SRIO_InitErrManageRegs();

        *((unsigned int *)(0x01840040)) = 0x3;//CACHE_L1DCFG, L1数据配置缓冲区寄存器宏定义,设置为16K, CorePac用户指南Table3-1
        return  SRIO_INIT_OK;
}



#if 0
static void test_isr_srio_mix (void* handle)
{

	pEC1900Hanle_st pHnd = (pEC1900Hanle_st)handle;
	unsigned int nRead = SRIODoorBellInterruptGet(0);//读中断状态寄存器

	if(0x4 == nRead&0x4)//硬同步
	{
		if (pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blping)
		{
			SRIO_ReadHDSYNCInfoByDoorbell(pHnd->RecvData[EC_HDSYNC_RECVE_DATA].pRxFifoPing);
			pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blping = 0;
			pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blDataping = 1;
			pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blDatapong = 0;
		}
		else
		{
			SRIO_ReadHDSYNCInfoByDoorbell(pHnd->RecvData[EC_HDSYNC_RECVE_DATA].pRxFifoPong);
			pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blping = 1;
			pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blDataping = 0;
			pHnd->RecvData[EC_HDSYNC_RECVE_DATA].blDatapong = 1;
		}
		SRIODoorBellInterruptClear(0,2);
	}
	else if(0x8 == nRead&0x8)//异步422
	{
		if (pHnd->RecvData[EC_ASYN422_RECVE_DATA].blping)
		{
			SRIO_ReadRS422InfoByDoorbell(pHnd->RecvData[EC_ASYN422_RECVE_DATA].pRxFifoPing);
			pHnd->RecvData[EC_ASYN422_RECVE_DATA].blping = 0;
			pHnd->RecvData[EC_ASYN422_RECVE_DATA].blDataping = 1;
			pHnd->RecvData[EC_ASYN422_RECVE_DATA].blDatapong = 0;
		}
		else
		{
			SRIO_ReadRS422InfoByDoorbell(pHnd->RecvData[EC_ASYN422_RECVE_DATA].pRxFifoPong);
			pHnd->RecvData[EC_ASYN422_RECVE_DATA].blping = 1;
			pHnd->RecvData[EC_ASYN422_RECVE_DATA].blDataping = 0;
			pHnd->RecvData[EC_ASYN422_RECVE_DATA].blDatapong = 1;
		}
		SRIODoorBellInterruptClear(0,3);
		g_Asyn422Serial++;
	}
	else if(1 == nRead&0x1)//版本信息
	{
		if (pHnd->RecvData[EC_SYS_VER].blping)
		{
			SRIO_ReadSysVerInfoByDoorbell(pHnd->RecvData[EC_SYS_VER].pRxFifoPing);
			pHnd->RecvData[EC_SYS_VER].blping = 0;
			pHnd->RecvData[EC_SYS_VER].blDataping = 1;
			pHnd->RecvData[EC_SYS_VER].blDatapong = 0;
		}
		else
		{
			SRIO_ReadSysVerInfoByDoorbell(pHnd->RecvData[EC_SYS_VER].pRxFifoPong);
			pHnd->RecvData[EC_SYS_VER].blping = 1;
			pHnd->RecvData[EC_SYS_VER].blDataping = 0;
			pHnd->RecvData[EC_SYS_VER].blDatapong = 1;
		}
		SRIODoorBellInterruptClear(0,0);
	}

	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(0)) = 0;
	INTC_CIC_ClearSysInter(0,112,4);
}

static void test_isr_srio_Syn422(void* hand)
{
	pEC1900Hanle_st pHnd = (pEC1900Hanle_st)hand;
	if (pHnd->RecvData[EC_SYN422_RECVE_DATA].blping)
	{
		SRIO_ReadUS422InfoByDoorbell(pHnd->RecvData[EC_SYN422_RECVE_DATA].pRxFifoPing);
		pHnd->RecvData[EC_SYN422_RECVE_DATA].blping = 0;
		pHnd->RecvData[EC_SYN422_RECVE_DATA].blDataping = 1;
		pHnd->RecvData[EC_SYN422_RECVE_DATA].blDatapong = 0;
	}
	else
	{
		SRIO_ReadUS422InfoByDoorbell(pHnd->RecvData[EC_SYN422_RECVE_DATA].pRxFifoPong);
		pHnd->RecvData[EC_SYN422_RECVE_DATA].blping = 1;
		pHnd->RecvData[EC_SYN422_RECVE_DATA].blDataping = 0;
		pHnd->RecvData[EC_SYN422_RECVE_DATA].blDatapong = 1;
	}
	SRIODoorBellInterruptClear(0,1);
	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(1)) = 0;
	INTC_CIC_ClearSysInter(0,113,5);
	g_Syn422Serial++;
}

static void test_isr_srio_Can0(void* hand)
{
	pEC1900Hanle_st pHnd = (pEC1900Hanle_st)hand;
	if (pHnd->RecvData[EC_CAN0_RECVE_DATA].blping)
	{
		SRIO_ReadCan0InfoByDoorbell(pHnd->RecvData[EC_CAN0_RECVE_DATA].pRxFifoPing);
		pHnd->RecvData[EC_CAN0_RECVE_DATA].blping = 0;
		pHnd->RecvData[EC_CAN0_RECVE_DATA].blDataping = 1;
		pHnd->RecvData[EC_CAN0_RECVE_DATA].blDatapong = 0;
	}
	else
	{
		SRIO_ReadCan0InfoByDoorbell(pHnd->RecvData[EC_CAN0_RECVE_DATA].pRxFifoPong);
		pHnd->RecvData[EC_CAN0_RECVE_DATA].blping = 1;
		pHnd->RecvData[EC_CAN0_RECVE_DATA].blDataping = 0;
		pHnd->RecvData[EC_CAN0_RECVE_DATA].blDatapong = 1;
	}
	SRIODoorBellInterruptClear(0,4);
	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(2)) = 0;
	INTC_CIC_ClearSysInter(0,114,6);
	g_Can0Serial++;
}

static void test_isr_srio_Can1(void* hand)
{
	pEC1900Hanle_st pHnd = (pEC1900Hanle_st)hand;
	if (pHnd->RecvData[EC_CAN1_RECVE_DATA].blping)
	{
		SRIO_ReadCan1InfoByDoorbell(pHnd->RecvData[EC_CAN1_RECVE_DATA].pRxFifoPing);
		pHnd->RecvData[EC_CAN1_RECVE_DATA].blping = 0;
		pHnd->RecvData[EC_CAN1_RECVE_DATA].blDataping = 1;
		pHnd->RecvData[EC_CAN1_RECVE_DATA].blDatapong = 0;
	}
	else
	{
		SRIO_ReadCan1InfoByDoorbell(pHnd->RecvData[EC_CAN1_RECVE_DATA].pRxFifoPong);
		pHnd->RecvData[EC_CAN1_RECVE_DATA].blping = 1;
		pHnd->RecvData[EC_CAN1_RECVE_DATA].blDataping = 0;
		pHnd->RecvData[EC_CAN1_RECVE_DATA].blDatapong = 1;
	}
	SRIODoorBellInterruptClear(0,5);
	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(3)) = 0;
	INTC_CIC_ClearSysInter(0,115,7);
	g_Can1Serial++;
}

static void test_isr_srio_Camlink(void* hand)
{
	pEC1900Hanle_st pHnd = (pEC1900Hanle_st)hand;
	if (pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blping)
	{
		g_paramSet.destAddr = (unsigned int)(pHnd->RecvData[EC_CAMLINK_RECVE_DATA].pRxFifoPing);
		EDMA3_SetChParam(0, 0, &g_paramSet);
		EDMA3_StartChTrans(0);
		pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blping = 0;
		pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blDataping = 1;
		pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blDatapong = 0;
	}
	else
	{
		g_paramSet.destAddr = (unsigned int)(pHnd->RecvData[EC_CAMLINK_RECVE_DATA].pRxFifoPong);
		EDMA3_SetChParam(0, 0, &g_paramSet);
		EDMA3_StartChTrans(0);
		pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blping = 1;
		pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blDataping = 0;
		pHnd->RecvData[EC_CAMLINK_RECVE_DATA].blDatapong = 1;
	}
	SRIODoorBellInterruptClear(0,6);
	INTC_CIC_ClearSysInter(0,116,32);
	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(4)) = 0;
	memcpy((unsigned char*)(0x80100000 + g_CamLinkSerial * 4), (unsigned char*)(SOC_DDR3_0_DATA_REGS + SRIO_RECV_CAMLINK), 4);
	g_CamLinkSerial++;
}

#endif





#if 0

int SRIODoorbellIniterruptCfg(unsigned char DoorBellNo)
{
#if 1
	//配置中断控制寄存器
	SRIODoorBellInterruptRoutingControl(0);

	//设置ICRR1，ICRR2寄存器，设置七个不同的事件
	int i=0,serial = 0;
	for(i = 0; i < 7; i++)
	{
//		if((i == 0)||(i == 2)||(i == 3))//将版本信息，硬同步，异步422路由到INTDST0
//		{
//			SRIODoorBellInterruptConditionRoutingSet(DoorBellNo ,i ,0);
//		}
//		else
//		{
//			//将同步422路由到INTDST1
//			//Can0------>INTDST2
//			//CAN1------>INTDST3
//			//CamLink--->INTDST4
//			serial++;
			SRIODoorBellInterruptConditionRoutingSet(DoorBellNo ,i ,0);
		//}
	}

//	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(0)) = 0;
//	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(1)) = 0;
//	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(2)) = 0;
//	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(3)) = 0;
//	HWREG(SOC_SRIO_0_REGS + SRIO_INTDST_Rate_Cnt(4)) = 0;

	if(0 != INTC_Open(20,4,test_isr_srio_mix, NULL))
	{
		return -1;
	}

//	if(0 != INTC_Open(93,5,test_isr_srio_Syn422, (void*)pHnd))
//	{
//		return -1;
//	}
//
//	if(0 != INTC_Open(94,6,test_isr_srio_Can0, (void*)pHnd))
//	{
//		return -1;
//	}
//
//	if(0 != INTC_Open(95,7,test_isr_srio_Can1, (void*)pHnd))
//	{
//		return -1;
//	}
//
//	if(0 != INTC_Open(21,8,test_isr_srio_Camlink, (void*)pHnd))
//	{
//		return -1;
//	}
//
//	INTC_CIC_Init(0,112,4);
//
//	INTC_CIC_Init(0,113,5);
//
//	INTC_CIC_Init(0,114,6);
//
//	INTC_CIC_Init(0,115,7);
//
//	INTC_CIC_Init(0,116,32);

	//启用EDMA3传输图像

#endif
	return 0;
}

#endif

void SRIOClearAllInterrupt(unsigned char DoorBellNo,unsigned int nValue)
{
	HWREG(SOC_SRIO_0_REGS + SRIO_DoorBell_ICCR(DoorBellNo)) = nValue;
}




unsigned int SRIO_SwapDWord(unsigned int nData)
{
    return (((nData&0xff000000) >> 24) | ((nData&0x00ff0000) >> 8) | ((nData&0x0000ff00) << 8) | ((nData&0x000000ff) << 24));
}

//NREAD 读取FPGA 缓冲区有效数据长度传输配置
//TRUE 可用
//FALSE 不可用
int SRIO_ReadCamRamState(unsigned char nBlock)
{
    //检查FPGA RAM块是否可用g_UpBlockIndex
    unsigned char lsuNO= 0;

    SRIO_DirectIO( (unsigned int)&g_UpBlockState[0], FPGA_BLOCK_FLAG(nBlock),  FPGA_DEVICE_ID_8BIT, 8, SRIO_Port0, lsuNO,
                   SRIO_NRead , 0xa5a5);

   return SRIO_SwapDWord(g_UpBlockState[0]);
}

int SRIO_ClearCamRamState(unsigned char nBlock)
{
    unsigned char LSU = 0;
    unsigned char pData[8] = {0};

   //NWRITE 传输配置
   memset(pData,0xff,sizeof(SRIOLSUConfig));
    SRIO_DirectIO((unsigned int)pData, SRIOFpgaFlag[nBlock],  FPGA_DEVICE_ID_8BIT, 8, SRIO_Port0, LSU,
                  SRIO_NWrite, 0xa5a5);

     //NWRITE 传输配置
     memset(pData,0,sizeof(SRIOLSUConfig));
     SRIO_DirectIO((unsigned int)pData, SRIOFpgaFlag[nBlock],  FPGA_DEVICE_ID_8BIT, 8, SRIO_Port0, LSU,
                   SRIO_NWrite , 0xa5a5);

     return 0;
}


//NREAD 读取FPAG 缓冲区有效数据长度传输配置
int SRIO_ReadCamRamDataLen(unsigned char LSU, unsigned char nBlock)
{
    if (LSU >= 8)  LSU = 0;
    SRIO_DirectIO((unsigned int)&g_BlockLen[0], FPGA_BLOCK_LEN(nBlock),  FPGA_DEVICE_ID_8BIT, 8, SRIO_Port0, LSU,
                  SRIO_NRead , 0xa5a5);

    unsigned int nLen = 0;

   nLen = SRIO_SwapDWord(g_BlockLen[0]);

   nLen += 1;
   return (int)nLen * 8;
}

//NREAD读取VGA RAM状态
int SRIO_ReadVGARamState(unsigned char LSU)
{
    if (LSU >= 8)  LSU = 0;
    SRIO_DirectIO((unsigned int)&g_DnBlockState[0], VGA_BLOCK_STATE,  FPGA_DEVICE_ID_8BIT, 8, SRIO_Port0, LSU,
                  SRIO_NRead , 0xa5a5);

    //检查FPGA RAM块是否可用g_UpBlockIndex

   return SRIO_SwapDWord(g_DnBlockState[0]);
}
//#if 0
//NREAD读取VGA RAM状态
int SRIO_ReadRegister(unsigned char LSU, unsigned int nAddr,unsigned char *pData)
{
    if (LSU >= 8)  LSU = 0;
    return SRIO_DirectIO((Uint32)pData, nAddr,  FPGA_DEVICE_ID_8BIT,  8, SRIO_Port0, LSU,
                         SRIO_NRead , 0xa5a5);
}

int SRIO_WriteData(unsigned char uLSU, unsigned int uWrAddr, unsigned char *pData,int nLen)
{
    if (uLSU >= 8) uLSU = 0;
    return SRIO_DirectIO((Uint32)pData, uWrAddr,  FPGA_DEVICE_ID_8BIT, nLen, SRIO_Port0, uLSU,
                  SRIO_StreamWrite, 0xa5a5);
}


int SRIO_ReadData(unsigned char uLSU, unsigned int nRdAddr, unsigned char *pData,int nLen)
{
       //检查FPGA RAM块是否可用g_UpBlockIndex
    if (uLSU >= 8) uLSU = 0;
    return SRIO_DirectIO((Uint32)pData, nRdAddr,  FPGA_DEVICE_ID_8BIT, nLen, SRIO_Port0, uLSU,
                             SRIO_NRead, 0xa5a5);
}
//#endif

//发送数据到ＦＰＧＡ
//数据包ＮＷＲＩＴＥ
//尾包ＮＷＲＩＴＥ＿Ｒ
//使用ＬＳＵ　＝　１
int SRIO_SendDataToFpga(unsigned char LSU, unsigned char *pData, int nLen, int nCheckRamState)
{
    if (LSU >= 8 ) LSU = 0;

    if(nCheckRamState)
    {
        while(SRIO_ReadVGARamState(LSU) != 1)
       {
          // printf("VGA RAM W IDLE = 0\r\n");
       };

    }

    SRIO_DirectIO((Uint32)pData, SRIOFpgaBuf[g_DnBlockIndex],  FPGA_DEVICE_ID_8BIT, nLen - 256, SRIO_Port0, LSU,
                  SRIO_NWrite , 0xa5a5);


   SRIO_DirectIO((Uint32)&pData[nLen - 256], SRIOFpgaBuf[g_DnBlockIndex]+nLen - 256,  FPGA_DEVICE_ID_8BIT, 256, SRIO_Port0, LSU,
                 SRIO_NWriteResponse , 0xa5a5);

   g_DnBlockIndex++;
   g_DnBlockIndex %= MAX_RAM_COUNT;

   return nLen;
}


int SRIO_ReadDataFromFpga(unsigned char LSU, unsigned char *pData, int nLen)
{
    unsigned int nRamLen = 0;
    if (LSU >= 8)  LSU = 0;

    //检查FPGA RAM可用标记及数据长度
    //nRamLen 低16位为Flag标记，高16位为
    nRamLen = SRIO_ReadCamRamState(g_UpBlockIndex);

    if(!(nRamLen&0x01))
    {
        nLen = 0;
        return 0;
    }

    nRamLen = ((nRamLen>>16) + 1)*8;

    if(nRamLen < nLen)
    {
        nLen = nRamLen;
    }

    if(nLen%256)
    {
        nLen = (nLen/256+1)*256;
    }

    SRIO_DirectIO((unsigned int)pData, SRIOFpgaBuf[g_UpBlockIndex],  FPGA_DEVICE_ID_8BIT, nLen, SRIO_Port0, LSU,
                  SRIO_NRead , 0xa5a5);

#if 0
//清状态
   SRIO_ClearCamRamState(g_UpBlockIndex);

   SRIO_ReadRegister(0xf03020,Buf);
   printf("0xf03020 = %x %x %x %x \r\n",Buf[0],Buf[1],Buf[2],Buf[3]);

   SRIO_ReadRegister(0xf03028,Buf);
   printf("0xf03028 = %x %x %x %x \r\n",Buf[0],Buf[1],Buf[2],Buf[3]);
#endif
   g_UpBlockIndex++;  //回环模式下不累加
   g_UpBlockIndex %= MAX_RAM_COUNT;


   return nRamLen;
}


//数据校验
int SRioVerifyData(unsigned int *pSrcBuf, unsigned int *pDstBuf, int nLen)
{
    int i = 0,result = 1;

    for(i = 0; i < nLen ; i++)
    {
        if(pSrcBuf[i]  != pDstBuf[i])
        {
            result = 0;
            break;
        }
    }

    return result;

}




/*
 * SRIO 读寄存器接口，默认LSU = 0，size = 8
 */
int SRIO_ReadRegister32(unsigned int nRegAddr, unsigned char *pData)
{
   unsigned char LSU =0;

   return SRIO_DirectIO((unsigned int)pData, nRegAddr,  FPGA_DEVICE_ID_8BIT, 8, SRIO_Port0, LSU,
                 SRIO_NRead , 0xa5a5);
}


int SRIO_WriteRegister32(unsigned int nRegAddr, unsigned char *pData)
{
    unsigned char LSU =0;
    return SRIO_DirectIO((Uint32)pData, nRegAddr,  FPGA_DEVICE_ID_8BIT,  8, SRIO_Port0, LSU,
                    SRIO_NWrite , 0xa5a5);
}
unsigned int g_Bytes = 0;


//SRIO外部回环测试，需FPGA 回环版本配合使用
void SRioLoopTest(void)
{
    unsigned int *pTmp = (unsigned int *)SRIOSrcBuf;
    int i = 0, nLen  = 0;

    //SRIOInit();
    for(i = 0; i < BUFFER_SIZE/4; i++)
    {
        pTmp[i] = i;
    }
    while(1)
    {
        SRIO_SendDataToFpga(1,SRIOSrcBuf,BUFFER_SIZE,1);

        memset(SRIODstBuf,0,BUFFER_SIZE);
        nLen =  SRIO_ReadDataFromFpga(0,SRIODstBuf,BUFFER_SIZE);
        if(nLen != BUFFER_SIZE)
        {
            g_Bytes += nLen;
        }
        else
        {
            g_Bytes += BUFFER_SIZE;
        }


#if 1
       if(!SRioVerifyData((unsigned int *)SRIODstBuf,(unsigned int *)SRIOSrcBuf,BUFFER_SIZE/4))
       {
           printf("SRIO Data Verify .........Fail.\r\n");
       }
       else
       {
        //   printf("SRIO Data Verify .........Success.\r\n");
       }
#endif

    }

}


#define BLOCK_RAM_SIZE      163840
#define CHAR_RAM_SIZE       40960
#define SEPR

int SRIO_BlockRamTest(void)
{
    unsigned char rdBuf[BLOCK_RAM_SIZE] = {0};

    unsigned int nRdAddr = 0;
    unsigned int nRunCnt = 12,i =0,nTrans = 0;

    unsigned int  nDataLen = 0;
/*
    memset(wrBuf,0x5a,512);
    memset(rdBuf,0,512);

    SRIO_WriteData(0,nWrAddr,wrBuf,256);

    SRIO_ReadData(0,nRdAddr,rdBuf,256);

*/

   for( i = 0; i < nRunCnt; i++)
    {

#ifdef SEPR
        nRdAddr = 0;
        nDataLen = BLOCK_RAM_SIZE;
        memset(rdBuf,0,BLOCK_RAM_SIZE);
        for(nTrans  = 0; nTrans < BLOCK_RAM_SIZE/nDataLen; nTrans++)
        {
            //memset(wrBuf,i+1,512);
            //SRIO_WriteData(0,nWrAddr + i*BLOCK_RAM_SIZE/8 ,wrBuf,256)
            SRIO_ReadData(0,nRdAddr,rdBuf+nRdAddr,nDataLen);
            nRdAddr += nDataLen;

        }


        nRdAddr = BLOCK_RAM_SIZE;
        nDataLen = BLOCK_RAM_SIZE;
         memset(rdBuf,0x11,BLOCK_RAM_SIZE);
         for(nTrans  = 0; nTrans < BLOCK_RAM_SIZE/nDataLen; nTrans++)
         {
             //memset(wrBuf,i+1,512);
             //SRIO_WriteData(0,nWrAddr + i*BLOCK_RAM_SIZE/8 ,wrBuf,256)
             SRIO_ReadData(0,nRdAddr,rdBuf+nRdAddr-BLOCK_RAM_SIZE,nDataLen);
             nRdAddr += nDataLen;

         }

#else

         nWdataLen = 80;
         for(i = 0 ; i < CHAR_RAM_SIZE/nWdataLen; i++)
         {
             memset(wrBuf,i+1,512);
             SRIO_WriteData(0,nWrAddr + i*nWdataLen ,wrBuf,nWdataLen);
         }


         nRdAddr = 0;
         nDataLen = 512;
         memset(rdBuf,0,BLOCK_RAM_SIZE);
         SRIO_ReadData(0,nRdAddr,rdBuf,BLOCK_RAM_SIZE);

         nRdAddr = BLOCK_RAM_SIZE;
         nDataLen = 512;
         memset(rdBuf,0x11,BLOCK_RAM_SIZE);
         SRIO_ReadData(0,nRdAddr,rdBuf,BLOCK_RAM_SIZE);

#endif
    }
   return 0;
}

//粗略延时
void  delay_ms(unsigned ms)
{
        volatile unsigned long long startTSC, currentTSC;
        unsigned long long delayCycles;
        unsigned tscl, tsch;

        unsigned int dspCoreSpeed_HZ= 1000000000;    //DSP core clock speed in Hz
        TSCL = 0;   /* Enable the TSC */
        tscl= TSCL;
        tsch= TSCH;
        startTSC= _itoll(tsch,tscl);

        delayCycles= ((unsigned long long)ms*dspCoreSpeed_HZ/1000);

        do
        {
            tscl= TSCL;
            tsch= TSCH;
            currentTSC= _itoll(tsch,tscl);
        }
        while((currentTSC-startTSC)<delayCycles);
}
void  delay_us(unsigned us)
{
    volatile unsigned long long startTSC, currentTSC;
    unsigned long long delayCycles;
    Uint32 tscl, tsch;
    unsigned int dspCoreSpeed_HZ= 1000000000;    //DSP core clock speed in Hz

    TSCL = 0;   /* Enable the TSC */
    tscl= TSCL;
    tsch= TSCH;
    startTSC= _itoll(tsch,tscl);

    delayCycles= ((unsigned long long)us*dspCoreSpeed_HZ/1000000);

    do
    {
        tscl= TSCL;
        tsch= TSCH;
        currentTSC= _itoll(tsch,tscl);
    }
    while((currentTSC-startTSC)<delayCycles);
}

//1848文档显示, 维护ackid需要链路中 IDLE2 在用.
void SRIO_MatchAckID(unsigned uiLocalPort,  unsigned uiDestID,  unsigned uiRemotePort)//Make the ACK_ID of both sides match
{
    if (uiLocalPort >= 4 ||  uiRemotePort >= 4 )   return ;
    if(0 == HWREG(SOC_SRIO_0_REGS + SRIO_BLK_EN(uiLocalPort + 5)) )  //未使能的
        return;


    unsigned uiMaintenanceValue = 0, uiResult = 0;
    unsigned uiLocal_In_ACK_ID = 0, uiRemote_In_ACK_ID = 0, uiRemote_out_ACK_ID = 0;

        //send a "restart-from-error" command, request the ACK_ID of the other side
        HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_REQ(uiLocalPort))  = 4;

        //wait for link response
        while(0==(  HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(uiLocalPort)) >> (31) ))
            ;

        uiRemote_In_ACK_ID= ( (HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(uiLocalPort)))  & (0x000007E0u) )>>  (5);

        //设置 local OUTBOUND_ACKID = uiRemote_In_ACK_ID
        HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ACKID_STAT(uiLocalPort)) = uiRemote_In_ACK_ID;
        if(uiRemote_In_ACK_ID != 0)
            printf("match_ACK_ID SP_ACKID_STAT=0x%x\n", HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ACKID_STAT(uiLocalPort)) );     //for dubug

        do
        {
            //设置远端OUTBOUND_ACKID =  local INBOUND_ACKID
            uiLocal_In_ACK_ID= ( HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ACKID_STAT(uiLocalPort)) & 0x3F000000u)>>  24;

            uiMaintenanceValue= ((uiRemote_In_ACK_ID+1)<< 24)|uiLocal_In_ACK_ID;

            //set the remote ACK_ID through maintenance packet
            uiResult= SRIO_Maintenance(uiLocalPort, uiLocalPort, uiDestID,
                0x148+(0x20*uiRemotePort), GLOBAL_ADDR(&uiMaintenanceValue),
                SRIO_MaintenanceWrite);

            if(uiResult)    //fail
                continue;

            //readback the remote ID
            uiResult= SRIO_Maintenance(uiLocalPort, uiLocalPort,
                uiDestID, 0x148+(0x20*uiRemotePort), GLOBAL_ADDR(&uiMaintenanceValue),
                SRIO_MaintenanceRead);
            uiRemote_out_ACK_ID= uiMaintenanceValue& 0x0000003Fu;
        }while(uiResult|(uiLocal_In_ACK_ID+1 != uiRemote_out_ACK_ID));
}

int   SRIO_Maintenance(unsigned uiPort, unsigned char uiLSU_No,  unsigned uiDestID, unsigned uiOffSet, unsigned uiLocalAddress,  unsigned char  uiPacketType)
{
       SRIOLSUConfig cfg;
    #ifdef _LITTLE_ENDIAN
        Uint32 * uipData = (Uint32 *)uiLocalAddress;

        //swap maintenance value for little endian
        *uipData= _swap4(_packlh2(*uipData, *uipData));
    #endif

        SRIO_InitLsuCfg(&cfg, uiLocalAddress, uiOffSet, uiPacketType, 4,  uiDestID, uiPort);

        unsigned int  retryCount = 0xFFFF; //维护包不检查LSU
        SRIODirectIOTransfer(uiLSU_No, &cfg);

        while( COMPLETED_NOERRORS != SRIOLSUStatusGet(uiLSU_No, &cfg))
        {
            retryCount--;
            if(!retryCount)
            {
                return -1;
            }
        }
    #ifdef _LITTLE_ENDIAN
        //swap maintenance value for little endian
        *uipData= _swap4(_packlh2(*uipData, *uipData));
    #endif

        return 0;
}


//dspAddress为本地DSP的内存地址,  LSB为remote传输目标内存地址低32bit
void SRIO_InitLsuCfg(SRIOLSUConfig*pCfg, Uint32 uDspAddress, Uint32 uLSB, unsigned char cPacketType,Uint32 uBytesCount,  Uint16 usDstID, Uint8 cPort)
{
    memset(pCfg, 0, sizeof (SRIOLSUConfig));

    pCfg->Address.DSPAddress = uDspAddress ;
    pCfg->Address.RapidIOAddress.LSB = uLSB;
    pCfg->PacketType = cPacketType;
    pCfg->ByteCount = uBytesCount; //LSU传输多少个byte
    pCfg->ID.Dest = usDstID;
    pCfg->ID.Port = cPort;
    pCfg->ID.Size = SRIO_ID_8Bit;
    if (SRIO_DoorBell == cPacketType)
        pCfg->HopCount = 0;
    else if (SRIO_MaintenanceRead <= cPacketType && cPacketType <= SRIO_MaintenancePortWrite)
        pCfg->HopCount = 1;
    else
        pCfg->HopCount =0xff;//维护包设置为1, doorbell包设置为0
}

/*包类型为Doorbell和Doorbell使能的区别:
 * PacketType  != SRIO_DoorBell时,  使能为
 * 0 --> 包后面无doorbell info被发送
 * 1 --> doorbell info有效
 * 如果无需响应, 发送最后一个片段后发送一个doorbell
 * 如果需要响应, 所有响应都接收到后, 若响应都无错, 生成一个带doorbell info的doorbell
 * 如果需要响应, 如果收到的响应时出错, 则不会生成doorbell
*/
SRIORWStatus SRIO_SendDoorBell(Uint8 cPort, Uint8 cLSU,  Uint16 usDestID, Uint16 usDoorBellInfo)
{
    SRIOLSUConfig cfg;
    SRIO_InitLsuCfg(&cfg, 0, 0, SRIO_DoorBell, 8,  usDestID, cPort);

    //cfg.ID.Src = 1;
    cfg.IntReq = TRUE;
    cfg.DoorBell.Enable  = FALSE;
    cfg.DoorBell.Info      = usDoorBellInfo;

     Uint32 uRetryCount=0xFFFFF;
     SRIODirectIOTransfer(cLSU, &cfg);

     while( COMPLETED_NOERRORS  != SRIOLSUStatusGet(cLSU, &cfg) )//得到完成码（调试用）
     {
         uRetryCount--;
         if(!uRetryCount)
         {
             return COMPLETED_NOERRORS;
         }
     }
     return COMPLETED_NOERRORS;
}

SRIORWStatus SRIO_DirectIO(Uint32 uiLocalAddress, Uint32 uiRemoteAddress,  Uint32 uiDestID, Uint32 uiByteCount, Uint8 uiPort, Uint8 uiLSU_No,
    Uint8 packetType, Uint16 uDoorbellInfo)
{
    SRIOLSUConfig cfg;
//#ifdef _LITTLE_ENDIAN
//    Uint32 * uipData = (Uint32 *)uiLocalAddress;
//
//    //swap maintenance value for little endian
//    *uipData= _swap4(_packlh2(*uipData, *uipData));
//
//    Uint32 * uipData2 = (Uint32 *)uiRemoteAddress;
//
//        //swap maintenance value for little endian
//    *uipData2= _swap4(_packlh2(*uipData2, *uipData2));
//
//#endif
    SRIO_InitLsuCfg(&cfg, uiLocalAddress, uiRemoteAddress, packetType, uiByteCount,  uiDestID, uiPort);
    //cfg.ID.Src = 0;
    //cfg.Address.RapidIOAddress.MSB = 0;
    cfg.DoorBell.Info = uDoorbellInfo;
    //cfg.DoorBell.Enable = FALSE;
    //cfg.Priority = 0;
    //cfg.SupGoodInt = 0;
    //cfg.IntReq = 0;
    //cfg.HopCount = 0; //类型为8的维护包

        unsigned int  uRetryCount = 0xFFFF;

       // ////////////////////
        unsigned char cStatus = SRIOLSUFullAndBusyCheck(uiLSU_No);
        while(cStatus != 0)
        {
            cStatus = SRIOLSUFullAndBusyCheck(uiLSU_No);
            uRetryCount--;
            if (!uRetryCount)
            {
                if (cStatus & 0x0F)
                    return TIMEOUT_LSU_BUSY;
                else
                    return TIMEOUT_LSU_FULL;
            }
        }
        ///////////////////

       SRIODirectIOTransfer(uiLSU_No, &cfg);

        uRetryCount=0xFFFFF;
        while( COMPLETED_NOERRORS != SRIOLSUStatusGet(uiLSU_No, &cfg))
        {
            uRetryCount--;
            if(!uRetryCount)
            {
                return TIMEOUT;//超时
            }
        }
    return COMPLETED_NOERRORS;
}

//TI建议在初始化之后、尝试发送任何数据包之前添加以下步骤。无论设备当前是处于停止输出还是停止输入错误状态，都可以添加此步骤。
//立即从输入和输出错误停止条件恢复链路的两端....
//调用后会导致端口 received an unexpected packet or control symbol.
void SRIO_PreTransferSet(Uint8 uiPort)
{
    Uint32 uiStatus =  HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ERR_STAT(uiPort));
    if ((  uiStatus & 0x00010000) || (uiStatus & 0x00000100))
        HWREG(SOC_SRIO_0_REGS + SRIO_PLM_SP_LONG_CS_TX1(uiPort)) =  0x40FC8000;//0x2003F044 ;//如果您有对齐问题，并且DSP在RIO_SP_ERR_STAT寄存器中报告了错误，
    //清除错误
    HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ERR_DET(uiPort)) = 0;
    HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ERR_STAT(uiPort) ) =  0xFFFFFFFF;

    //测试功能:

    //HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_REQ(uiPort))  = 4;
           //wait for link response
       //    while(0==(  HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(uiPort)) >> (31) ))
               ;


    HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_REQ(uiPort))  = 3;
       //wait for link response
       while(0==(  HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(uiPort)) >> (31) ))
           ;
       unsigned uiRemote_In_ACK_ID= ( (HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(uiPort)))  & (0x000007E0u) )>>  (5);

       return;
}
Bool SRIO_LinkStatusCheck(Uint8 cPort)
{
    Bool bRet = 1;

    Uint32 uStatus =  (HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(cPort)) & 0x1F);
    if (0x10 == uStatus   ) //OK
    {
        //printf("++port %d Link-status field from the link-response control symbol:  OK\n", uiPort) ;
            bRet =  0;
        }
        else if (2 == uStatus ) //Error
        {
            printf("++port %d Link-status field from the link-response control symbol:  error have occurred\n", cPort);
            bRet = 1;
        }
        else  if (4 == uStatus  )
        {
             printf("++port %d Link-status field from the link-response control symbol:  Retry-stopped\n", cPort);
             //解决方式是A发送restart-from-retry给设备B
             bRet = 1;
        }
        else if (0x05 == uStatus  )
        {
            printf("++port %d Link-status field from the link-response control symbol:  Error-stopped\n", cPort);
       // //解决方式是设备A发送link-request给设备B
       // HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_REQ(uiPort))  = 4;
       // //wait for link response
       // while(0==(  HWREG(SOC_SRIO_0_REGS + SRIO_SPn_LM_RESP(uiPort)) >> (31) ))
       //     ;
            bRet = 1;
        }
        else
        {
            bRet = 1;
            printf("++port %d Link-status field from the link-response control symbol:  ??\n", cPort);
        }
    return bRet;
}


void   SRIOClearAllIntcStatus()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 16; j++)
            SRIODoorBellInterruptClear(i, j);
    }
    for(j = 0; j < 16; j++)
    {
        SRIOLSUPendingInterruptClear(0, j);
        SRIOLSUPendingInterruptClear(1, j);
    }
    SRIOErrRstEventInterruptClear();
}

#define TSC_getDelay(startTSC)  ((unsigned int)((0xFFFFFFFFl+TSCL)- (unsigned long long)startTSC)+ 1)
#define TSC_count_cycle_from(startTSC)  (TSC_getDelay(startTSC)- 50)

extern Int32 KeyStone_disable_PSC_module (Uint32 pwrDmnNum, Uint32 moduleNum);
extern Int32 KeyStone_disable_PSC_Power_Domain (Uint32 pwrDmnNum);

void  SRIOSoftReset()
{
    if ((CSL_PSC_getPowerDomainState(CSL_PSC_PD_SRIO) == PSC_PDSTATE_ON) &&
            (CSL_PSC_getModuleState (CSL_PSC_LPSC_SRIO) == PSC_MODSTATE_ENABLE))
        {
        //soft reset SRIO if it is already enabled
            if(HWREG(SOC_SRIO_0_REGS + SRIO_PeripheralControl)& 0x00000004u)//CSL_SRIO_RIO_PCR_PEREN_MASK
            {
                    int  i = 0; int j = 0, k = 0;
                    int  iMaxLsuNum = 8;
                    int  iMaxDeviceIDNum = 16;


                    for (i = 0; i < iMaxLsuNum; i++)
                    {
                        for (j = 0; j < iMaxDeviceIDNum;  j++)
                        {
                            HWREG(SOC_SRIO_0_REGS + SRIO_LSU_REG6(i))  = 1 | (j << 2);
                            /*This can take more than one cycle to do the flush.    wait for a while*/
                            for(k = 0; k < 100; k++);
                        }
                    }
                    SRIO_PreTransferSet(2);
                    k = 0;
                    SRIOPeripheralDisable();//disable the PEREN bit of the PCR register to stop all new logical layer transactions
                    delay_ms(1000); //Wait one second to finish any current DMA transfer
                    ////reset all logic blocks in SRIO
                    for (i = 0; i < 9; i++)
                    {
                        SRIOBlockDisable(i);
                    }
                    SRIOGlobalDisable();
                    //disable Serdes
                    {
                        //
                        KickUnlock();
                        SRIOSerDesPLLSet(0);//disable  PLL
                        ////disable TX/RX links  ??
                        for (i = 0; i < 4;  i++)
                        {
                            if (1 == i ) continue;
                            SRIOSerDesTxSet(i, 0);
                            SRIOSerDesRxSet(i, 0);
                        }
                    }
                    //disable SRIO through PSC
                    KeyStone_disable_PSC_module(CSL_PSC_PD_SRIO, CSL_PSC_LPSC_SRIO);
                    KeyStone_disable_PSC_Power_Domain(CSL_PSC_PD_SRIO);


            }
        }

}


void  SRIO_InitErrManageRegs(void)
{

        HWREG(SOC_SRIO_0_REGS + SRIO_ERR_RST_EVNT_ICCR) |= 0x10F07;
        //HWREG(SOC_SRIO_0_REGS + SRIO_EM_MECS_STAT)        |= 0x00FF;

        HWREG(SOC_SRIO_0_REGS + SRIO_ERR_RPT_BH)  = 0x30000007;//next ext=0x0000(last)
        HWREG(SOC_SRIO_0_REGS + SRIO_ERR_DET)        &= 0x243FFF3F;  //clear
        HWREG(SOC_SRIO_0_REGS + SRIO_H_ADDR_CAPT)= 0;//clear
        HWREG(SOC_SRIO_0_REGS + SRIO_ADDR_CAPT)    &= 0x4;
        HWREG(SOC_SRIO_0_REGS + SRIO_ID_CAPT)          = 0;//clear
        HWREG(SOC_SRIO_0_REGS + SRIO_CTRL_CAPT)      = 0;
        int i = 0;
        for (;  i<4;  i++)
        {
            if (i == 1) continue;
            HWREG(SOC_SRIO_0_REGS + SRIO_PortControl(i))  &=  ~(1 << 20);
            HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ERR_ATTR_CAPT(i)) &= 0x2000000E;
//            HWREG(SOC_SRIO_0_REGS + SRIO_PW_RX_CAPTn(i)) = 0;//clear
            HWREG(SOC_SRIO_0_REGS + SRIO_SPn_ERR_DET(i))  &= 0x7F81BFC8;
            HWREG(SOC_SRIO_0_REGS + SRIO_PortErrorStatus(i))  = 0xFFFFFFF; //写1清除
            //SP0_CTL, SP1_CTL, SP2_CTL,  SP3_CTL,
            //HWREG(SOC_SRIO_0_REGS + SRIO_PortControl(i)) &= (~((1 << 3) | (1 <<2)));//当错误达到门限后,端口会继续传输报文（这个取决于Port Control CSR里面的Stop on和Drop Packet两个位域）
        }
}






















