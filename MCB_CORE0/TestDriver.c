/*
 * TestDriver.c
 *
 *  Created on: 2020-9-1
 *      Author: linxi
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "TestDriver.h"


unsigned int SwapDWordByte(unsigned int nData)
{
	return (((nData & 0xff000000) >> 24) | ((nData & 0x00ff0000) >> 8) | ((nData & 0x0000ff00) << 8) | ((nData & 0x000000ff) << 24));
}


void Init_EDAM3_Param(unsigned int nLen, pEDMA3CCParam_st pParam)
{
	unsigned int nMaxACnt = 16 * 1024;
	unsigned int acnt = 0, bcnt = 1, ccnt = 1;
	unsigned int syncType = EDMA3_SYNCTYPE_A;
	memset(pParam, 0, sizeof(EDMA3CCParam_st));

	if (nLen > nMaxACnt)
	{
		acnt = nMaxACnt;
		bcnt = nLen / nMaxACnt;
		ccnt = 1;
		syncType = EDMA3_SYNCTYPE_AB;
	}
	else
	{
		acnt = nLen;
		bcnt = ccnt = 1;
	}

	pParam->aCnt = (unsigned short)acnt;
	pParam->bCnt = (unsigned short)bcnt;
	pParam->cCnt = (unsigned short)ccnt;

	// 设置 SRC / DES 索引
	pParam->srcBIdx = (short)acnt;
	pParam->destBIdx = (short)acnt;

	if (syncType == EDMA3_SYNCTYPE_A)
	{
		// A Sync 传输模式
		pParam->srcCIdx = (short)acnt;
		pParam->destCIdx = (short)acnt;
	}
	else
	{
		pParam->srcCIdx = ((short)acnt * (short)bcnt);
		pParam->destCIdx = ((short)acnt * (short)bcnt);
	}

	pParam->bCntReload = (unsigned short)0;

	// 编程 TCC
	pParam->opt.tcc = 0;//暂不赋值

	// 使能 Intermediate & Final 传输完成中断
	pParam->opt.tcintEn = 1;

	if (syncType == EDMA3_SYNCTYPE_A)
	{
		pParam->opt.syncDim = 0;
	}
	else
	{
		pParam->opt.syncDim = 1;
	}
}

extern void  TestADDA_Nread( );
//SRIO中断
#ifdef  __cplusplus
#pragma CODE_SECTION("AuroraCode")
#else
#pragma  CODE_SECTION(test_isr_Doorbell,  "AuroraCode")
#endif
static   void test_isr_Doorbell(void* hand)
{
	unsigned int nStatus = SRIOGetDoorbellStatus();
	switch (nStatus)
	{
		case 1://版本信息
			SRIOClearDoorbellIntcStatus(0);
			break;
		case 2://同步422
			SRIOClearDoorbellIntcStatus(1);
			break;
		case 4://硬件同步
			SRIOClearDoorbellIntcStatus(2);
			break;
		case 8://异步422
			SRIOClearDoorbellIntcStatus(3);
			break;
		case 16://Can0
			SRIOClearDoorbellIntcStatus(4);
			break;
		case 32://Can1
			SRIOClearDoorbellIntcStatus(5);
			break;
		case 64://图像传输
			SRIOClearDoorbellIntcStatus(6);
			break;
		default:
			break;
	}
	//char* pPrintMsg = "Doorbell ... \r\n";
	//    Uart_Printf(pPrintMsg, strlen(pPrintMsg));




    //先从AD 读数,  再写DA
	//if (uIntercCount % 256 ==0)
	//    printf("Doorbell :%d\n", uIntercCount);
	//TestADDA();
	TestADDA_Nread();
//	TestVMERdWr();

}


//EDMA3完成中断
volatile unsigned int blComp0Code = 0;
volatile unsigned int blComp1Code = 0;
volatile unsigned int blComp2Code = 0;
volatile unsigned int blComp3Code = 0;
unsigned int nCh0Serial = 0;
unsigned int nCh1Serial = 0;
static void test_EDMA3_Finish_ISR(void* hand)
{
	unsigned int nCompCode = EDMA3_GetIntrStatus();
	switch (nCompCode)
	{
		case 1:
			blComp0Code = 1;
			nCh0Serial++;
			EDMA3_ClrIntr(0);
			break;
		case 2:
			blComp1Code = 1;
			nCh1Serial++;
			EDMA3_ClrIntr(1);
			break;
		case 4:
			blComp2Code = 1;
			EDMA3_ClrIntr(2);
			break;
		case 8:
			blComp3Code = 1;
			EDMA3_ClrIntr(3);
			break;
		default:
			break;
	}
	INTC_CIC_ClearSysInter(0, 38, 2);
}


//GPIO中断
unsigned int nGPIO = 0;
static void test_GPIO_ISR(void* hand)
{
	char buf[128] = { 0 };
	sprintf(buf, "GPIO = %u\n", nGPIO++);
	Uart_Printf(buf, strlen(buf));
}


//IPC中断
volatile unsigned char blFlag = 0;
void Test_Core_IPC(void* hand)
{
	unsigned int nRead = 0;
	IPC_GetCoreSoureInfo(0, &nRead);
	Invalidate_Cache(CACHE_L1D, (void*)(0x0C0F1000), 1, 1);
	unsigned char nData = *(unsigned char*)0x0C0F1000;
	if ((nRead == 0x80) && (nData == 8))
	{
		blFlag = 1;
	}
	else
	{
		blFlag = 0;
	}
	IPC_ClearSourceInfo(0);
}



//Timer中断
unsigned int nTimeSerial[2] = { 0 };
static void test_isr_Timer(void* handle)
{
	nTimeSerial[0]++;
	if (nTimeSerial[0] % 2 == 0)
	{
		GPIO_PinWrite(14, GPIO_PIN_HIGH);
	}
	else
	{
		GPIO_PinWrite(14, GPIO_PIN_LOW);
	}
#if 0
	char buf[128] = { 0 };
	sprintf(buf, "Timer1 = %u\n", nTimeSerial[0]++);
	Uart_Printf(buf, strlen(buf));

	if (nTimeSerial[0] > 20)
	{
		Timer_StopPeriod(0);
	}
#endif
}

static void test_isr_Timer2(void* handle)
{
	nTimeSerial[1]++;
	if (nTimeSerial[1] > 5000)
	{
		nTimeSerial[1] = 0;
	}
}

unsigned char blUartRxSerial = 0;
//Uart接收中断
static void Test_Uart_Rx_ISR(void* hand)
{
	int nStatus = Uart_GetIntStatus();
	int s=1;
	switch(nStatus)
	{
		case 1:
			s++;
			break;
		case 2:
			blUartRxSerial = 1;
			break;
		case 3:
			s++;
			break;
		case 6:
			s++;
			break;
		default:
			break;

	}
	INTC_CIC_ClearSysInter(0, 148, 42);
}



void Global_Default_Init()
{
	//配置DSP Cache 大小
	CfgCache_L1_L2_Size(ENUM_L1_SIZE_32_KB, ENUM_L1_SIZE_32_KB, ENUM_L2_SIZE_0_KB);

	//初始化主频为1GHz
	Init_PLL(39, 1);

	//配置所有模块电源域，emif、spi、srio时钟域
	PSCModuleControl(HW_PSC_EMIF16AndSPI, PSC_MDCTL_NEXT_ENABLE, PSC_POWERDOMAIN_ALWAYS_ON, PSC_PDCTL_NEXT_ON);
	PSCModuleControl(HW_PSC_SRIO, PSC_MDCTL_NEXT_ENABLE, PSC_POWERDOMAIN_SRIO, PSC_PDCTL_NEXT_ON);
	PSCModuleControl(HW_PSC_PACKET, PSC_MDCTL_NEXT_ENABLE, PSC_POWERDOMAIN_PACKET,PSC_PDCTL_NEXT_ON);
	PSCModuleControl(HW_PSC_SGMII, PSC_MDCTL_NEXT_ENABLE, PSC_POWERDOMAIN_PACKET,PSC_PDCTL_NEXT_ON);
	
	//配置PASS时钟为1050MHz
	Init_PASS_Pll(20, 0);

	//配置DDR时钟为1333MHz
	//Init_DDR_Pll(39, 2);

	//配置DDR3 1333MHz的时序
	//Init_DDR3_Timing(DDR3_TIMING_1333MHZ);

}


int Init_All_Intc()
{
	INTC_Init();
    INTC_CIC_Open(0);

	//SRIO中断
	if (0 != INTC_Open(20, (int)4, test_isr_Doorbell, NULL))
	{
		return -1;
	}

//	INTC_CIC_Init(0, 148, 42);

	return 0;
}




/**********************测试EDMA3 - Start*******************************************/

#define CamLink_SIZE	 32//640*512

volatile unsigned char*   EdmaBuff1= (unsigned char*)0x80100000;
volatile unsigned char*   EdmaBuff2=(unsigned char*)0x80300000;
volatile unsigned char*   EdmaBuff3=(unsigned char*)0x80500000;

volatile unsigned char*  pMSMC1=(unsigned char*)0x0C100000;
volatile unsigned char*  pMSMC2=(unsigned char*)0x0C200000;


void Test_EDMA3()
{
	char blStatus=1;
	char buf[128]={0};
    unsigned int i=0;
    int nIndex=10000;
    EDMA3CCParam_st paramSet;
	memset(&paramSet, 0, sizeof(EDMA3CCParam_st));

	Init_EDAM3_Param(CamLink_SIZE, &paramSet);
	paramSet.opt.tcc = 0;
	paramSet.srcAddr = (unsigned int)EdmaBuff1;
	paramSet.destAddr = (unsigned int)pMSMC1;
	paramSet.linkAddr = 0x4000 + (0x20 * 0);
	EDMA3_Init(0, 0, 0, 0, 0, paramSet);

	paramSet.opt.tcc = 1;
	paramSet.srcAddr = paramSet.destAddr;
	paramSet.destAddr = (unsigned int)pMSMC2;
	paramSet.linkAddr = 0x4000 + (0x20 * 1);
	EDMA3_Init(0, 1, 1, 1, 0, paramSet);

	paramSet.opt.tcc = 2;
	paramSet.srcAddr = paramSet.destAddr;
	paramSet.destAddr = (unsigned int)EdmaBuff2;
	paramSet.linkAddr = 0x4000 + (0x20 * 2);
	EDMA3_Init(0, 2, 2, 2, 0, paramSet);

	paramSet.opt.tcc = 3;
	paramSet.srcAddr = paramSet.destAddr;
	paramSet.destAddr = (unsigned int)EdmaBuff3;
	paramSet.linkAddr = 0x4000 + (0x20 * 3);
	EDMA3_Init(0, 3, 3, 3, 0, paramSet);

    while(nIndex)
    {
        for(i = 0; i < CamLink_SIZE; ++i)
        {
        	EdmaBuff1[i]= nIndex + i;
        }

        memset((void*)EdmaBuff2,0,CamLink_SIZE);
        memset((void*)EdmaBuff3,0,CamLink_SIZE);
        memset((void*)pMSMC1,0,CamLink_SIZE);
        memset((void*)pMSMC2,0,CamLink_SIZE);
    	blStatus = 1;
		//DDR - >MMSC
		Invalidate_Cache(CACHE_L1D, (void*)EdmaBuff1, CamLink_SIZE, 1);
		EDMA3_StartChTrans(0);
		//Writeback_Cache(CACHE_L1D, (void*)pMSMC1, CamLink_SIZE, 1);
		while(0 == blComp0Code){}
		blComp0Code=0;
		for(i = 0; i < CamLink_SIZE; ++i)
		{
			if(pMSMC1[i] != EdmaBuff1[i])
			{
				blStatus = 0;
				break;
			}
		}
		if(!blStatus)
		{
			sprintf(buf,"Test EDMA3 [DDR -> MMSC] Failed\r\n");
			Uart_Printf(buf,strlen(buf));
			for(i = 0; i < CamLink_SIZE; ++i)
			{
				sprintf(buf,"%d-%d ",EdmaBuff1[i],pMSMC1[i]);
				Uart_Printf(buf,strlen(buf));
			}
			break;
		}

		//MMSC - >MMSC
		Invalidate_Cache(CACHE_L1D, (void*)pMSMC1, CamLink_SIZE, 1);
		EDMA3_StartChTrans(1);
		//Writeback_Cache(CACHE_L1D, (void*)pMSMC2, CamLink_SIZE, 1);
		while(0 == blComp1Code){}
		blComp1Code=0;
		for(i = 0; i < CamLink_SIZE; ++i)
		{
			if(pMSMC2[i] != EdmaBuff1[i])
			{
				blStatus = 0;
				break;
			}
		}
		if(!blStatus)
		{
			sprintf(buf,"Test EDMA3 [MMSC -> MMSC] Failed\r\n");
			Uart_Printf(buf,strlen(buf));
			for(i = 0; i < CamLink_SIZE; ++i)
			{
				sprintf(buf,"%d-%d ",pMSMC1[i],pMSMC2[i]);
				Uart_Printf(buf,strlen(buf));
			}
			break;
		}

		//MMSC - >DDR
		Invalidate_Cache(CACHE_L1D, (void*)pMSMC2, CamLink_SIZE, 1);
		EDMA3_StartChTrans(2);
		//Writeback_Cache(CACHE_L1D, (void*)EdmaBuff2, CamLink_SIZE, 1);
		while(0 == blComp2Code){}
		blComp2Code=0;
		for(i = 0; i < CamLink_SIZE; ++i)
		{
			if(EdmaBuff2[i] != EdmaBuff1[i])
			{
				blStatus = 0;
				break;
			}
		}
		if(!blStatus)
		{
			sprintf(buf,"Test EDMA3 [MMSC -> DDR] Failed\r\n");
			Uart_Printf(buf,strlen(buf));
			for(i = 0; i < CamLink_SIZE; ++i)
			{
				sprintf(buf,"%d-%d ",pMSMC2[i],EdmaBuff2[i]);
				Uart_Printf(buf,strlen(buf));
			}
			break;
		}
		//DDR - >DDR
		Invalidate_Cache(CACHE_L1D, (void*)EdmaBuff2, CamLink_SIZE, 1);
		EDMA3_StartChTrans(3);
		//Writeback_Cache(CACHE_L1D, (void*)EdmaBuff3, CamLink_SIZE, 1);
		while(0 == blComp3Code){}
		blComp3Code=0;

		//数据比对
		for(i = 0; i < CamLink_SIZE; ++i)
		{
			if(EdmaBuff3[i] != EdmaBuff1[i])
			{
				blStatus = 0;
				break;
			}
		}
		nIndex--;
		if(blStatus)
		{
			sprintf(buf,"Test EDMA3 Trans OK, Index = %d\r\n",10000 - nIndex);
			Uart_Printf(buf,strlen(buf));
		}
		else
		{
			sprintf(buf,"Test EDMA3 Trans Failed, Index = %d\r\n",10000 - nIndex);
			Uart_Printf(buf,strlen(buf));
			for(i = 0; i < CamLink_SIZE; ++i)
			{
				sprintf(buf,"%d-%d ",EdmaBuff2[i],EdmaBuff3[i]);
				Uart_Printf(buf,strlen(buf));
			}
			break;
		}
    }
	EDMA3_UnInit(0, 0, 0);
	EDMA3_UnInit(1, 1, 0);
	EDMA3_UnInit(2, 2, 0);
	EDMA3_UnInit(3, 3, 0);
}

/**********************测试EDMA3 - End*******************************************/




/**********************测试Timer - Start*******************************************/



void Test_Timer()
{
	unsigned int time = 20/*ms*/ * 1000 * 1000 / 6;
	Timer_Config(8, 0, time);
	Timer_Start(8, TIMER_ENAMODE_CONT);
	time= 1000/*ms*/ * 1000 * 1000 / 6;
	Timer_Config(0, 0, time);
	Timer_Start(0, TIMER_ENAMODE_CONT);
}
/**********************测试Timer - End*******************************************/




/**********************测试SPIFlash - Start*******************************************/
unsigned char pFlshWrite[FLASH_BLOCK_SIZE];
unsigned char pFlshRead[FLASH_BLOCK_SIZE];
int CheckSPIDataValidity(unsigned char* WriteAddr, unsigned char* ReadAddr, int nLen)
{
	int i = 0;
	for (i = 0; i < nLen; ++i)
	{
		if (WriteAddr[i] != ReadAddr[i])
		{
			return -1;
		}
	}
	return 0;
}

void Test_SPI_Nor_Flash()
{
	Uart_Printf("Test SPI-Nor-Flash Statrt\r\n",strlen("Test SPI-Nor-Flash Statrt\r\n"));
	int i = 0;
	unsigned int j = 0;
	unsigned int pWAddr = 0;
	unsigned int nEraseHigh = 0, nEraseLow = 0;
	unsigned int nWriteHigh = 0, nWriteLow = 0;
	unsigned int nReadHigh = 0, nReadLow = 0;
	long double nEraseTime = 0, nWriteTime = 0, nReadTime = 0;
	char blSuccess = 0;
	char buff[128] = { 0 };

	
	spiInit(1);

	//先按照块的擦写测试，170块，共170*FLASH_BLOCK_SIZE字节
	for (i = 0; i < 170; ++i)
	{
		for (j = 0; j < FLASH_BLOCK_SIZE; ++j)
		{
			pFlshWrite[j] = j + i;
		}
		memset(pFlshRead, 0, FLASH_BLOCK_SIZE);

		if (i % 30 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashErase(pWAddr, FLASH_BLOCK_SIZE,1);//每次擦除1块
		if (i % 30 == 0)
		{
			Timer_GetCount(8, &nEraseHigh, &nEraseLow);
			Timer_Close(8);
		}
		
		if (i % 30 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}	
		spiFlashWrite(pWAddr, pFlshWrite, FLASH_BLOCK_SIZE);//每次写入一块
		if (i % 30 == 0)
		{
			Timer_GetCount(8, &nWriteHigh, &nWriteLow);
			Timer_Close(8);
		}

		if (i % 30 == 0)
		{

			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashRead(pWAddr, pFlshRead, FLASH_BLOCK_SIZE);//每次读出一块
		if (i % 30 == 0)
		{

			Timer_GetCount(8, &nReadHigh, &nReadLow);
			Timer_Close(8);
		}

		if (CheckSPIDataValidity(pFlshWrite,pFlshRead, FLASH_BLOCK_SIZE) != 0)//数据比对
		{
			if (i % 30 == 0)
			{
				sprintf(buff, "SPI-Flash Data Comparison Failed ; WriteAddr = %X\r\n", FLASH_BLOCK_SIZE*i);
				Uart_Printf(buff, strlen(buff));
			}
			blSuccess = 0;
			break;
		}
		else
		{
			if (i % 30 == 0)
			{
				sprintf(buff, "SPI-Flash Data Comparison Success ; Block=%d,nEraseLow=%X,nWriteLow=%X,nReadLow=%X\r\n", i, nEraseLow, nWriteLow, nReadLow);
				nEraseTime += nEraseLow;
				nWriteTime += nWriteLow;
				nReadTime += nReadLow;
				Uart_Printf(buff, strlen(buff));
			}
			blSuccess = 1;
		}
		pWAddr += FLASH_BLOCK_SIZE;
	}

	if (blSuccess > 0)
	{
		Uart_Printf("Test SPI - Flash Block OK\r\n", strlen("Test SPI - Flash Block OK\r\n"));
		double time1 = (long double)nEraseTime / 1000 / 1000;//ms
		double time2 = (long double)nWriteTime / 1000 / 1000;
		double time3 = (long double)nReadTime / 1000 / 1000;
		sprintf(buff, "Erase a Block Time: %.2fms, Write a Block Time: %.2fms, Read a Block Time: %.2fms\r\n\r\n", time1, time2, time3);
		Uart_Printf(buff, strlen(buff));
	}
	else
	{
		Uart_Printf("Test SPI - Flash Block Failed\r\n", strlen("Test SPI - Flash Block Failed\r\n"));
		return;
	}

	nEraseTime = nWriteTime = nReadTime = 0;
	//按照SECTOR擦除读写,共170*FLASH_BLOCK_SIZE字节
	for (i = 0; i < 170 * FLASH_BLOCK_SIZE / FLASH_SECTOR_SIZE; i++)
	{
		for (j = 0; j < FLASH_SECTOR_SIZE; ++j)
		{
			pFlshWrite[j] = j + i;
		}
		memset(pFlshRead, 0, FLASH_BLOCK_SIZE);

		if (i % 500 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashErase(pWAddr, FLASH_SECTOR_SIZE,0);//每次擦除一扇形区域
		if (i % 500 == 0)
		{
			Timer_GetCount(8, &nEraseHigh, &nEraseLow);
			Timer_Close(8);
		}

		if (i % 500 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashWrite(pWAddr, pFlshWrite, FLASH_SECTOR_SIZE);//每次写入一扇形区域
		if (i % 500 == 0)
		{
			Timer_GetCount(8, &nWriteHigh, &nWriteLow);
			Timer_Close(8);
		}

		if (i % 500 == 0)
		{

			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashRead(pWAddr, pFlshRead, FLASH_SECTOR_SIZE);//每次读出一扇形区域
		if (i % 500 == 0)
		{

			Timer_GetCount(8, &nReadHigh, &nReadLow);
			Timer_Close(8);
		}

		if (CheckSPIDataValidity(pFlshWrite, pFlshRead, FLASH_SECTOR_SIZE) != 0)//数据比对
		{
			if (i % 500 == 0)
			{
				sprintf(buff, "SPI-Flash Data Comparison Failed ; WriteAddr = %X\r\n", pWAddr);
				Uart_Printf(buff, strlen(buff));
			}
			blSuccess = 0;
			break;
		}
		else
		{
			if (i % 500 == 0)
			{
				sprintf(buff, "SPI-Flash Data Comparison Success ; Sector=%d,nEraseLow=%X,nWriteLow=%X,nReadLow=%X\r\n", i, nEraseLow, nWriteLow, nReadLow);
				Uart_Printf(buff, strlen(buff));
				nEraseTime += nEraseLow;
				nWriteTime += nWriteLow;
				nReadTime += nReadLow;
			}
			blSuccess = 1;
		}
		pWAddr += FLASH_SECTOR_SIZE;
	}

	if (blSuccess > 0)
	{
		Uart_Printf("Test SPI - Flash Sector OK\r\n", strlen("Test SPI - Flash Sector OK\r\n"));
		double time1 = (long double)nEraseTime / 1000 / 1000;//ms
		double time2 = (long double)nWriteTime / 1000 / 1000;
		double time3 = (long double)nReadTime / 1000 / 1000;
		sprintf(buff, "Erase a Sector Time: %.2fms, Write a Sector Time: %.2fms, Read a Sector Time: %.2fms\r\n\r\n", time1, time2, time3);
		Uart_Printf(buff, strlen(buff));
	}
	else
	{
		Uart_Printf("Test SPI - Flash Sector Failed\r\n", strlen("Test SPI - Flash Sector Failed\r\n"));
		return;
	}


	nEraseTime = nWriteTime = nReadTime = 0;
	for (i = 0; i < 172; i++)
	{
		spiFlashErase(pWAddr + i*FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE, 1);//剩下的全部擦除
	}
	//按照Page擦除读写,共172*FLASH_BLOCK_SIZE字节
	for (i = 0; i < 172 * FLASH_BLOCK_SIZE / FLASH_PAGE_SIZE; i++)
	{
		for (j = 0; j < FLASH_PAGE_SIZE; ++j)
		{
			pFlshWrite[j] = j + i;
		}
		memset(pFlshRead, 0, FLASH_BLOCK_SIZE);

		if (i % 10000 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashWrite(pWAddr, pFlshWrite, FLASH_PAGE_SIZE);//每次写入一页
		if (i % 10000 == 0)
		{
			Timer_GetCount(8, &nWriteHigh, &nWriteLow);
			Timer_Close(8);
		}

		if (i % 10000 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		spiFlashRead(pWAddr, pFlshRead, FLASH_PAGE_SIZE);//每次读出一扇形区域
		if (i % 10000 == 0)
		{
			Timer_GetCount(8, &nReadHigh, &nReadLow);
			Timer_Close(8);
		}

		if (CheckSPIDataValidity(pFlshWrite, pFlshRead, FLASH_PAGE_SIZE) != 0)//数据比对
		{
			if (i % 10000 == 0)
			{
				sprintf(buff, "SPI-Flash Data Comparison Failed ; WriteAddr = %X\r\n", pWAddr);
				Uart_Printf(buff, strlen(buff));
			}
			blSuccess = 0;
			break;
		}
		else
		{
			if (i % 10000 == 0)
			{
				sprintf(buff, "SPI-Flash Data Comparison Success ; Sector=%d,nWriteLow=%X,nReadLow=%X\r\n", i,nWriteLow, nReadLow);
				Uart_Printf(buff, strlen(buff));
				nWriteTime += nWriteLow;
				nReadTime += nReadLow;
			}
			blSuccess = 1;
		}
		pWAddr += FLASH_PAGE_SIZE;
	}

	if (blSuccess > 0)
	{
		Uart_Printf("Test SPI - Flash Page OK\r\n", strlen("Test SPI - Flash Page OK\r\n"));
		double time2 = (long double)nWriteTime / 5 * 6 / 1000;
		double time3 = (long double)nReadTime / 5 * 6 / 1000;
		sprintf(buff, "Write a page Time: %.2fus, Read a page Time: %.2fus\r\n\r\n", time2, time3);
		Uart_Printf(buff, strlen(buff));
	}
	else
	{
		Uart_Printf("Test SPI - Flash Page Failed\r\n", strlen("Test SPI - Flash Page Failed\r\n"));
		return;
	}
	Uart_Printf("Test SPI-Nor-Flash finished\r\n",strlen("Test SPI-Nor-Flash finished\r\n"));
}
/**********************测试SPIFlash - End*******************************************/









/**********************测试Nand - Start*******************************************/

#define NAND_PAGE_SIZE_IN_BYTES                 (512)
#define NAND_BLOCK_SIZE_IN_BYTES                (16*1024)
#define NAND_NUMOF_BLK                          (4096)
#define NAND_PAGE_PER_BLOCK                     (32)



void Nand_EMIF_Confg_Param(NandInfo_t* pNandInfo)
{
	pNandInfo->eccType = NAND_ECC_ALGO_RS_4BIT;

	pNandInfo->busWidth = NAND_BUSWIDTH_8BIT;

	pNandInfo->hNandCtrlInfo.EMIF_Init = EMIF_Init;
	pNandInfo->hNandCtrlInfo.EMIF_WaitPinStatusGet = EMIF_WaitPinStatusGet;
	pNandInfo->hNandCtrlInfo.currChipSelect = EMIFA_CHIP_SELECT_0;
	pNandInfo->hNandCtrlInfo.waitPin = EMIFA_EMA_WAIT_PIN0;

	/* 设置异步等待时序 */
	int moduleClkInMHz = NAND_MODULE_CLK_IN_MHZ;
	pNandInfo->hNandCtrlInfo.nTiming = EMIFA_ASYNC_WAITTIME_CONFIG((((moduleClkInMHz * NAND_WRITE_SETUP_TIME_IN_NS) / 1000u) & EMIFA_WRITE_SETUP_RESETVAL),
		(((moduleClkInMHz * NAND_WRITE_STROBE_TIME_IN_NS) / 1000u) & EMIFA_WRITE_STROBE_RESETVAL),
		(((moduleClkInMHz * NAND_WRITE_HOLD_TIME_IN_NS) / 1000u) & EMIFA_WRITE_HOLD_RESETVAL),
		(((moduleClkInMHz * NAND_READ_SETUP_TIME_IN_NS) / 1000u) & EMIFA_READ_SETUP_RESETVAL),
		(((moduleClkInMHz * NAND_READ_STROBE_TIME_IN_NS) / 1000u) & EMIFA_READ_STROBE_RESETVAL),
		(((moduleClkInMHz * NAND_READ_HOLD_TIME_IN_NS) / 1000u) & EMIFA_READ_HOLD_RESETVAL),
		(((moduleClkInMHz * NAND_TURN_ARND_TIME_IN_NS) / 1000u) & EMIFA_TA_RESETVAL));


}

#if 1
static int NANDDataIntegrityCheck(unsigned char* pWrite,unsigned char* pRead)
{
	int byteCnt = 0;

    for(byteCnt = 0; byteCnt < NAND_PAGE_SIZE_IN_BYTES; byteCnt++)
    {
        if(pWrite[byteCnt] != pRead[byteCnt])
        {
			return -1;
        }
    }
    return 0;
}

unsigned char pFlashWrite[NAND_PAGE_SIZE_IN_BYTES] = { 0 };
unsigned char pFlashRead[NAND_PAGE_SIZE_IN_BYTES] = { 0 };

void Test_EMIF_Nand_Flash()
{

	NandInfo_t NandInfo;
	printf("Test Emif-Nand Start\r\n");
	Nand_EMIF_Confg_Param(&NandInfo);
	int serialE = 0, serialWR = 0;
	volatile unsigned int i = 0, j = 0, k = 0;
	unsigned char nMID, DID;
	unsigned int nReadHigh = 0, nReadLow = 0, nWriteHigh = 0, nWriteLow = 0;
	unsigned long long nEraseTime = 0, nWriteTime = 0, nReadTime = 0;
	NANDOpen(NandInfo);
	NANDReset();
	NANDReadId(&nMID, &DID);

#if 1
	for (i = 0; i < NAND_NUMOF_BLK; ++i)
	{
		if (NAND_BLOCK_BAD == NANDBadBlockCheck(i))
		{
			printf("Block %d damage\r\n", i);
			continue;//检测到坏块，就跳过该块
		}

		if (i % 800 == 0)
		{
			Timer_Config(9, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(9,TIMER_ENAMODE_ONCE);
		}
		if (NAND_STATUS_PASSED != NANDBlockErase(i))
		{
			printf("Erase Block %d Failed\r\n", i);
			continue;//擦除失败
		}
		if (i % 800 == 0)
		{
			Timer_GetCount(9, &nReadHigh, &nReadLow);
			Timer_Close(9);
			printf("Erase Block %d TimerH=%X,TimerL=%X\r\n", i, nReadHigh, nReadLow);
			nEraseTime += nReadLow;
			serialE++;
		}

		for (j = 0; j < NAND_PAGE_PER_BLOCK; ++j)
		{
			for (k = 0; k < NAND_PAGE_SIZE_IN_BYTES; k++)
			{
				pFlashWrite[k] = i + j + k;
			}
			memset(pFlashRead, 0, NAND_PAGE_SIZE_IN_BYTES);

			if ((i % 800 == 0) && (j % 20 == 0))
			{
				Timer_Config(10, 0xFFFF, 0xFFFFFFFF);
				Timer_Start(10,TIMER_ENAMODE_ONCE);
			}
			if (NAND_STATUS_PASSED != NANDPageWrite(i, j, pFlashWrite))
			{
				printf("Write Nand-Flash Block:%d,Page:%d Failed\r\n", i, j);
				continue;
			}
			if ((i % 800 == 0) && (j % 20 == 0))
			{
				nWriteHigh = 0, nWriteLow = 0;
				Timer_GetCount(10, &nWriteHigh, &nWriteLow);
				Timer_Close(10);
			}


			if ((i % 800 == 0) && (j % 20 == 0))
			{
				Timer_Config(9, 0xFFFF, 0xFFFFFFFF);
				Timer_Start(9, TIMER_ENAMODE_ONCE);
			}

			if (NAND_STATUS_PASSED != NANDPageRead(i, j, pFlashRead))
			{
				printf("Read Nand-Flash Block:%d,Page:%d Failed\r\n", i, j);
				continue;
			}

			if ((i % 800 == 0) && (j % 20 == 0))
			{
				nReadHigh = 0, nReadLow = 0;
				Timer_GetCount(9, &nReadHigh, &nReadLow);
				Timer_Close(9);
			}
#if 1
			if (0 != NANDDataIntegrityCheck(pFlashWrite, pFlashRead))
			{
				printf("Test Nand-Flash Block:%d,Page:%d Failed\r\n", i, j);
			}
			else
			{
				if ((i % 800 == 0) && (j % 20 == 0))
				{
					printf("Test Nand-Flash Block:%d,Page:%d OK,WTimerH=%X, WTimerL=%X, RTimerH=%X, RTimerL=%X\r\n", i, j, nWriteHigh, nWriteLow, nReadHigh, nReadLow);
					nWriteTime += nWriteLow;
					nReadTime += nReadLow;
					serialWR++;
				}
			}
#endif
		}
	}
#endif
	double time1 = (long double)nEraseTime / serialE * 6 / 1000 / 1000;//ms
	double time2 = (long double)nWriteTime / serialWR * 6 / 1000;//us
	double time3 = (long double)nReadTime / serialWR * 6 / 1000;//us
	printf("Test Nand-Flash, Erase a Block Time: %.2fms, Write a Page Time: %.2fus, Read a Page Time: %.2fus\r\n", time1, time2, time3);
	printf("Test Emif-Nand Finish\r\n");

}
/**********************测试Nand - End*******************************************/
#endif



/**********************测试GPIO - Start*******************************************/


void Test_GPIO()
{
	GPIO_DirModeSet(15,GPIO_DIR_INPUT);
	GPIO_DirModeSet(14,GPIO_DIR_OUTPUT);
	GPIO_IntTypeSet(15,GPIO_INT_TYPE_BOTHEDGE);
	GPIO_IntcEnable();
}


/**********************测试GPIO - End*******************************************/



/**************测试DDR读写-Start*****************/
volatile unsigned char* pWddr = (unsigned char*)0x90000000; //256---2G

static int DDRDataIntegrityCheck(volatile unsigned char* pWtr, volatile unsigned char* pRtr)
{
	unsigned int i = 0;
	for (i = 0; i < 1024 * 64; ++i)
	{
		if (pWtr[i] != pRtr[i])
		{
			return -1;
		}
	}
	return 0;
}

void Test_DDR()
{
	char blStatus = 1;
	char buf[128] = { 0 };
	unsigned int i = 0, j = 0;
	unsigned int nWTimerH = 0, nWTimerL = 0, nRTimerH = 0, nRTimerL = 0;
	unsigned long long nWriteTime = 0, nReadTime = 0;
	
	Uart_Printf("Test DDR Start\r\n", strlen("Test DDR Start\r\n"));
	unsigned char* pData = malloc(0x10000);
	if (NULL == pData)
	{
		Uart_Printf("malloc Failed\r\n", strlen("malloc Failed\r\n"));
	}
	volatile unsigned char* pRddr = pWddr;

	for (j = 0; j < 28672; j++)
	{
		memset(pData, 0, 0x10000);
		//测试写入时间
		if (j % 4096 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		for (i = 0; i < 1024 * 64; ++i)
		{
			*pWddr = i*j;
			pWddr++;
		}
		if (j % 4096 == 0)
		{
			Timer_GetCount(8, &nWTimerH, &nWTimerL);
			Timer_Close(8);
		}

		//测试读取时间
		if (j % 160 == 0)
		{
			Timer_Config(8, 0xFFFF, 0xFFFFFFFF);
			Timer_Start(8,TIMER_ENAMODE_ONCE);
		}
		for (i = 0; i < 1024 * 64; ++i)
		{
			pData[i] = pRddr[i];
		}
		if (j % 4096 == 0)
		{
			Timer_GetCount(8, &nRTimerH, &nRTimerL);
			Timer_Close(8);
		}

		if (-1 == DDRDataIntegrityCheck(pRddr, pData))
		{
			Uart_Printf("Test DDR Failed\r\n", strlen("Test DDR Failed\r\n"));
			blStatus=0;
			break;
		}
		else
		{
			if (j % 4096 == 0)
			{
				sprintf(buf, "Test DDR OK,WTimerH=%X,WTimerL=%X,RTimerH=%X,RTimerL=%X\r\n", nWTimerH, nWTimerL, nRTimerH, nRTimerL);
				Uart_Printf(buf, strlen(buf));
				nWriteTime += nWTimerL;
				nReadTime += nRTimerL;
			}
		}
		pRddr += (1024 * 64);
	}
	if(blStatus)
	{
		double time1 = 0, time2 = 0;
		time1 = (long double)nWriteTime / 1000 / 1000 / 7 * 6;//ms
		time2 = (long double)nReadTime / 1000 / 1000 / 7 * 6;
		sprintf(buf, "Test DDR Finished, Write 64KB Data Time: %.2fms, Read 64KB Data Time: %.2fms\r\n", time1, time2);
		Uart_Printf(buf, strlen(buf));
	}
	else
	{
		Uart_Printf("Test DDR No Finished,Failed\r\n",strlen("Test DDR No Finished,Failed\r\n"));
	}
	Uart_Printf("Test DDR Finished\r\n", strlen("Test DDR Finished\r\n"));
	free(pData);
}
/**************测试DDR读写-End*****************/


/**********************测试多核同步、启动 - Start*******************************************/



void Test_Core()
{
	Uart_Printf("Core 0 Send Notify to Core 1\r\n", strlen("Core 0 Send Notify to Core 1\r\n"));
	*(unsigned char*)0x0C0F1000 = 1;
	IPC_SendInterruptToCoreX(1, 0x10);
	Writeback_Cache(CACHE_L1D, (void*)(0x0C0F1000), 1, 1);
	while (!blFlag)
	{
	}
	Uart_Printf("Core 0 Received Notify from Core 7\r\n", strlen("Core 0 Received Notify from Core 7\r\n"));
}

/**********************测试多核同步、启动 - End*******************************************/

extern unsigned int g_Syn422Serial;
extern unsigned int g_Asyn422Serial;
extern unsigned int g_Can0Serial;
extern unsigned int g_Can1Serial;
extern unsigned int g_CamLinkSerial;

/**************测试SRIO中断配置-Start*****************/
void Test_SRIO_Interrupt_Deprecated()
{
	int i = 0;
	unsigned long long timer=0xffffff;
	unsigned int* pData = (unsigned int*)0x80100000;
	unsigned char buf[256] = { 0 };
	char LogInfo[128] = { 0 };
	EC_CanMsg_st CanMsg;
	CanMsg.blExtId = 1;
	CanMsg.len = 8;
	CanMsg.nId = 0x1111;



	for (i = 0; i < 256; i++)
	{
		buf[i] = i;
	}
	for (i = 0; i < 10000; i++)
	{
#if 1
		if(Syn422_SendData(buf, 29))
		{
			Uart_Printf("Send Syn422 Failed\r\n",strlen("Send Syn422 Failed\r\n"));
			return;
		}
		if(Asyn422_SendData(buf, 7))
		{
			Uart_Printf("Send ASyn422 Failed\r\n",strlen("Send ASyn422 Failed\r\n"));
			return;
		}
#endif
		if(Can_SendData(0, &CanMsg))
		{
			Uart_Printf("Send Syn422 Failed\r\n",strlen("Send Syn422 Failed\r\n"));
			return;
		}
		if(Can_SendData(1, &CanMsg))
		{
			Uart_Printf("Send Syn422 Failed\r\n",strlen("Send Syn422 Failed\r\n"));
			return;
		}

		Timer_Delay(0, 0, 300*1000/6);
	}
	Timer_Delay(0, 0, 1000*1000*1000/6);
#if 1
	int nCamLink = CamLinkSerial;
	sprintf(LogInfo,"CamLink Serial:%X\r\n",nCamLink);
	Uart_Printf(LogInfo,strlen(LogInfo));

	for (i = 0; i < nCamLink; i++)
	{
		sprintf(LogInfo, "%.8X  ", *(pData + i));
		Uart_Printf(LogInfo, strlen(LogInfo));
		if (i % 20 == 19)
		{
			Uart_Printf("\r\n", strlen("\r\n"));
		}
	}

	Uart_Printf("\r\n", strlen("\r\n"));

	timer=(unsigned long long)30000*1000*1000/6;
	Timer_Delay(0, timer>>32, timer&0xFFFFFFFF);
#endif
	sprintf(LogInfo, "Syn422  Serial----%d\r\n", Syn422Serial);
	Uart_Printf(LogInfo, strlen(LogInfo));

	sprintf(LogInfo, "Asyn422 Serial----%d\r\n", Asyn422Serial);
	Uart_Printf(LogInfo, strlen(LogInfo));

	sprintf(LogInfo, "Can0 Serial----%d\r\n", Can0Serial);
	Uart_Printf(LogInfo, strlen(LogInfo));

	sprintf(LogInfo, "Can1 Serial----%d\r\n", Can1Serial);
	Uart_Printf(LogInfo, strlen(LogInfo));
}

void Test_EDMA3_CCode()
{
	EDMA3CCParam_st paramSet;
	memset(&paramSet, 0, sizeof(EDMA3CCParam_st));

	int i = 0, j = 0;

	Init_EDAM3_Param(CamLink_SIZE, &paramSet);
	paramSet.opt.tcc = 1;
	paramSet.opt.itcchEn = 1;
	paramSet.srcAddr = (unsigned int)EdmaBuff1;
	paramSet.destAddr = (unsigned int)EdmaBuff2;
	paramSet.linkAddr = 0x4000 + (0x20 * 0);
	EDMA3_Init(0, 0, 0, 0, 0, paramSet);

	paramSet.opt.tcc = 0;
	paramSet.opt.itcchEn = 0;
	paramSet.srcAddr = (unsigned int)pMSMC1;
	paramSet.destAddr = (unsigned int)pMSMC2;
	paramSet.linkAddr = 0x4000 + (0x20 * 1);
	EDMA3_Init(0, 1, 1, 1, 0, paramSet);

	for (i = 0; i < 100; i++)
	{
		for (j = 0; j < CamLink_SIZE; j++)
		{
			EdmaBuff1[j] = i + j;
			pMSMC1[j] = i + j;
		}
		Invalidate_Cache(CACHE_L1D, (void*)EdmaBuff1, CamLink_SIZE, 1);
		EDMA3_StartChTrans(0);
		Timer_Delay(0, 0, 1000*1000*1000/6);
		char buf[128] = { 0 };
		sprintf(buf, "nCh0 = %d,nCh1 = %d\r\n", nCh0Serial, nCh1Serial);
		Uart_Printf(buf, strlen(buf));

		for (j = 0; j < CamLink_SIZE; j++)
		{
			sprintf(buf, "%d-", EdmaBuff2[j]);
			Uart_Printf(buf, strlen(buf));
		}
		sprintf(buf, "\r\nOK=%d\r\n", i + 1);
		Uart_Printf(buf, strlen(buf));
	}

}

void Test_Uart()
{
	int i = 0;
	char buf[128] = { 0 };
	unsigned char data[128] = { 0 };
	while (1)
	{
		if (blUartRxSerial)
		{
			int nCout = Uart_RecvInfo(data, 128);
			if (nCout)
			{
				for (i = 0; i < nCout; i++)
				{
					sprintf(buf, "%.2X ", data[i]);
					Uart_Printf(buf, strlen(buf));
				}
				Uart_Printf("\r\n", strlen("\r\n"));
			}
			blUartRxSerial = 0;
		}
	}
}






#if  1
void OnRecvNetPacket(unsigned char pIP[4], unsigned short nPort, unsigned char* pData, int nLen, void* pFunParam)
{
	int i=0;
	char buf[64]={0};
	sprintf(buf,"Receive (IP:%d.%d.%d.%d,Port:%d) %d Byte Data: ", pIP[0], pIP[1], pIP[2], pIP[3], nPort, nLen);
	Uart_Printf(buf, strlen(buf));
	for (i = 0; i < nLen; i++)
	{
		sprintf(buf,"%.2X ", pData[i]);
		Uart_Printf(buf, strlen(buf));
	}
	Uart_Printf("\r\n", strlen("\r\n"));
}


void Test_Ethernet()
{
	int i=0;

	unsigned short LocalPort = 8888;
	unsigned short RemotePort = 6666;
	unsigned char buf[2048] = "asdfghjklqwertyuiop\r\n";
	unsigned char Mac[6] = { 0x11,0x22,0x33,0x44,0x55,0x66 };
	unsigned char pIP[4] = { 172,16,1,44 };
	unsigned char Mask[4] = { 255,255,0,0 };
	unsigned char pDestIP[4] = { 172,16,1,37 };

//TCP
#if 1
	HwNet_Tcp_Init(NETWORK_TCP_CLIENT, Mac, pIP, Mask, LocalPort, pDestIP, RemotePort, 5, OnRecvNetPacket, NULL);
	for (i = 0; i < 100; i++)
	{
		int j = 0;
		for (j = 0; j < 32; j++)
		{
			buf[j] = i + j;
		}
		HwNet_SendTcpData(buf, 512);
		//Timer_Delay(0,0,100*1000*1000/6);
		//delay_ms(50);
	}


//UDP
#else
	HwNet_Udp_Init(Mac, pIP, Mask, LocalPort, 5, OnRecvNetPacket, NULL);
	for (i = 0; i < 100; i++)
	{
		int j = 0;
		for (j = 0; j < 32; j++)
		{
			buf[j] = i + j;
		}
		HwNet_SendUdpData(pDestIP, RemotePort, buf, 32);
		Timer_Delay(0,0,100*1000*1000/6);
	}
#endif // TCP

	printf("Test Finished\r\n");
}
#endif
