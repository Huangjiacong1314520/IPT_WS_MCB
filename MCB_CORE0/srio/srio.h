/****************************************************************************/
/*                                                                          */
/*              广州创龙电子科技有限公司                                    */
/*                                                                          */
/*              Copyright 2015 Tronlong All rights reserved                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*              SRIO 设备抽象层函数                                         */
/*                                                                          */
/*              2015年11月23日                                              */
/*                                                                          */
/****************************************************************************/
#ifndef __SRIO_H__
#define __SRIO_H__

/****************************************************************************/
/*                                                                          */
/*              相关定义                                                    */
/*                                                                          */
/****************************************************************************/




#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/tistdtypes.h>
#include <ti/csl/csl_cpIntcAux.h>

#include "srio_cfg.h"



// DSP 器件地址
//#define DSP_DEVICE_ID_8BIT          (0x12)
//#define DSP_DEVICE_ID_16BIT         (0x5678)

#define DEVICE_VENDOR_ID            0x30
#define DEVICE_REVISION             0x0

#define DEVICE_ASSEMBLY_ID          0x0
#define DEVICE_ASSEMBLY_VENDOR_ID   0x30
#define DEVICE_ASSEMBLY_REVISION    0x0
#define DEVICE_ASSEMBLY_INFO        0x0100



// 使用8位器件地址还是16位的器件地址
// 0：8位器件地址
// 1：16位器件地址
#define ID_SMALL_LARGE              (0)


// SRIO 使能禁用
#define SRIO_Enable          1
#define SRIO_Disable         0

// SRIO 端口
#define SRIO_Port0           0
#define SRIO_Port1           1
#define SRIO_Port2           2
#define SRIO_Port3           3



// SRIO ID
#define SRIO_ID_8Bit         0
#define SRIO_ID_16Bit        1

// SRIO 工作模式
#define SRIO_Loopback        1
#define SRIO_Normal          0

// SRIO 数据包类型
#define SRIO_NRead           0x24 // 读
#define SRIO_NWrite          0x54 // 写
#define SRIO_NWriteResponse  0x55 // 写 要求接收端相应
#define SRIO_StreamWrite     0x60 // 流写 数据长度必须为 8 字节整数倍
#define SRIO_MaintenanceRead   0x80
#define SRIO_MaintenanceWrite  0x81
#define SRIO_MaintenanceReadRsp  0x82
#define SRIO_MaintenanceWriteRsp 0x83
#define SRIO_MaintenancePortWrite  0x84
#define SRIO_DoorBell        0xA0 // DoorBell
#define SRIO_RespNoData      0xD0
#define SRIO_RespWithData    0xD8
// SRIO DoorBell 中断
#define SRIO_DoorBell_Dedicated_INT 0 // DoorBell 中断路由到专用中断
#define SRIO_DoorBell_General_INT   1 // DoorBell 中断路由到 16 个通用中断

#define SRIO_DoorBell0       0    // DoorBell 中断组 0
#define SRIO_DoorBell1       1    // DoorBell 中断组 1
#define SRIO_DoorBell2       2    // DoorBell 中断组 2
#define SRIO_DoorBell3       3    // DoorBell 中断组 3

#define SRIO_DoorBellInt0    0    // DoorBell 中断 0
#define SRIO_DoorBellInt1    1    // DoorBell 中断 1
#define SRIO_DoorBellInt2    2    // DoorBell 中断 2
#define SRIO_DoorBellInt3    3    // DoorBell 中断 3
#define SRIO_DoorBellInt4    4    // DoorBell 中断 4
#define SRIO_DoorBellInt5    5    // DoorBell 中断 5
#define SRIO_DoorBellInt6    6    // DoorBell 中断 6
#define SRIO_DoorBellInt7    7    // DoorBell 中断 7
#define SRIO_DoorBellInt8    8    // DoorBell 中断 8
#define SRIO_DoorBellInt9    9    // DoorBell 中断 9
#define SRIO_DoorBellInt10   10   // DoorBell 中断 10
#define SRIO_DoorBellInt11   11   // DoorBell 中断 11
#define SRIO_DoorBellInt12   12   // DoorBell 中断 12
#define SRIO_DoorBellInt13   13   // DoorBell 中断 13
#define SRIO_DoorBellInt14   14   // DoorBell 中断 14
#define SRIO_DoorBellInt15   15   // DoorBell 中断 15

#define SRIO_IntDst_0_16     0    // 中断目标  0 / 16
#define SRIO_IntDst_1_17     1    // 中断目标  1 / 17
#define SRIO_IntDst_2_18     2    // 中断目标  2 / 18
#define SRIO_IntDst_3_19     3    // 中断目标  3 / 19
#define SRIO_IntDst_4_20     4    // 中断目标  4 / 20
#define SRIO_IntDst_5_21     5    // 中断目标  5 / 21
#define SRIO_IntDst_6_22     6    // 中断目标  6 / 22
#define SRIO_IntDst_7_23     7    // 中断目标  7 / 23
#define SRIO_IntDst_8        8    // 中断目标  8 / 保留
#define SRIO_IntDst_9        9    // 中断目标  9 / 保留
#define SRIO_IntDst_10       10   // 中断目标 10 / 保留
#define SRIO_IntDst_11       11   // 中断目标 11 / 保留
#define SRIO_IntDst_12       12   // 中断目标 12 / 保留
#define SRIO_IntDst_13       13   // 中断目标 13 / 保留
#define SRIO_IntDst_14       14   // 中断目标 14 / 保留
#define SRIO_IntDst_15       15   // 中断目标 15 / 保留


#define SRIO_DOWN_LSU_INDEX     (1)     //发送数据到ＦＰＧＡ使用的ＬＳＵ
#define SRIO_UP_LSU_INDEX       (2)     //读取ＦＰＧＡ数据使用的ＬＳＵ

/*convert local address to global address for DMA on multi-core DSP*/
#define GLOBAL_ADDR(addr) ((Uint32)addr<0x1000000?\
                        (Uint32)addr+(0x10000000+DNUM*0x1000000):\
                        (Uint32)addr)

#define DEVICE_TYPE_ID              0x009D      //创龙例子中与FPGA通信中用到这个来作为SRIODeviceInfoSet的第一个参数, 且只此一处有用到, 和参考文档里一样
// LSU 配置
typedef struct
{
	// 地址
	struct
	{
		// RapidIO 地址
		struct
		{
			unsigned int MSB;//传输目标地址扩展高32bit域，即如果地址位数比32bit多时，会用到它，一般为0
			unsigned int LSB;//传输目标地址低32bit，一般情况的目标地址就是这个
		} RapidIOAddress;

		// DSP 地址
		unsigned int DSPAddress;//本地DSP的地址
	} Address;

	// ID
	struct
	{
		// 目标设备 ID, LSU传输目标的DEVICEID
		unsigned short Dest;
		// 源设备 ID 索引
		unsigned short Src;
		// 设备 ID 大小, DEVICEID使用8bit还是16bit格式的。1表示使用16bit格式。
		unsigned char Size;
		// 端口 ID
		unsigned char Port;
	} ID;

	struct
	{
		// DoorBell 使能: 是否需要在传输完成后发送doorbell，0表示不需要
		unsigned char Enable;
		// DoorBell 信息, 是为类型10的包（doorbell包）准备的
		unsigned short Info;
	} DoorBell;


	// 传输数据大小
	unsigned int ByteCount;
	// LSU传输的优先级，该属性只有在多个LSU传输时才有意义
	unsigned char Priority;
	// Xambs, 指明扩展地址最高位，一般情况下不需要
	unsigned char Xambs;
	// Suppress Good 中断, 是否需要“压制”传输完成后的中断，0表示不压制
	unsigned char SupGoodInt;
	// 中断请求使能,  传输完成后是否需要产生中断，1表示需要
	unsigned char IntReq;
	// Hop Count, 是为类型8的维护包准备的
	unsigned short HopCount;
	// 数据包类型
	unsigned char PacketType;

	struct
	{
		// 完成代码（CC）是否属于当前传输
		unsigned char Context;
		// 传输索引（用于识别完成代码（CC））
		unsigned char TransactionIndex;
	} Status;

/*
	// PrivID
	unsigned char PrivID;
	// 清除 Busy 位
	unsigned char CBusy;
	// SrcID MAP, 确定在该LSU中使用哪个srcID映射寄存器
	unsigned char SrcIDMAP;
	// Restart
	unsigned char Restart;
	// Flush
	unsigned char Flush;
*/
} SRIOLSUConfig;

// LSU 状态
typedef struct
{
	// 忙状态
	unsigned char Busy;
	// 影子（Shadow Register）寄存器空闲状态
	unsigned short Full;
	// LSU 上下文状态位
	unsigned char LCB;
	// LSU 传输索引
	unsigned char LTID;
} SRIOLSUStatus;

/*
// PE 特性配置
typedef struct
{
	unsigned char ExtendedAddressingSupport;                // 扩展地址支持
	unsigned char ExtendedFeatures;                         // PE 支持扩展特性
	unsigned char CommonTransportLargeSystemSupport;        // 8 位 ID 或 8 位 / 16 位 ID 支持
	unsigned char CRFSupport;                               // PE 支持 Critical Request Flow (CRF) 功能
	unsigned char RetransmitSuppressionSupport;             // PE 支持禁止 CRC 校验错误恢复
	unsigned char FlowControlSupport;                       // PE 支持流控机制
	unsigned char StandardRoutingTableSonfigurationSupport; // 标准路由表配置支持
	unsigned char Switch;                                   // PE 可以桥接到其它 RapidIO 接口
	unsigned char Processor;                                // PE 具有本地处理器或类似设备
	unsigned char Memory;                                   // PE 具有可寻址的本地物理地址空间
	unsigned char Bridge;                                   // PE 可以桥接到其它接口
} SRIOPEFeatureConfig;
*/

/* SRIO 1x 2x 4x path mode configuration:
    In a configuration 1, the following operating mode is available:
     Mode 0: One independent 1x port in lane A

    In Configuration 2, a maximum of 2 active ports and 2 lanes per path are supported as follows:
     Mode 0: Two independent 1x ports in lanes A and B
     Mode 1: One independent 2x port in lanes {A, B}

    In Configuration 4, a maximum of 4 active ports and 4 lanes per path are supported as follows:
     Mode 0: Four independent 1x ports in lanes A, B, C, and D
     Mode 1: One independent 2x port in lanes {A,B}, and two independent 1x ports in lanes C and D
     Mode 2: Two independent 1x ports in lanes A and B, and one independent 2x port in lanes {C,D}
     Mode 3: Two independent 2x ports, occupying lanes {A,B} and {C,D} respectively
     Mode 4: One independent 4x port in lanes {A,B,C,D}*/

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




/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
//一次最大传1MB,
void SRIODirectIOTransfer(unsigned char LSU, SRIOLSUConfig *config);
//影子寄存器的完成码状态
SRIORWStatus SRIOLSUStatusGet(unsigned char LSU, SRIOLSUConfig *config);

// 配置 SRIO SerDes 时钟
void SRIOSerDesPLLSet(unsigned int config);
// 等待 SRIO SerDes 锁定 ! (X &0x01)
unsigned int SRIOSerDesPLLStatus();
// 配置 SRIO SerDes 发送 / 接收
void SRIOSerDesTxSet(unsigned char index, unsigned int config);
void SRIOSerDesRxSet(unsigned char index, unsigned int config);

//BLOCK0 -MMRS, BLOCK1 - LSU, BLOCK2 -MAU, BLOCK3 -TXU, BLOCK4 -RXU, BLOCK5- Port0, BLOCK6 - Port1, BLOCK7-Port2, BLOCK8 -Port3
void SRIOGlobalEnable();
void SRIOGlobalDisable();

void SRIOBlockEnable(unsigned char block);
void SRIOBlockDisable(unsigned char block);
unsigned int IsSRIOBlockEnabled(unsigned char block);
// 设置 Boot Complete 为 0 以便可以修改 SRIO 所有寄存器包括只读（Read Only）寄存器
void SRIOBootCompleteSet(unsigned char val);
void SRIOBytesSWAPSet(unsigned int val);
// 配置 SRIO Lane 工作模式
void SRIOModeSet(unsigned char Lane, unsigned char val);
// 使能自动优先级提升
void SRIOAutomaticPriorityPromotionEnable();
void SRIOAutomaticPriorityPromotionDisable();
// 设置 SRIO VBUS 预分频
void SRIOPrescalarSelectSet(unsigned char val);

void SRIOLSUPendingInterruptClear(unsigned char LSU, unsigned char id);
void SRIODoorBellInterruptRoutingControl(unsigned char value);
void SRIODoorBellInterruptConditionRoutingSet(unsigned char DoorBellNo, unsigned char DoorBell, unsigned char InterruptDestination);
unsigned int SRIODoorBellInterruptGet(unsigned char DoorBellNo);
void SRIODoorBellInterruptClear(unsigned char DoorBellNo, unsigned char DoorBell);
void SRIODoorBellInterruptGenerate(unsigned char Port, unsigned char LSU, unsigned char IDSize, unsigned short DestinationID, unsigned short Info);
void SRIOErrRstEventInterruptClear();

void SRIODeviceInfoSet(unsigned short ID, unsigned short VendorID, unsigned int Revision);
void SRIOAssemblyInfoSet(unsigned short ID, unsigned short VendorID, unsigned short Revision, unsigned short ExtendedFeaturesPtr);
unsigned long long SRIODeviceInfoGet();
unsigned long long SRIOAssemblyInfoGet();

// 设置 SRIO BASE_ID
void SRIODeviceIDSet(unsigned char BaseID, unsigned short VendorIDEntity);
//The host base device ID lock CSR contains the base device ID value for the processing element in the system that is responsible for initializing
//this processing element.
void SRIOHostDeviceIDSet(unsigned short ID);//Host Base Device ID Lock CSR
unsigned short SRIOHostDeviceIDGet();
void SRIOComponentTagSet(unsigned int value);
unsigned int SRIOComponentTagGet();
// 设置端口写 ID
void SRIOPortWriteTargetDeviceID(unsigned char MSB, unsigned char LSB, unsigned char cfg);
// PE 特性配置
void SRIOProcessingElementFeaturesSet(unsigned int cfg);
void SRIOSourceOperationsSet(unsigned int cfg);
void SRIODestinationOperationsSet(unsigned int cfg);
// 配置 TLM 基本路由信息
void SRIOTLMPortBaseRoutingSet(unsigned char Port, unsigned char BRR, unsigned char Enable, unsigned char RouteMaintenanceRequestToLLM, unsigned char Private);
void SRIOTLMPortBaseRoutingPatternMatchSet(unsigned char Port, unsigned char BRR, unsigned short Pattern, unsigned short Match);
// 配置 PLM 端口 Silence Timer, PLM是路径控制寄存器
void SRIOPLMPortSilenceTimerSet(unsigned char Port, unsigned char cfg);
// 配置 PLM 端口 Discovery Timer
void SRIOPLMPortDiscoveryTimerSet(unsigned char Port, unsigned char cfg);
// 设置 LLM Port IP 预分频
void SRIOServerClockPortIPPrescalar(unsigned char Div);
void SRIOPLMPortForeceReLink(unsigned char Port);

void SRIOInputPortEnable(unsigned char Port);
void SRIOOutputPortEnable(unsigned char Port);

void SRIOSetSPCtl2(unsigned char Port ,unsigned int cfg);
// 配置端口连接超时
void SRIOPortLinkTimeoutSet(unsigned int val);
// 配置端口 Write Reception Capture
void SRIOPortWriteRxCapture(unsigned char Port, unsigned int cfg);
// 端口 Master 使能
void SRIOPortGeneralSet(unsigned char Host, unsigned char Master, unsigned char Discovered);
// 清除 Sticky Register 位
void SRIORegisterResetControlClear();
// 设置数据流最大传输单元MTU
void SRIODataDtreamingLogicalLayerControl(unsigned char cMTU);
// 配置端口路由模式
void SRIOPLMPathModeControl(unsigned char cPort, unsigned char cMode);
// 使能外设
void SRIOPeripheralEnable();
void SRIOPeripheralDisable();
// 检查端口是否就绪,端口的初始化配置主要是端口的接收时钟窗对齐以及端口宽度的确认过程；大部分情况端口宽度通常是固定配置的，只有接收时钟窗需要调整。
//https://www.cnblogs.com/fpga/archive/2013/03/06/2947194.html
unsigned char SRIOPortOKCheck(unsigned char cPort);

unsigned int SRIOLSUFullCheck(unsigned char cLSU);
unsigned int SRIOLSUBusyCheck(unsigned char cLSU);
unsigned char  SRIOLSUFullAndBusyCheck(unsigned char cLSU);
//void SRIOLSUFlush(unsigned char LSU);
//void SRIOLSURestart(unsigned char LSU);
void SRIOFlowControl(unsigned char index, unsigned short destID);


SRIOInitStatus   SRIOInit(unsigned char cMode, unsigned char cDSPID, SRIOLoopbackMode  loopbackMode);
int SRIODoorbellIniterruptCfg(unsigned char cDoorBellNo);
void SRIOClearAllInterrupt(unsigned char cDoorBellNo,unsigned int uValue);

void SRIO_ReadSysVerInfoByDoorbell(unsigned char* pData);
void SRIO_ReadUS422InfoByDoorbell(unsigned char* pData);
void SRIO_ReadHDSYNCInfoByDoorbell(unsigned char* pData);
void SRIO_ReadRS422InfoByDoorbell(unsigned char* pData);
void SRIO_ReadCan0InfoByDoorbell(unsigned char* pData);
void SRIO_ReadCan1InfoByDoorbell(unsigned char* pData);
void SRIO_ReadCamLinkInfoByDoorbell(unsigned char* pData);



int SRIO_ReadDataFromFpga(unsigned char cLSU, unsigned char *pData, int nLen);
int SRIO_SendDataToFpga(unsigned char cLSU, unsigned char *pData, int nLen, int nCheckRamState);



int SRIO_WriteData(unsigned char cLSU, unsigned int uWrAddr, unsigned char *pData,int nLen);
int SRIO_ReadData(unsigned char cLSU, unsigned int uRdAddr, unsigned char *pData,int nLen);
int SRIO_BlockRamTest(void);
//SRIO 读写寄存器，默认读取长度8字节
int SRIO_ReadRegister32(Uint32 uRegAddr, Uint8 *pData);
int SRIO_WriteRegister32(Uint32 uRegAddr, Uint8 *pData);

void  delay_ms(Uint32 ms);
void  delay_us(Uint32 us);

//下面函数转自K1_STK_v1.1 里KeyStone_SRIO_对应函数
//SRIO maintenance operation
int   SRIO_Maintenance(Uint32 uPort, Uint8 cLSU,  Uint32 uDestID, Uint32 uOffSet, Uint32 uLocalAddress,  Uint8 cPacketType);
/*
 * ACK ID 是用来确认通信是否正常，inbound ID会和本地存储的ID比较，匹配的话才接收。outbound ID是发送时需要填进去的值，每
 * 次成功发送接收后，对应的outbound和inbound 都需要加1. 值是在0~31之间变化。
 * ACK id中有inbound,outbound,outstand三种ID.
 * 使用前先检查link建立起来没有, 如果已经建立起来了, match ackid可以跳过.
 * 在链路伙伴之间发送数据或维护包之前，ackid必须同步。如果两个链接伙伴都结束了重置，那么ackid将已经对齐，
 * 不需要额外的步骤。如果没有，则必须立即采取额外的步骤来对齐ackid。
 * 主要目的是通过发送restart-from-error命令使本地的inbound ACKID和远端的outbound ACKID匹配，主要用于error recovery或
 * 者远端和本地reset和link request顺序不正确时，比如1）Chip1 rest 2） Chip2 rest 3） Chip1 link init 4） Chip2 link init，这种情
 * 况就不需要调用这个函数，如果是Chip1 rest, Chip1 link init《这个时候Chip2还在运行，而此时chip2发出的OUTBOUND ACKID和
 * chip1的link request 所期待的inbound ACKID并不匹配》 Chip2 reset Chip2 link init就需要用这个函数进行ACKID的匹配。所以为增强系统的鲁棒性，建议加上.
 */
void SRIO_MatchAckID(Uint32 uLocalPort,  Uint32 uDestID,  Uint32 uRemotePort);//Make the ACK_ID of both sides match



/*
 * for directIO test, dspAddress is source address in memory; for message test, dspAddress is source queue number
* for directIO test, LSB is destination address in memory;  for message test, LSB is dest device ID, 不同的dest ID map to不同的 flow to different memory in this test
* */
void SRIO_InitLsuCfg(SRIOLSUConfig*cfg, Uint32 dspAddress, Uint32 LSB, Uint8 packetType,Uint32 bytesCount,  Uint16 dstID, Uint8 port);

SRIORWStatus SRIO_SendDoorBell(Uint8 uiPort, Uint8 uiLSU_No,  Uint16 uiDestID, Uint16 uiDoorBellInfo);
SRIORWStatus SRIO_DirectIO(Uint32 uiLocalAddress, Uint32 uiRemoteAddress,  Uint32 uiDestID, Uint32 uiByteCount, Uint8 uiPort, Uint8 uiLSU_No,
    Uint8 packetType , Uint16 uDoorbellInfo);


void SRIO_PreTransferSet(Uint8 uiPort);

//端口link-status
Bool SRIO_LinkStatusCheck(Uint8 uiPort);
void SRIOClearAllIntcStatus();//清除Doorbell0-3的所有中断位, 清除LSU0-1的LSUx_ICSR
void  SRIOSoftReset();
#endif
