#ifndef  ___SRIO_AD__H_
#define ___SRIO_AD__H_

#include "tistdtypes.h"
#include "srio_common.h"


/****************************************************************************/
/*                                                                          */
/*              AD卡用户寄存器列表                                                 */
/*                                                                          */
/****************************************************************************/

#define  SRIO_AD_SYS_VERTIME    0x20
#define  SRIO_AD_SYS_VERSION    0x21
#define  SRIO_AD_SAMPLE_MODE  0x24
#define  SRIO_AD_SAMPLE_RATE    0x25
#define  SRIO_AD_SPI_BAUD            0x26
#define  SRIO_AD_SYNC             0x27
//ADC寄存器,  0~F,     具体参考AD4110-1芯片手册中的寄存器说明, P66
#define  SRIO_ADC_REG_BEGIN
#define  SRIO_ADC_REG_STATUS          0x0
#define  SRIO_ADC_REG_MODE             0x1
#define  SRIO_ADC_REG_INTERFACE    0x2
#define  SRIO_ADC_REG_CONFIG          0x3
#define  SRIO_ADC_REG_DATA              0x4
#define  SRIO_ADC_REG_FILTER            0x5
#define  SRIO_ADC_REG_GPIO_CONFIG  0x6
#define  SRIO_ADC_REG_ID                        0x7
#define  SRIO_ADC_REG_OFFSET0            0x8
#define  SRIO_ADC_REG_OFFSET1            0x9
#define  SRIO_ADC_REG_OFFSET2            0xA
#define  SRIO_ADC_REG_OFFSET3            0xB
#define  SRIO_ADC_REG_GAIN0                0xC
#define  SRIO_ADC_REG_GAIN1                0xD
#define  SRIO_ADC_REG_GAIN2                0xE
#define  SRIO_ADC_REG_GAIN3                0xF
#define  SRIO_ADC_REG_END

//AFE寄存器,  0x10 ~ 0x1F,  具体参考AD4110-1芯片手册中的寄存器说明, P58
#define  SRIO_AFE_REG_BEGIN
#define  SRIO_AFE_REG_TOP_STATUS       0x0
#define  SRIO_AFE_REG_CNTRL1                 0x1
#define  SRIO_AFE_REG_RESERVE_2           0x2
#define  SRIO_AFE_REG_CLK_CTRL              0x3
#define  SRIO_AFE_REG_CNTRL2                 0x4
#define  SRIO_AFE_REG_PGA_RTD_CTRL   0x5
#define  SRIO_AFE_REG_ERR_DISABLE       0x6
#define  SRIO_AFE_REG_DETAIL_STATUS  0x7
#define  SRIO_AFE_REG_RESERVE_8           0x8
#define  SRIO_AFE_REG_RESERVE_9           0x9
#define  SRIO_AFE_REG_RESERVE_A          0xA
#define  SRIO_AFE_REG_RESERVE_B           0xB
#define  SRIO_AFE_REG_CAL_DATA            0xC
#define  SRIO_AFE_REG_RSENSE_DATA    0xD
#define  SRIO_AFE_REG_NO_PWR_DEFAULT_SEL           0xE
#define  SRIO_AFE_REG_NO_PWR_DEFAULT_STATUS   0xF

#define   SRIO_AFE_REG_END



//// VME总线空间
//#define   SRIO_VME_AM_REG     0x0020                //VME地址修改码寄存器, [5:0]地址修改码,  [31:6]为保留
//#define   SRIO_VME_DS_REG      0x0028                //VME数据选通寄存器, [1:0]数据选通, [31:2]保留
//#define   SRIO_VME_LW_REG     0x002C                //VME字长选择寄存器, [0]字长选择, [31:1]保留
//

//#define   SRIO_RS485_BASE                     0x30000                   // RS485        0x0017_0000 ~ 0x0017_FFFF
//#define   SRIO_RS485_VERSION1             0x0000                      //版本时间
//#define   SRIO_RS485_VERSION2             0x0008                      //版本号
//#define   SRIO_RS485_BAUD_RATE          0x0100                      //波特率配置寄存器,配置值=系统时钟/波特率, 默认值为125M时钟/9600波特率
//#define   SRIO_RS485_PARITY_EN            0x0108                      //奇偶校验使能
//#define   SRIO_RS485_PARITY_SEL           0x0110                      //奇偶校验选择, 0为奇校验, 1为偶校验
//#define   SRIO_RS485_STOP_WIDTH        0x0118                     //停止位,默认1为停止位,1~3有效 ??
////#define   SRIO_RS485_DELAY_VALUE       0x0110                     //字节间延时, 以10ns为单位, 当延时大于此数据时,则认为一帧数据结束
//#define   SRIO_RS485_TX_FIFO_DIN         0x0200                     //发送数据FIFO数据入,[31:8]保留, [7:0]数据
//#define   SRIO_RS485_TX_FIFO_STATUS   0x0208                     //发送数据FIFO状态,[31:2]保留,[1]FIFO空, [0]FIFO满
//#define   SRIO_RS485_TX_FIFO_CNT        0x0210                      //发送数据FIFO读数据计数,即发送计数
//#define   SRIO_RS485_TX_FIFO_NUM       0x0218                      //发送数据FIFO中数据个数
//#define   SRIO_RS485_RX_FIFO_DOUT      0x0300                     //接收数据FIFO数据出,[31]保留,[30]校验结果,[29:8]保留,[7:0]数据
//#define   SRIO_RS485_RX_FIFO_STATUS   0x0308                     //接收数据FIFO状态,[31:2]保留,[1]FIFO空状态,[0]FIFO满状态
//#define   SRIO_RS485_RX_FIFO_CNT        0x0310                     //接收数据FIFO写数据计数,即接收计数
//#define   SRIO_RS485_RX_FIFO_NUM       0x0318                      //接收数据FIFO中数据个数

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


#if  0

void   SetAuroraPacketHead(Uint32  uRegAddress,  BOOL bRW, Uint32 uLen, Uint32  uChannel, Uint32 uTransactionType, Uint8* pBuffer);
void   SetAuroraPacketData(Uint32  uData,  Uint8  uChannelNO, Uint8* pBuffer);
int     WriteAuroraPacket(Uint32  uWrAddr);
int     WriteAuroraPacketWithBuffer(Uint32   uWrAddr, Uint32  uLen,  Uint8* pBuffer);
int     SendReadAuroraPacketRequest(Uint32   uWrAddr, Uint32  uLen);
int     ReadAuroraPacket(Uint8  uFrameLen,  Uint32  uRdAddr);

//求解整数n的二进制中1的个数.
Uint8  CountOne (int  n);
#endif

// DA的通用读寄存器
int   ReadADReg(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint32  uWriteRequestAddr, AuroraPacket* pPkt );
int   WriteDAReg(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pData, AuroraPacket* pPkt );


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


//读AD 采样值
int   ReadADSampleReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt);
void SetDAReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pBuffer,  AuroraPacket* pPkt  );



#ifdef __cplusplus
}
#endif

#endif
