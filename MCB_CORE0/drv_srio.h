/*
 * SRIO.h
 *
 *  Created on: 2020-8-31
 *      Author: linxi
 */

#ifndef SRIO_H_
#define SRIO_H_


// SRIO 端口模式
#define SRIO_Mode0_4_1x      0
#define SRIO_Mode1_2_1x_1_2x 1
#define SRIO_Mode2_1_2x_2_1x 2
#define SRIO_Mode3_2_2x      3
#define SRIO_Mode4_1_4x      4

//SRIO 速率
#define SRIO_LINK_SPEED_1p25     0
#define SRIO_LINK_SPEED_2p5      1
#define SRIO_LINK_SPEED_3p125    2
#define SRIO_LINK_SPEED_5        3

//SRIO 包类型
#define SRIO_PACKET_TYPE_NRead   			0
#define SRIO_PACKET_TYPE_NWRITE  			               1
#define SRIO_PACKET_TYPE_NWRITERESPONSE	       2
#define SRIO_PACKET_TYPE_STREAMWRITE                 3
#define SRIO_PACKET_TYPE_DOORBELL                        4
#define SRIO_PACKET_TYPE_MAINTENANCE_READ        5
#define SRIO_PACKET_TYPE_MAINTENANCE_WRITE       6
#define SRIO_PACKET_TYPE_MAINTENANCE_READ_RSP  7
#define SRIO_PACKET_TYPE_MAINTENANCE_WRITE_RSP 8
#define SRIO_PACKET_TYPE_MAINTENANCE_PORT_WRITE  9
#define SRIO_PACKET_TYPE_RSP_NO_DATA               10
#define SRIO_PACKET_TYPE_RSP_WITH_DATA            11
/*
传输完成代码

- 0b000 — Transaction complete, No Errors (Posted/Non-posted)
- 0b001 — Transaction Timeout occurred on Non-posted transaction
- 0b010 — Transaction complete, Packet not sent due to flow control blockade (Xoff)
- 0b011 — Transaction complete, Non-posted response packet (type 8 and 13) contained ERROR status, or response payload length was in error
- 0b100 — Transaction complete, Packet not sent due to unsupported transaction type or invalid programming encoding for one or more LSU register fields
- 0b101 — DMA data transfer error
- 0b110 — “Retry” DOORBELL response received, or Atomic Test-and-swap was not allowed (semaphore in use)
- 0b111 — Transaction completed, No packets sent as the transaction was killed using the CBUSY bit.

*/




#include "srio/srio_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

int				SRIOInitialize(unsigned char devID,  SRIOLoopbackMode  loopbackMode );
void			SRIODoorbellRouteCtl(unsigned char cIntcMode);
void			SRIODoorbellIntcRouteSet(unsigned char cDoorbell, unsigned char cInterruptDestination);
unsigned int	SRIOGetDoorbellStatus();
void            SRIOClearDoorbellIntcStatus(unsigned char cDoorbell);//清除Doorbell0的第Doorbell位, 只能0-15


SRIORWStatus SRIOWriteData(unsigned char cRemoteDevID_8Bit,unsigned int uWrAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen);
SRIORWStatus SRIOReadData(unsigned char cRemoteDevID_8Bit, unsigned int uRdAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen);

// 修改
unsigned char SRIOPacketType(unsigned int uPacketType);
#ifdef __cplusplus
}
#endif
#endif /* SRIO_H_ */
