#ifndef  __SRIO_DA_H_
#define  __SRIO_DA_H_

#include "tistdtypes.h"
#include "srio_common.h"

/****************************************************************************/
/*                                                                          */
/*              DA卡用户寄存器列表                                                 */
/*                                                                          */
/****************************************************************************/
#define  SRIO_DA_SYS_VERTIME        0x20
#define  SRIO_DA_SYS_VERSION          0x21
//#define  SRIO_DAC_INTERFACE_RESET_REG    0x22
#define  SRIO_DA_SPI_BAUD        0x26
#define  SRIO_DA_TEST_EN           0x28
#define  SRIO_DA_WAVE_SEL       0x29
#define  SRIO_DA_PERIOD             0x2A
#define  SRIO_DA_FREQ                  0x2B
#define  SRIO_DA_AMP                   0x2C
// DAC 寄存器,  0~F,  具体参考AD5791芯片手册中的寄存器说明
#define    SRIO_DAC_REG_BEGIN
#define    SRIO_DAC_REG_WRITE  0x01   //Write to the DAC register
#define    SRIO_DAC_REG_END


//void   SetAuroraPacketHead(Uint32  uRegAddress,  BOOL bRW, Uint32 uLen, Uint32  uChannel, Uint32 uTransactionType, Uint8* pBuffer);
//void   SetAuroraPacketData(Uint32  uData,  Uint8  uChannelNO, Uint8* pBuffer);
//
//int     WriteAuroraPacketWithBuffer(Uint32   uWrAddr, Uint32  uLen,  Uint8* pBuffer);
//int     SendReadAuroraPacketRequest(Uint32   uWrAddr, Uint32  uLen);
//int     ReadAuroraPacket(Uint8  uFrameLen,  Uint32  uRdAddr);

//求解整数n的二进制中1的个数.
//Uint8  CountOne (int  n);


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
