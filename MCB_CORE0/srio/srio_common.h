#ifndef  __SRIO_COMMON_H_
#define  __SRIO_COMMON_H_

#include "tistdtypes.h"
//SRIO packet status
typedef enum
{
    COMPLETED_NOERRORS = 0,
    TIMEOUT,
    COMPLETED_NOPACKETSSENT0,
    COMPLETED_WITHERRORS,
    COMPLETED_NOPACKETSSENT1,
    DMADATATRANSFERERROR,
    RETRYDOORBELLRESPONSERECEIVED,
    COMPLETED_NOPACKETSSENT2,
    TIMEOUT_NOSEND ,//检测当前状态繁忙或没有空间处理当前事务,未进行发送操作
    TIMEOUT_LSU_FULL,  //LSU_Reg6 :  no shadow register available
    TIMEOUT_LSU_BUSY, //LSU_Reg6 :  Command register are busy with current transfer

} SRIORWStatus;

//SRIO initial status
typedef enum
{
    SRIO_INIT_OK,
    SRIO_INIT_PLL_LOCKFAIL,
    SRIO_INIT_PORT_0_NOTOK,
    SRIO_INIT_PORT_1_NOTOK,
    SRIO_INIT_PORT_2_NOTOK,
    SRIO_INIT_PORT_3_NOTOK,
}SRIOInitStatus;



// loopback 分类:
typedef enum{
    SRIO_NO_LOOPBACK = 0 ,
    SRIO_DIGITAL_LOOPBACK,
    SRIO_SERDES_LOOPBACK,
//    SRIO_EXTERNAL_LINE_LOOPBACK,
//    SRIO_EXTERNAL_FORWARD_BACK,
}SRIOLoopbackMode;


// FPGA器件地址
#define FPGA_DEVICE_ID_8BIT         (0xff)
#define FPGA_DEVICE_ID_16BIT        (0xABCD)

#define CACHE_L1D 1
/****************************************************************************/
/*                                                                          */
/*              对端各功能模块寄存器基地址及其后跟的偏移地址                                                 */
/*                                                                          */
/****************************************************************************/
/*****             基地址空间                                      *******/
#define   SRIO_SYS_REG_BASE    0x0000                 // 系统通用寄存器:  0x0000 0000 ~ 0x0000 0FFF,
#define   SRIO_VME_BUS_BASE  0x10000                // VME总线     0x0001_0000 ~  0x0001_FFFF
#define   SRIO_P2BUS_BASE      0x100000               // P2Bus         0x0010_0000 ~ 0x0010_FFFF
#define   SRIO_SFP1_BASE         0x110000              // SFP1           0x0011_0000  ~  0x0011_FFFF
#define   SRIO_SFP2_BASE         0x120000              // SFP2           0x0012_0000  ~  0x0012_FFFF
#define   SRIO_SFP3_BASE         0x130000              // SFP3           0x0013_0000  ~  0x0013_FFFF
#define   SRIO_SFP4_BASE         0x140000              // SFP4           0x0014_0000  ~  0x0014_FFFF
#define   SRIO_SFP5_BASE         0x150000              // SFP5           0x0015_0000  ~  0x0015_FFFF
#define   SRIO_SFP6_BASE         0x160000              // SFP6          0x0016_0000  ~  0x0016_FFFF
#define   SRIO_SYS485_BASE    0x170000              // 同步485     0x0017_0000 ~  0x0017_FFFF


// 系统通用寄存器空间
#define   SRIO_SYS_VERSION_REG            0x0000                //RW,  复位值64’h2021_0329_0100_0000
#define   SRIO_SYS_SLOT_NUM_REG        0x0008                //R,  槽位号
#define   SRIO_SYS_FRONT_LED_REG       0x0010                //W, 前面板LED, 使用0-3位
#define   SRIO_SYS_SFP_STA_REG             0x0018                //R, 光模块状态寄存器,
#define   SRIO_SYS_P2BUS_MODE_REG   0x0100                //W, P2BUS模式选择寄存器
#define   SRIO_SYS_GPIO_SEL_REG           0x0800                //W, GPIO方向选择寄存器
#define   SRIO_SYS_GPI_REG                      0x0808                //R,   GPI开关量读取寄存器
#define   SRIO_SYS_GPO_REG                    0x0810                //W,   GPO开关量输出寄存器


// VME总线空间
#define   SRIO_VME_AM_REG     0x0020                //VME地址修改码寄存器, [5:0]地址修改码,  [31:6]为保留
#define   SRIO_VME_DS_REG      0x0028                //VME数据选通寄存器, [1:0]数据选通, [31:2]保留
#define   SRIO_VME_LW_REG     0x002C                //VME字长选择寄存器, [0]字长选择, [31:1]保留


#define   SRIO_RS485_BASE                     0x30000                   // RS485        0x0017_0000 ~ 0x0017_FFFF
#define   SRIO_RS485_VERSION1             0x0000                      //版本时间
#define   SRIO_RS485_VERSION2             0x0008                      //版本号
#define   SRIO_RS485_BAUD_RATE          0x0100                      //波特率配置寄存器,配置值=系统时钟/波特率, 默认值为125M时钟/9600波特率
#define   SRIO_RS485_PARITY_EN            0x0108                      //奇偶校验使能
#define   SRIO_RS485_PARITY_SEL           0x0110                      //奇偶校验选择, 0为奇校验, 1为偶校验
#define   SRIO_RS485_STOP_WIDTH        0x0118                     //停止位,默认1为停止位,1~3有效 ??
//#define   SRIO_RS485_DELAY_VALUE       0x0110                     //字节间延时, 以10ns为单位, 当延时大于此数据时,则认为一帧数据结束
#define   SRIO_RS485_TX_FIFO_DIN         0x0200                     //发送数据FIFO数据入,[31:8]保留, [7:0]数据
#define   SRIO_RS485_TX_FIFO_STATUS   0x0208                     //发送数据FIFO状态,[31:2]保留,[1]FIFO空, [0]FIFO满
#define   SRIO_RS485_TX_FIFO_CNT        0x0210                      //发送数据FIFO读数据计数,即发送计数
#define   SRIO_RS485_TX_FIFO_NUM       0x0218                      //发送数据FIFO中数据个数
#define   SRIO_RS485_RX_FIFO_DOUT      0x0300                     //接收数据FIFO数据出,[31]保留,[30]校验结果,[29:8]保留,[7:0]数据
#define   SRIO_RS485_RX_FIFO_STATUS   0x0308                     //接收数据FIFO状态,[31:2]保留,[1]FIFO空状态,[0]FIFO满状态
#define   SRIO_RS485_RX_FIFO_CNT        0x0310                     //接收数据FIFO写数据计数,即接收计数
#define   SRIO_RS485_RX_FIFO_NUM       0x0318                      //接收数据FIFO中数据个数

//#define   SRIO_GPIO_BASE                       0x40000                // GPIO          0x0004_0000 ~ 0x0004_FFFF
//#define   SRIO_GPIO_GFG_REG                 0x0000                 //GPIO配置寄存器
//#define   SRIO_GPIO_I_LV                         0x0010               //GPIO电平输入值, 对应GPIO[15:0]的输入电平值, [31:16]保留
//#define   SRIO_GPIO_0_LV                        0x0018               //GPIO电平输出值, 对应GPIO[15:0]的输出电平值, [31:16]保留
//#define   SRIO_GPIO_DEB_VALUE               0x0020             //去抖值,滤除毛刺的宽度, 63:32保留, 31:0-->滤除宽度




/****************************************************************************/
/*                                                                          */
/*              对端各功能模块寄存器读写的函数                                                 */
/*                                                                          */
/****************************************************************************/
typedef   unsigned char BOOL ;


#ifdef __cplusplus
extern "C" {
#endif


//小端
typedef struct{
    // word 1
    Uint32   reserved;
    // word 0
    Uint32   reg_addr     : 7;
    Uint32   rw_flag            : 1; //读写标志,0写1读
    Uint32   data_reg_len                  : 2; //数据/寄存器长度，单位字节
    Uint32   packet_len      : 6; //包头 + 载荷
    Uint32   channel_sel          : 12;  //12 channel
    Uint32   reserved2      : 2;
    Uint32   transaction_type : 2;//

}AuroraPacketHead;

typedef struct{
    // word 1
    Uint32    reserved;
    // word 0
    Uint32    data                     : 24  ;
    Uint32    channel_NO          : 4  ; //通道编号, 1开始,按顺序
    Uint32    reserved1               : 4  ;
}AuroraPacketBody,  *pAuroraPacketBody ;


//#pragma  STRUCT_ALIGN(transaction_type, 4)
//#pragma  STRUCT_ALIGN(AuroraPacketBody, 4)


#define   Flag_Read    1
#define   Flag_Write   0
#define   Transaction_Type_Data    0x3
#define   Transaction_Type_Syn      0x1
#define   Transaction_Type_Sample   0x2
#define   Transaction_Type_Reserve   0x00

//支持的最大Aurora数据包个数:
#define   PacketBodyMax      32
typedef struct{
    AuroraPacketHead * head;
    AuroraPacketBody * body[PacketBodyMax];

}AuroraPacket;




void   SetAuroraPacketHead(Uint32  uRegAddress,  BOOL bRW, Uint32 uLen, Uint32  uChannel, Uint32 uTransactionType, Uint8* pBuffer);
void   SetAuroraPacketData(Uint32  uData,  Uint8  uChannelNO, Uint8* pBuffer);
//int     WriteAuroraPacket(Uint32  uWrAddr);
int     WriteAuroraPacketWithBuffer(Uint32   uWrAddr, Uint32  uLen,  Uint8* pBuffer);
int     SendReadAuroraPacketRequest(Uint32   uWrAddr, Uint32  uLen);
int     ReadAuroraPacket(Uint8  uFrameLen,  Uint32  uRdAddr);

//求解整数n的二进制中1的个数.
Uint8  CountOne (int  n);

Uint32 Reverse32(Uint32  x);

#if 1
// DA的通用读寄存器
int   ReadADReg(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint32  uWriteRequestAddr, AuroraPacket* pPkt );
int   WriteDAReg(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pData, AuroraPacket* pPkt );


#if  0
//版本时间寄存器
int   ReadDAVersionTimeReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDAVersionTimeReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pVersionTimes,  AuroraPacket* pPkt  );
//版本号寄存器
int   ReadDAVersionNOReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDAVersionNOReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pVersionNO,  AuroraPacket* pPkt  );
//DAC接口逻辑复位寄存器
int   ReadDACLogicResetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDACLogicResetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pReset,  AuroraPacket* pPkt  );
//SPI波特率设置寄存器
int   ReadSPIBaudSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetSPIBaudSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pBaud,  AuroraPacket* pPkt  );
//DAC测试模式使能寄存器
int   ReadDACTestModeEnReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDACTestModeEnReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pModeEn,  AuroraPacket* pPkt  );
//DAC测试波形选择寄存器
int   ReadDACTestWaveSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDACTestWaveSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pWaveSel,  AuroraPacket* pPkt  );
//DAC测试数据更新速率寄存器
int   ReadDACTestUpdateVReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDACTestUpdateVReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pUpdateV,  AuroraPacket* pPkt  );
//DAC 测试波形频率设置寄存器
int   ReadDACTestFreqSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDACTestFreqSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pFreqSet,  AuroraPacket* pPkt  );
//DAC测试波形幅度选择寄存器
int   ReadDACTestRangeSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDACTestRangeSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pRangeSelt,  AuroraPacket* pPkt  );

#endif

//读AD 采样值
int   ReadADSampleReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDAReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pBuffer,  AuroraPacket* pPkt  );
#endif


#ifdef __cplusplus
}
#endif

#endif
