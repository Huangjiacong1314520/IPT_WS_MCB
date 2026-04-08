/*
 * SRIO.c
 *
 *  Created on: 2020-8-31
 *      Author: linxi
 */
#include "string.h"
#include "SRIO.h"


#include "../include/hw_types.h"


#include "../drv_psc.h"
#include <stdio.h>
#include <stdlib.h>
#include <c6x.h>
#include "../drv_intc.h"
#include "../drv_srio.h"
#include "hw_soc_c6678.h"


//common use
unsigned char SRIOPacketType(unsigned int uPacketType)
{
    unsigned char cType = SRIO_StreamWrite;
    switch(uPacketType)
    {
           case SRIO_PACKET_TYPE_NRead:
               cType=SRIO_NRead;
               break;
            case SRIO_PACKET_TYPE_NWRITE:
                cType=SRIO_NWrite;
                break;
            case SRIO_PACKET_TYPE_NWRITERESPONSE:
                cType=SRIO_NWriteResponse;
                break;
            case SRIO_PACKET_TYPE_STREAMWRITE:
                cType=SRIO_StreamWrite;
                break;
            case SRIO_PACKET_TYPE_DOORBELL:
                cType=SRIO_DoorBell;
                break;
            case SRIO_PACKET_TYPE_MAINTENANCE_READ:
                cType = SRIO_MaintenanceRead;
                break;
            case SRIO_PACKET_TYPE_MAINTENANCE_WRITE:
                cType = SRIO_MaintenanceWrite;
                break;
            case SRIO_PACKET_TYPE_MAINTENANCE_READ_RSP:
                cType = SRIO_MaintenanceReadRsp;
                break;
            case SRIO_PACKET_TYPE_MAINTENANCE_WRITE_RSP:
                cType = SRIO_MaintenanceWriteRsp;
                break;
            case SRIO_PACKET_TYPE_MAINTENANCE_PORT_WRITE:
                cType = SRIO_MaintenancePortWrite;
                break;
            case SRIO_PACKET_TYPE_RSP_NO_DATA:
                cType = SRIO_RespNoData;
                break;
            case SRIO_PACKET_TYPE_RSP_WITH_DATA:
                cType = SRIO_RespWithData;
                break;
            default:
                cType=SRIO_StreamWrite;
                break;
      }
    return cType;
}
int SRIOInitialize( unsigned char cDSPID, SRIOLoopbackMode  loopbackMode )
{
    unsigned char cMode  = SRIO_Mode1_2_1x_1_2x;
    SRIOInitStatus  initStatus = SRIOInit( cMode,  cDSPID,    loopbackMode);
    if (SRIO_INIT_OK == initStatus )
	    return 0;
    else
    {
        //printf("srio init failed : %d\n", initStatus);
        return 1;
    }
}

// Doorbell interrupts mapped to new 8 interrupts versus the legacy 16 interrupts 3.8.9
 /* 设置Doorbell路径，以确定使用哪个路由表。该配置意味着中断路由表的配置如下:
     * 设置为0, 专用路由表:
     *  Interrupt Destination 0 - INTDST 16
     *  ........
     *  Interrupt Destination 7 - INTDST 23
     *  设置为1, 通用路由表:
     *  Interrupt Destination 0 - INTDST 0
     *  .....
     *  Interrupt Destination 15 - INTDST 15
     *  需要在SRIO初始化之后设置才会生效
     */
void SRIODoorbellRouteCtl(unsigned char nIntcMode)
{
	if (nIntcMode)
	{
		SRIODoorBellInterruptRoutingControl(1);
	}
	else
	{
		SRIODoorBellInterruptRoutingControl(0);
	}
}

//Route the Doorbell bits 'DoorBell' for Doorbell '0' to destination 'InterruptDestination'
void SRIODoorbellIntcRouteSet(unsigned char Doorbell, unsigned char InterruptDestination)
{
	SRIODoorBellInterruptConditionRoutingSet(0, Doorbell, InterruptDestination);
}

unsigned int SRIOGetDoorbellStatus()
{
	return SRIODoorBellInterruptGet(0);
}

void SRIOClearDoorbellIntcStatus(unsigned char Doorbell)
{
	SRIODoorBellInterruptClear(0, Doorbell);
}

SRIORWStatus  SRIOWriteData(unsigned char cRemoteDevID_8Bit, unsigned int uWrAddr, unsigned char cPktType, unsigned char *pLocalBuffer, int nLen)
{
	SRIOLSUConfig cfg;
	unsigned char cLSU =0;
	unsigned char cType=0;
	unsigned int  uRetryCount = 0xFFFF;
	unsigned char cPort = 0;
    if (FPGA_DEVICE_ID_8BIT != cRemoteDevID_8Bit)
    {
        cPort = 2;
        cLSU = 1;
    }

	unsigned char cStatus = SRIOLSUFullAndBusyCheck(cLSU);
	while(cStatus != 0)
	{
	    cStatus = SRIOLSUFullAndBusyCheck(cLSU);
	    uRetryCount--;
        if (!uRetryCount)
        {
            if (cStatus & 0x0F)
                return TIMEOUT_LSU_BUSY;
            else
                return TIMEOUT_LSU_FULL;
        }
	}


	cType = SRIOPacketType(cPktType);

	SRIO_InitLsuCfg(&cfg, (Uint32)pLocalBuffer, uWrAddr, cType, nLen,  cRemoteDevID_8Bit, cPort);

	if(cType == SRIO_DoorBell)
	{
		cfg.DoorBell.Enable = TRUE;//是否需要在传输完成后发送doorbell，0表示不需要
		cfg.DoorBell.Info = pLocalBuffer[0]<<8 | pLocalBuffer[1];
	}
	else
	{
	    cfg.DoorBell.Enable = FALSE;
	    cfg.DoorBell.Info = 0xa5a5;
	}


	uRetryCount=0xFFFFF;
	SRIORWStatus nStatus=COMPLETED_NOERRORS;
	SRIODirectIOTransfer(cLSU, &cfg);

	nStatus = SRIOLSUStatusGet(cLSU, &cfg);//得到完成码（调试用）
	while(nStatus != COMPLETED_NOERRORS)
	{
		nStatus = SRIOLSUStatusGet(cLSU, &cfg);//得到完成码（调试用）
		uRetryCount--;
		if(!uRetryCount)
		{
			return TIMEOUT;
		}
	}

	return COMPLETED_NOERRORS;
}

SRIORWStatus   SRIOReadData(unsigned char cRemoteDevID_8Bit, unsigned int uRdAddr, unsigned char cPackType, unsigned char *pLocalBuff, int nLen)
{
        unsigned char cLSU = 0;
        unsigned char cType=0;

      // SRIO_PreTransferSet(uPort);//和FPGA通信不需要本行和下面的检查
 //       if (0 != SRIO_LinkStatusCheck(uPort))
        {
//            //SRIO_MatchAckID(uPort,  uRemoteDevID_8Bit,   uPort);
//            SRIOPLMPortForeceReLink(uPort);
//            SRIOPMMSoftRestPort(uPort);
        }
        unsigned char cPort = 0;
        if (FPGA_DEVICE_ID_8BIT != cRemoteDevID_8Bit)
        {
            cPort = 2;
            cLSU = 1;
        }
        cType = SRIOPacketType(cPackType);
        return SRIO_DirectIO((unsigned int)pLocalBuff, uRdAddr,  cRemoteDevID_8Bit, nLen, cPort, cLSU,
                      cType , 0xa5a5);
}


