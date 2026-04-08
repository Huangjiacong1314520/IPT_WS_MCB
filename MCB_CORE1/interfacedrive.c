#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interfacedrive.h"
#include "drv_edma3.h"
#include "drv_cache.h"
#include "drv_intc.h"
#include "drv_uart.h"
#include "./srio/srio.h"

#define EC_SYSVER_INFO_LEN             32	//版本信息长度
#define EC_SYN422_DATA_LEN             32  //接收同步422数据长度 32
#define EC_HDSYNC_DATA_LEN             32   //接收硬同步数据长度
#define EC_ASYN422_DATA_LEN            12   //接收异步422数据长度 12
#define EC_CAN_DATA_LEN                32   //接收Can数据长度 16*2
#define EC_CAMLINK_DATA_LEN            320 * 256 * 2 //接收图像传输数据长度


#define SRIO_RECV_SYSVER        (0xF0000)
#define SRIO_RECV_US422         (0xF0400)
#define SRIO_RECV_HDSYNC        (0xF0C00)
#define SRIO_RECV_RS422         (0xF1000)
#define SRIO_RECV_CAN0          (0xF1800)
#define SRIO_RECV_CAN1          (0xF2000)
#define SRIO_RECV_CAMLINK       (0x00000)

#define SRIO_RECIVE_MMSC_BASEADDR (0x0C000000)



RecveData_st g_RecvParam[7];

unsigned char g_pSysVerPing[EC_SYSVER_INFO_LEN];
unsigned char g_pSysVerPong[EC_SYSVER_INFO_LEN];

unsigned char g_pSyn422Ping[EC_SYN422_DATA_LEN];
unsigned char g_pSyn422Pong[EC_SYN422_DATA_LEN];

unsigned char g_pHdsyncPing[EC_HDSYNC_DATA_LEN];
unsigned char g_pHdsyncPong[EC_HDSYNC_DATA_LEN];

unsigned char g_pAsyn422Ping[EC_ASYN422_DATA_LEN];
unsigned char g_pAsyn422Pong[EC_ASYN422_DATA_LEN];

unsigned char g_pCan0Ping[EC_CAN_DATA_LEN];
unsigned char g_pCan0Pong[EC_CAN_DATA_LEN];

unsigned char g_pCan1Ping[EC_CAN_DATA_LEN];
unsigned char g_pCan1Pong[EC_CAN_DATA_LEN];

unsigned char* g_pCamLinkPing = (unsigned char*)0x80000000;
unsigned char* g_pCamLinkPong = (unsigned char*)0x80030000;



unsigned int SwapDWord(unsigned int nData)
{
	return (((nData & 0xff000000) >> 24) | ((nData & 0x00ff0000) >> 8) | ((nData & 0x0000ff00) << 8) | ((nData & 0x000000ff) << 24));
}


unsigned short GetCalcCRC16(unsigned char* pData, int nLen)
{
	int i = 0, j = 0;
	unsigned short nCRC16 = 0xFFFF;  //设置初始值

	for (i = 0; i < nLen; ++i)
	{
		nCRC16 = pData[i] ^ (nCRC16 & 0xFF);
		for (j = 0; j < 8; j++)
		{
			if (nCRC16 & 0x1)
			{
				nCRC16 = (nCRC16 >> 1) ^ 0x1021;
			}
			else
			{
				nCRC16 = nCRC16 >> 1;
			}
		}	
	}
	return nCRC16;
}


void SRIO_ReadSysVerInfoByDoorbell(unsigned char * pData)
{
	Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_SYSVER), EC_SYSVER_INFO_LEN, 1);
	memcpy(pData, (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_SYSVER), EC_SYSVER_INFO_LEN);
}

void SRIO_ReadUS422InfoByDoorbell(unsigned char * pData)
{
	Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_US422), EC_SYN422_DATA_LEN, 1);
	memcpy(pData, (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_US422), EC_SYN422_DATA_LEN);
}

void SRIO_ReadHDSYNCInfoByDoorbell(unsigned char * pData)
{
	Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_HDSYNC), EC_HDSYNC_DATA_LEN, 1);
	memcpy(pData, (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_HDSYNC), EC_HDSYNC_DATA_LEN);
}

void SRIO_ReadRS422InfoByDoorbell(unsigned char * pData)
{
	Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_RS422), EC_ASYN422_DATA_LEN, 1);
	memcpy(pData, (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_RS422), EC_ASYN422_DATA_LEN);
}

void SRIO_ReadCan0InfoByDoorbell(unsigned char * pData)
{
	Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAN0), EC_CAN_DATA_LEN, 1);
	memcpy(pData, (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAN0), EC_CAN_DATA_LEN);
}

void SRIO_ReadCan1InfoByDoorbell(unsigned char * pData)
{
	Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAN1), EC_CAN_DATA_LEN, 1);
	memcpy(pData, (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAN1), EC_CAN_DATA_LEN);
}


void InitEDMA3_Param(unsigned int nLen, pEDMA3CCParam_st pParam)
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

	pParam->linkAddr = (unsigned short)0xFFFF;
	pParam->bCntReload = (unsigned short)0;

	// 编程 TCC
	pParam->opt.tcc = 0;//暂不赋值

						// 使能 Intermediate & Final 传输完成中断
	pParam->opt.tcintEn = 1;
	pParam->opt.itcintEn = 1;

	if (syncType == EDMA3_SYNCTYPE_A)
	{
		pParam->opt.syncDim = 0;
	}
	else
	{
		pParam->opt.syncDim = 1;
	}
}




static void test_isr_srio_mix(void* handle)
{
	unsigned int nRead = SRIOGetDoorbellStatus();//读中断状态寄存器
	//SRIOClearAllInterrupt(0,nRead);
	char str[100]={0};
#if 1
	switch (nRead)
	{
		case 1:
		{
			if (g_RecvParam[EC_SYS_VER].blping)
			{
				SRIO_ReadSysVerInfoByDoorbell(g_RecvParam[EC_SYS_VER].pRxFifoPing);
				g_RecvParam[EC_SYS_VER].blping = 0;
				g_RecvParam[EC_SYS_VER].blDataping = 1;
				g_RecvParam[EC_SYS_VER].blDatapong = 0;
			}
			else
			{
				SRIO_ReadSysVerInfoByDoorbell(g_RecvParam[EC_SYS_VER].pRxFifoPong);
				g_RecvParam[EC_SYS_VER].blping = 1;
				g_RecvParam[EC_SYS_VER].blDataping = 0;
				g_RecvParam[EC_SYS_VER].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(0);
			break;
		}
		case 2:
		{
			if (g_RecvParam[EC_SYN422_RECVE_DATA].blping)
			{
				SRIO_ReadUS422InfoByDoorbell(g_RecvParam[EC_SYN422_RECVE_DATA].pRxFifoPing);
				g_RecvParam[EC_SYN422_RECVE_DATA].blping = 0;
				g_RecvParam[EC_SYN422_RECVE_DATA].blDataping = 1;
				g_RecvParam[EC_SYN422_RECVE_DATA].blDatapong = 0;
			}
			else
			{
				SRIO_ReadUS422InfoByDoorbell(g_RecvParam[EC_SYN422_RECVE_DATA].pRxFifoPong);
				g_RecvParam[EC_SYN422_RECVE_DATA].blping = 1;
				g_RecvParam[EC_SYN422_RECVE_DATA].blDataping = 0;
				g_RecvParam[EC_SYN422_RECVE_DATA].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(1);
			Syn422Serial++;
			break;
		}
		case 4:
		{
			if (g_RecvParam[EC_HDSYNC_RECVE_DATA].blping)
			{
				SRIO_ReadHDSYNCInfoByDoorbell(g_RecvParam[EC_HDSYNC_RECVE_DATA].pRxFifoPing);
				g_RecvParam[EC_HDSYNC_RECVE_DATA].blping = 0;
				g_RecvParam[EC_HDSYNC_RECVE_DATA].blDataping = 1;
				g_RecvParam[EC_HDSYNC_RECVE_DATA].blDatapong = 0;
			}
			else
			{
				SRIO_ReadHDSYNCInfoByDoorbell(g_RecvParam[EC_HDSYNC_RECVE_DATA].pRxFifoPong);
				g_RecvParam[EC_HDSYNC_RECVE_DATA].blping = 1;
				g_RecvParam[EC_HDSYNC_RECVE_DATA].blDataping = 0;
				g_RecvParam[EC_HDSYNC_RECVE_DATA].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(2);
			break;
		}
		case 8:
		{
			if (g_RecvParam[EC_ASYN422_RECVE_DATA].blping)
			{
				SRIO_ReadRS422InfoByDoorbell(g_RecvParam[EC_ASYN422_RECVE_DATA].pRxFifoPing);
				g_RecvParam[EC_ASYN422_RECVE_DATA].blping = 0;
				g_RecvParam[EC_ASYN422_RECVE_DATA].blDataping = 1;
				g_RecvParam[EC_ASYN422_RECVE_DATA].blDatapong = 0;
			}
			else
			{
				SRIO_ReadRS422InfoByDoorbell(g_RecvParam[EC_ASYN422_RECVE_DATA].pRxFifoPong);
				g_RecvParam[EC_ASYN422_RECVE_DATA].blping = 1;
				g_RecvParam[EC_ASYN422_RECVE_DATA].blDataping = 0;
				g_RecvParam[EC_ASYN422_RECVE_DATA].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(3);
			Asyn422Serial++;
			break;
		}
		case 16:
		{
			if (g_RecvParam[EC_CAN0_RECVE_DATA].blping)
			{
				SRIO_ReadCan0InfoByDoorbell(g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPing);
				g_RecvParam[EC_CAN0_RECVE_DATA].blping = 0;
				g_RecvParam[EC_CAN0_RECVE_DATA].blDataping = 1;
				g_RecvParam[EC_CAN0_RECVE_DATA].blDatapong = 0;
			}
			else
			{
				SRIO_ReadCan0InfoByDoorbell(g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPong);
				g_RecvParam[EC_CAN0_RECVE_DATA].blping = 1;
				g_RecvParam[EC_CAN0_RECVE_DATA].blDataping = 0;
				g_RecvParam[EC_CAN0_RECVE_DATA].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(4);
			Can0Serial++;
			break;
		}
		case 32:
		{
			if (g_RecvParam[EC_CAN1_RECVE_DATA].blping)
			{
				SRIO_ReadCan1InfoByDoorbell(g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPing);
				g_RecvParam[EC_CAN1_RECVE_DATA].blping = 0;
				g_RecvParam[EC_CAN1_RECVE_DATA].blDataping = 1;
				g_RecvParam[EC_CAN1_RECVE_DATA].blDatapong = 0;
			}
			else
			{
				SRIO_ReadCan1InfoByDoorbell(g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPong);
				g_RecvParam[EC_CAN1_RECVE_DATA].blping = 1;
				g_RecvParam[EC_CAN1_RECVE_DATA].blDataping = 0;
				g_RecvParam[EC_CAN1_RECVE_DATA].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(5);
			Can1Serial++;
			break;
		}
		case 64:
		{
			Invalidate_Cache(CACHE_L1D, (void*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAMLINK), EC_CAMLINK_DATA_LEN, 1);
			if (g_RecvParam[EC_CAMLINK_RECVE_DATA].blping)
			{
				EDMA3_StartChTrans(0);
				g_RecvParam[EC_CAMLINK_RECVE_DATA].blping = 0;
				g_RecvParam[EC_CAMLINK_RECVE_DATA].blDataping = 1;
				g_RecvParam[EC_CAMLINK_RECVE_DATA].blDatapong = 0;
			}
			else
			{
				EDMA3_StartChTrans(1);
				g_RecvParam[EC_CAMLINK_RECVE_DATA].blping = 1;
				g_RecvParam[EC_CAMLINK_RECVE_DATA].blDataping = 0;
				g_RecvParam[EC_CAMLINK_RECVE_DATA].blDatapong = 1;
			}
			SRIOClearDoorbellIntcStatus(6);
			memcpy((unsigned char*)(0x80100000 + CamLinkSerial * 4), (unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAMLINK), 4);
			CamLinkSerial++;
			break;
		}
		default:
			sprintf(str,"nRead = %d\r\n",nRead);
			Uart_Printf(str,strlen(str));
			break;
	}
#endif
}


void EC_OpenDevice()
{
	Syn422Serial = 0;
	Asyn422Serial = 0;
	Can0Serial = 0;
	Can1Serial = 0;
	CamLinkSerial = 0;

	//系统信息,固定32Byte
	g_RecvParam[EC_SYS_VER].blDataping = 0;
	g_RecvParam[EC_SYS_VER].blDatapong = 0;
	g_RecvParam[EC_SYS_VER].blping = 1;
	g_RecvParam[EC_SYS_VER].pRxFifoPing = g_pSysVerPing;
	g_RecvParam[EC_SYS_VER].pRxFifoPong = g_pSysVerPong;

	//同步422
	g_RecvParam[EC_SYN422_RECVE_DATA].blDataping = 0;
	g_RecvParam[EC_SYN422_RECVE_DATA].blDatapong = 0;
	g_RecvParam[EC_SYN422_RECVE_DATA].blping = 1;
	g_RecvParam[EC_SYN422_RECVE_DATA].pRxFifoPing = g_pSyn422Ping;
	g_RecvParam[EC_SYN422_RECVE_DATA].pRxFifoPong = g_pSyn422Pong;

	//硬件同步
	g_RecvParam[EC_HDSYNC_RECVE_DATA].blDataping = 0;
	g_RecvParam[EC_HDSYNC_RECVE_DATA].blDatapong = 0;
	g_RecvParam[EC_HDSYNC_RECVE_DATA].blping = 1;
	g_RecvParam[EC_HDSYNC_RECVE_DATA].pRxFifoPing = g_pHdsyncPing;
	g_RecvParam[EC_HDSYNC_RECVE_DATA].pRxFifoPong = g_pHdsyncPong;

	//异步422
	g_RecvParam[EC_ASYN422_RECVE_DATA].blDataping = 0;
	g_RecvParam[EC_ASYN422_RECVE_DATA].blDatapong = 0;
	g_RecvParam[EC_ASYN422_RECVE_DATA].blping = 1;
	g_RecvParam[EC_ASYN422_RECVE_DATA].pRxFifoPing = g_pAsyn422Ping;
	g_RecvParam[EC_ASYN422_RECVE_DATA].pRxFifoPong = g_pAsyn422Pong;

	//Can0
	g_RecvParam[EC_CAN0_RECVE_DATA].blDataping = 0;
	g_RecvParam[EC_CAN0_RECVE_DATA].blDatapong = 0;
	g_RecvParam[EC_CAN0_RECVE_DATA].blping = 1;
	g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPing = g_pCan0Ping;
	g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPong = g_pCan0Pong;

	//Can1
	g_RecvParam[EC_CAN1_RECVE_DATA].blDataping = 0;
	g_RecvParam[EC_CAN1_RECVE_DATA].blDatapong = 0;
	g_RecvParam[EC_CAN1_RECVE_DATA].blping = 1;
	g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPing = g_pCan1Ping;
	g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPong = g_pCan1Pong;

	//图像
	g_RecvParam[EC_CAMLINK_RECVE_DATA].blDataping = 0;
	g_RecvParam[EC_CAMLINK_RECVE_DATA].blDatapong = 0;
	g_RecvParam[EC_CAMLINK_RECVE_DATA].blping = 1;
	g_RecvParam[EC_CAMLINK_RECVE_DATA].pRxFifoPing = g_pCamLinkPing;
	g_RecvParam[EC_CAMLINK_RECVE_DATA].pRxFifoPong = g_pCamLinkPong;

	//SRIO中断
	INTC_Init();
	INTC_Open(20, (int)10, test_isr_srio_mix, NULL);

	SRIODoorbellRouteCtl(0);//专用路由表
	int i = 0;
	for (i = 0; i < 7; i++)
	{
		SRIODoorbellIntcRouteSet(i, 0);
	}



	//初始化SRIO
	if(SRIOInitialize(  0x12, SRIO_NO_LOOPBACK))
	{
		printf("SRIO Link Failed\r\n");
		return;
	}


	//配置EDMA3
	EDMA3CCParam_st param;
	InitEDMA3_Param(EC_CAMLINK_DATA_LEN, &param);

	param.srcAddr = (unsigned int)((unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAMLINK));
	param.destAddr = (unsigned int)g_RecvParam[EC_CAMLINK_RECVE_DATA].pRxFifoPing;
	param.linkAddr = 0x4000 + (0x20 * 0);
	EDMA3_Init(0, 0, 0, 0, 0, param);

	param.srcAddr = (unsigned int)((unsigned char*)(SRIO_RECIVE_MMSC_BASEADDR + SRIO_RECV_CAMLINK));
	param.destAddr = (unsigned int)g_RecvParam[EC_CAMLINK_RECVE_DATA].pRxFifoPong;
	param.linkAddr = 0x4000 + (0x20 * 1);
	EDMA3_Init(0, 1, 1, 1, 0, param);
}



int GetSysVerInfo(unsigned char * pData, unsigned int nLen)
{
	int nInfoLen = 0;
	unsigned char buff[128] = { 0 };
	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}


	if (nLen >= EC_SYSVER_INFO_LEN)
	{
		nInfoLen = EC_SYSVER_INFO_LEN;
	}
	else
	{
		nInfoLen = nLen;
	}

	buff[0] = 0x4A;
	buff[1] = 0x5A;
	buff[2] = 0x01;
	buff[29] = 0x4E;
	buff[30] = 0xAF;
	if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, 0x0, SRIO_PACKET_TYPE_STREAMWRITE, buff, 32))//发送获取版本信息的指令给FPGA
	{
		return EC_SRIO_WRITE_ERROR;
	}

	//等待FGPA响应
	int nTime = 100000;
	while ((0 == g_RecvParam[EC_SYS_VER].blDataping) && (0 == g_RecvParam[EC_SYS_VER].blDatapong))
	{
		nTime--;
		if (!nTime)
		{
			return 0;
		}
		else
		{
			continue;
		}
	}

	if (g_RecvParam[EC_SYS_VER].blDataping)
	{
		memcpy(pData, g_RecvParam[EC_SYS_VER].pRxFifoPing, nInfoLen);
		g_RecvParam[EC_SYS_VER].blDataping = 0;//清除状态
	}
	else
	{
		memcpy(pData, g_RecvParam[EC_SYS_VER].pRxFifoPong, nInfoLen);
		g_RecvParam[EC_SYS_VER].blDatapong = 0;//清除状态
	}	
	return (int)nInfoLen;
}

int Syn422_RecvData(unsigned char * pData, unsigned int nLen)
{
	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}
	

	int nInfoLen = 0;
	if (nLen >= EC_SYN422_DATA_LEN)
	{
		nInfoLen = EC_SYN422_DATA_LEN;
	}
	else
	{
		nInfoLen = nLen;
	}

	if (!g_RecvParam[EC_SYN422_RECVE_DATA].blping)
	{	
		if (g_RecvParam[EC_SYN422_RECVE_DATA].blDataping)
		{
			memcpy(pData, g_RecvParam[EC_SYN422_RECVE_DATA].pRxFifoPing, nInfoLen);
			g_RecvParam[EC_SYN422_RECVE_DATA].blDataping = 0;//清除状态
			return (int)nInfoLen;
		}
	}
	else
	{
		if (g_RecvParam[EC_SYN422_RECVE_DATA].blDatapong)
		{
			memcpy(pData, g_RecvParam[EC_SYN422_RECVE_DATA].pRxFifoPong, nInfoLen);
			g_RecvParam[EC_SYN422_RECVE_DATA].blDatapong = 0;//清除状态
			return (int)nInfoLen;
		}
	}
	return EC_OK;
}

int Syn422_SendData(unsigned char * pData, unsigned int nLen)
{
	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}
	if (nLen < 29)
	{
		return EC_PARAM2_ERROR;
	}

	unsigned char pBuf[128] = { 0 };
	memcpy(pBuf, pData, 29);
	if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, 0x10000, SRIO_PACKET_TYPE_STREAMWRITE, pBuf, 32))
	{
		return EC_SRIO_WRITE_ERROR;
	}
	return EC_OK;
}

int HDSYNC_RecvData(unsigned char * pData, unsigned int nLen)
{
	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}

	int nInfoLen = 0;
	if (nLen >= EC_HDSYNC_DATA_LEN)
	{
		nInfoLen = EC_HDSYNC_DATA_LEN;
	}
	else
	{
		nInfoLen = nLen;
	}

	if (g_RecvParam[EC_HDSYNC_RECVE_DATA].blping)
	{
		if (g_RecvParam[EC_HDSYNC_RECVE_DATA].blDatapong)
		{
			memcpy(pData, g_RecvParam[EC_HDSYNC_RECVE_DATA].pRxFifoPong, nInfoLen);
			g_RecvParam[EC_HDSYNC_RECVE_DATA].blDatapong = 0;//清除状态
			return (int)nInfoLen;
		}
	}
	else
	{
		if (g_RecvParam[EC_HDSYNC_RECVE_DATA].blDataping)
		{
			memcpy(pData, g_RecvParam[EC_HDSYNC_RECVE_DATA].pRxFifoPing, nInfoLen);
			g_RecvParam[EC_HDSYNC_RECVE_DATA].blDataping = 0;//清除状态
			return (int)nInfoLen;
		}
	}
	return EC_OK;
}

int Asyn422_RecvData(unsigned char * pData, unsigned int nLen)
{
	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}

	int nInfoLen = 0;
	if (nLen >= EC_ASYN422_DATA_LEN)
	{
		nInfoLen = EC_ASYN422_DATA_LEN;
	}
	else
	{
		nInfoLen = nLen;
	}

	if (g_RecvParam[EC_ASYN422_RECVE_DATA].blping)
	{
		if (g_RecvParam[EC_ASYN422_RECVE_DATA].blDatapong)
		{
			memcpy(pData, g_RecvParam[EC_ASYN422_RECVE_DATA].pRxFifoPong, 12);
			g_RecvParam[EC_ASYN422_RECVE_DATA].blDatapong = 0;//清除状态
			return (int)nInfoLen;
		}
	}
	else
	{
		if (g_RecvParam[EC_ASYN422_RECVE_DATA].blDataping)
		{
			memcpy(pData, g_RecvParam[EC_ASYN422_RECVE_DATA].pRxFifoPing, 12);
			g_RecvParam[EC_ASYN422_RECVE_DATA].blDataping = 0;//清除状态
			return (int)nInfoLen;
		}
	}
	return EC_OK;
}

int Asyn422_SendData(unsigned char * pData, unsigned int nLen)
{
	int nSerial = 0;

	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}

	int i = 0;
	int nSendLen = 0;
	nSerial = 8;
	unsigned char buff[256] = { 0 };
	buff[7] = nLen + 5;//包长度
	buff[nSerial]=0x10;
	nSerial++;


	for (i = 0; i < nLen; i++)//ID + Data
	{
		buff[nSerial] = pData[i];
		nSerial++;
	}
	buff[nSerial] = 0x77; //CRC1
	nSerial++;

	buff[nSerial] = 0xB2; //CRC2
	nSerial++;

	buff[nSerial] = 0x10;
	nSerial++;

	buff[nSerial] = 0x03;

	if (0 != buff[7]%8)
	{
		nSendLen = 8 + (buff[7] / 8 + 1) * 8;//SRIO包StreamWrite格式8字节对齐
	}
	else
	{
		nSendLen = 8 + buff[7];
	}
	if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, 0x30000, SRIO_PACKET_TYPE_STREAMWRITE, buff, nSendLen))
	{
		return EC_SRIO_WRITE_ERROR;
	}
	return EC_OK;
}

int Can_RecvData(char nChl, pEC_CanMsg_st pCanMsg)
{
	unsigned char Data[16]={0};
	unsigned int nRead=0;

	if ((nChl < 0) && (nChl > 1))
	{
		return EC_PARAM1_ERROR;
	}

	if (NULL == pCanMsg)
	{
		return EC_PARAM2_ERROR;
	}
	
	if (nChl)
	{
		if (g_RecvParam[EC_CAN1_RECVE_DATA].blping)
		{
			if (g_RecvParam[EC_CAN1_RECVE_DATA].blDatapong)
			{
				memcpy(Data, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPong + 4, 4);
				memcpy(Data + 4, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPong + 12, 4);
				memcpy(Data + 8, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPong + 20, 4);
				memcpy(Data + 12, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPong + 28, 4);
				g_RecvParam[EC_CAN1_RECVE_DATA].blDatapong = 0;//清除状态
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if (g_RecvParam[EC_CAN1_RECVE_DATA].blDataping)
			{
				memcpy(Data, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPing + 4, 4);
				memcpy(Data + 4, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPing + 12, 4);
				memcpy(Data + 8, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPing + 20, 4);
				memcpy(Data + 12, g_RecvParam[EC_CAN1_RECVE_DATA].pRxFifoPing + 28, 4);
				g_RecvParam[EC_CAN1_RECVE_DATA].blDataping = 0;//清除状态
			}
			else
			{
				return 0;
			}
		}		
	}
	else
	{
		if (g_RecvParam[EC_CAN0_RECVE_DATA].blping)
		{
			if (g_RecvParam[EC_CAN0_RECVE_DATA].blDatapong)
			{
				memcpy(Data, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPong + 4, 4);
				memcpy(Data + 4, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPong + 12, 4);
				memcpy(Data + 8, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPong + 20, 4);
				memcpy(Data + 12, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPong + 28, 4);
				g_RecvParam[EC_CAN0_RECVE_DATA].blDatapong = 0;//清除状态
			}
			else
			{
				return 0;
			}
		}
		else
		{
			if (g_RecvParam[EC_CAN0_RECVE_DATA].blDataping)
			{
				memcpy(Data, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPing + 4, 4);
				memcpy(Data + 4, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPing + 12, 4);
				memcpy(Data + 8, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPing + 20, 4);
				memcpy(Data + 12, g_RecvParam[EC_CAN0_RECVE_DATA].pRxFifoPing + 28, 4);
				g_RecvParam[EC_CAN0_RECVE_DATA].blDataping = 0;//清除状态
			}
			else
			{
				return 0;
			}
		}
	}
	
	nRead=SwapDWord(*(unsigned int*)Data);
	pCanMsg->blExtId = (nRead & (1 << 19)) ? 1 : 0;  //bit12（our bit19） is IDE
	pCanMsg->Reserve = (nRead & (1 << 20)) ? 1 : 0; //bit11（our bit20） is SRR/RTR
	pCanMsg->nId = nRead >> 21;  //bit10-bit0(our bit21-bit31)


	if (pCanMsg->blExtId)    //如果是扩展ID，那么还要进一步处理
	{
		pCanMsg->nId <<= 18; //文档上标注为:ID [28..18]，所以猜测如果是扩展ID，那么需要把标准ID部分左移18个bit，然后或操作扩展部分，扩展部分标注的是:ID[17..0]
		pCanMsg->nId |= ((nRead >> 1) & 0x3FFFF);    //bit30-bit13 (our bit1-bit18)is Extern Id's Bit0--bit17
	}

	nRead=SwapDWord(*(unsigned int*)(Data+4));
	pCanMsg->len=nRead>>28;

	memcpy(pCanMsg->pData,Data+8,8);
	return 16;
}

int Can_SendData(char nChl, pEC_CanMsg_st pCanMsg)
{
	if ((nChl < 0) && (nChl > 1))
	{
		return EC_PARAM1_ERROR;
	}

	if ((NULL == pCanMsg) || ((NULL != pCanMsg) && (pCanMsg->len > 8)))
	{
		return EC_PARAM2_ERROR;
	}

	int Serial = 3;
	unsigned char buff[32] = { 0 };
	unsigned int nTemp = 0;
	unsigned int pWrite[2] = { 0 };

	if (pCanMsg->blExtId)
	{
		pWrite[0] = pCanMsg->nId & 0x3FFFF;	//bit0-bit17
		pWrite[0] <<= 1;	//set to reg data bit30-bit13 (our bit1-bit18)
		nTemp = (pCanMsg->nId >> 18) & 0x7FF;
		nTemp <<= 21;
		pWrite[0] |= nTemp;	//bit18-bit28 set to Reg Data bit10-bit0 (our bit21-bit31)

		pWrite[0] |= (1 << 20);	//SRR/RTR is 1 at ExtId bit11 (our bit20)
		pWrite[0] |= (1 << 19);	//extid Flag, bit12 (our bit19)

		/*
		if (pCanMsg->Reserve)
		{
			pWrite[0] |= 1;	//bit31 Only ExtId Use (our bit0);
		}
		*/
	}
	else
	{
		nTemp = pCanMsg->nId & 0x7FF;
		pWrite[0] = nTemp << 21;	//bit10-bit0 is normal id (our bit21-bit31)
		/*
		if (pCanMsg->Reserve)
		{
			pWrite[0] |= (1 << 20);	//SRR/RTR is 1 at ExtId bit11 (our bit20)
		}
		*/
	}
	pWrite[1] = pCanMsg->len << 28;	//前面已经判断过长度有效了 bit3-bit0 (our bit28-bit31)

	pWrite[0] = SwapDWord(pWrite[0]);
	pWrite[1] = SwapDWord(pWrite[1]);

	buff[Serial] = 0x30;
	Serial++;

	memcpy(buff + Serial, pWrite, 4);
	Serial += 7;

	buff[Serial] = 0x34;
	Serial++;

	memcpy(buff + Serial, pWrite + 1, 4);
	Serial += 7;

	buff[Serial] = 0x38;
	Serial++;

	memcpy(buff + Serial, pCanMsg->pData, 4);
	Serial += 7;

	buff[Serial] = 0x3C;
	Serial++;

	memcpy(buff + Serial, pCanMsg->pData + 4, 4);
	Serial += 4;

	if (nChl)
	{
		if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, 0x50000, SRIO_PACKET_TYPE_STREAMWRITE, buff, Serial))
		{
			return EC_SRIO_WRITE_ERROR;
		}
	}
	else
	{
	    if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, 0x40000, SRIO_PACKET_TYPE_STREAMWRITE, buff, Serial))
		{
			return EC_SRIO_WRITE_ERROR;
		}
	}
	return EC_OK;
}



int CamLink_RecvData(unsigned char * pData, unsigned int nLen)
{
	EDMA3CCParam_st param;
	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;
	}

	int nInfoLen = 0;
	if (nLen >= EC_CAMLINK_DATA_LEN)
	{
		nInfoLen = EC_CAMLINK_DATA_LEN;
	}
	else
	{
		nInfoLen = nLen;
	}
	InitEDMA3_Param(nInfoLen, &param);
	param.destAddr = (unsigned int)pData;

	if (g_RecvParam[EC_CAMLINK_RECVE_DATA].blping)
	{
		if (g_RecvParam[EC_CAMLINK_RECVE_DATA].blDatapong)
		{
			param.srcAddr = (unsigned int)g_RecvParam[EC_CAMLINK_RECVE_DATA].pRxFifoPong;
			EDMA3_Init(0, 3, 3, 3, 0, param);
			EDMA3_StartChTrans(3);
			Invalidate_Cache(CACHE_L1D, pData, nInfoLen, 1);
			g_RecvParam[EC_CAMLINK_RECVE_DATA].blDatapong = 0;
			return (int)nInfoLen;
		}
	}
	else
	{
		if (g_RecvParam[EC_CAMLINK_RECVE_DATA].blDataping)
		{
			param.srcAddr = (unsigned int)g_RecvParam[EC_CAMLINK_RECVE_DATA].pRxFifoPing;
			EDMA3_Init(0, 2, 2, 2, 0, param);
			EDMA3_StartChTrans(2);
			Invalidate_Cache(CACHE_L1D, pData, nInfoLen, 1);
			g_RecvParam[EC_CAMLINK_RECVE_DATA].blDataping = 0;
			return (int)nInfoLen;
		}
	}
	return EC_OK;
}


#if 0
int Syn485_SendData(EC1900DEV hDev, unsigned char * pData, unsigned int nLen)
{
	pEC1900Hanle_st pHnd = (pEC1900Hanle_st)hDev;
	if (NULL == hDev)
	{
		return EC_PARAM1_ERROR;
	}

	if (!pHnd->hDev)
	{
		return EC_DEVICE_UINIT;
	}

	if (NULL == pData)
	{
		return EC_PARAM2_ERROR;
	}


	int i = 0, nSerial = 0, nSendLen = 0, RestLen = nLen + 3;//剩余的发往FPGA的数据长度=负载长度+3字节的固定头
	unsigned char buff[256] = { 0 };
	buff[7] = 0x4A;
	buff[15] = 0x5A;
	buff[23] = 0x54;

	char blDataTail = 0;
	if (RestLen > 32)
	{
		nSendLen = 29;
	}
	else
	{
		nSendLen = nLen;
		blDataTail = 1;
	}

	for (i = 0; i < nSendLen; i++)
	{
		buff[(i + 3) * 8 + 7] = pData[i];
	}

	if (blDataTail)//如果第一包数据包含了最后一个字节
	{
		buff[(nSendLen + 3) * 8 - 2] = 1;
	}
	SRIO_WriteData(0, 0x20000, buff, (nSendLen + 3) * 8);//发送该包的第一部分数据
	memset(buff, 0, 256);
	RestLen = RestLen - nSendLen - 3;
	nSerial = nSendLen;//记录当前从pData数组里取数的指针位置

	while (RestLen)
	{
		if (RestLen > 32)
		{
			nSendLen = 32;
			blDataTail = 0;
		}
		else
		{
			nSendLen = RestLen;
			blDataTail = 1;
		}

		for (i = 0; i < nSendLen; i++)
		{
			buff[i * 8 + 7] = pData[nSerial];
			nSerial++;
		}
		if (blDataTail)
		{
			buff[nSendLen * 8 - 2] = 1;
		}
		SRIO_WriteData(0, 0x20000, buff, nSendLen * 8);
		RestLen -= nSendLen;
	}
	
	return EC_OK;
}
#else

/*
 * 485数据发送
 */
int Syn485_SendData(unsigned char * pData, unsigned int nLen)
{

    if (NULL == pData)
    {
        return EC_PARAM1_ERROR;
    }

	if (nLen < 3072)
	{
		return EC_PARAM2_ERROR;
	}
  
	if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, 0x20000, SRIO_PACKET_TYPE_STREAMWRITE, pData, 3072))
	{
		return EC_SRIO_WRITE_ERROR;
	}
    return EC_OK;
}
#endif


int SendSubtitleInfo(unsigned char* pData, unsigned int nLen)
{
	unsigned int nOffset=0x60000;

	if (NULL == pData)
	{
		return EC_PARAM1_ERROR;

	}

	if(nLen < 10240)
	{
		return EC_PARAM2_ERROR;
	}


	if (SRIOWriteData(FPGA_DEVICE_ID_8BIT, nOffset, SRIO_PACKET_TYPE_STREAMWRITE, pData, 10240))
	{
		return EC_SRIO_WRITE_ERROR;
	}

	return EC_OK;
}

