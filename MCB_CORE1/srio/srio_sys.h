#ifndef  __SRIO_SYS_H_
#define  __SRIO_SYS_H_

#include "srio_common.h"



/****************************************************************************/
/*                                                                          */
/*              对端各功能模块寄存器基地址及其后跟的偏移地址                                                 */
/*                                                                          */
/****************************************************************************/
/*****             基地址空间                                      *******/
//#define   SRIO_SYS_REG_BASE    0x0000                 // 系统通用寄存器:  0x0000 0000 ~ 0x0000 0FFF,
//#define   SRIO_VME_BUS_BASE  0x10000                // VME总线     0x0001_0000 ~  0x0001_FFFF
//#define   SRIO_P2BUS_BASE      0x100000               // P2Bus         0x0010_0000 ~ 0x0010_FFFF
//#define   SRIO_SFP1_BASE         0x110000              // SFP1           0x0011_0000  ~  0x0011_FFFF
//#define   SRIO_SFP2_BASE         0x120000              // SFP2           0x0012_0000  ~  0x0012_FFFF
//#define   SRIO_SFP3_BASE         0x130000              // SFP3           0x0013_0000  ~  0x0013_FFFF
//#define   SRIO_SFP4_BASE         0x140000              // SFP4           0x0014_0000  ~  0x0014_FFFF
//#define   SRIO_SFP5_BASE         0x150000              // SFP5           0x0015_0000  ~  0x0015_FFFF
//#define   SRIO_SFP6_BASE         0x160000              // SFP6          0x0016_0000  ~  0x0016_FFFF
//#define   SRIO_SYS485_BASE    0x170000              // 同步485     0x0017_0000 ~  0x0017_FFFF


// 系统通用寄存器空间
#define   SRIO_SYS_VERSION_REG            0x0000                //RW,  复位值64’h2021_0329_0100_0000
#define   SRIO_SYS_SLOT_NUM_REG        0x0008                //R,  槽位号
#define   SRIO_SYS_FRONT_LED_REG       0x0010                //W, 前面板LED, 使用0-3位
#define   SRIO_SYS_SFP_STA_REG             0x0018                //R, 光模块状态寄存器,
#define   SRIO_SYS_P2BUS_MODE_REG   0x0100                //W, P2BUS模式选择寄存器
#define   SRIO_SYS_GPIO_SEL_REG           0x0800                //W, GPIO方向选择寄存器
#define   SRIO_SYS_GPI_REG                      0x0808                //R,   GPI开关量读取寄存器
#define   SRIO_SYS_GPO_REG                    0x0810                //W,   GPO开关量输出寄存器



/****************************************************************************/
/*                                                                          */
/*              对端各功能模块寄存器读写的函数                                                 */
/*                                                                          */
/****************************************************************************/
typedef   unsigned char BOOL ;


#ifdef __cplusplus
extern "C" {
#endif

// 系统通用寄存器空间的操作:

int  ReadSYSVersionReg(unsigned char* pVersionTime,  unsigned char* pVersionNum);
int  WriteSYSVersionReg(unsigned char*  uVersionTime, unsigned char* uVersionNum);

int  ReadSYSSlotNumReg(unsigned int*  uNO);

int  WriteSYSFrontLEDReg(unsigned short  uLED1, unsigned short  uLED2);

int  ReadSYSSFPSTAReg(unsigned char* pSFPLinkStatus );

int   WriteSYSP2BUSModeReg(BOOL  bMasterMode); //0 从模式,  1 主模式

int   WriteSYSGPIOSelReg(BOOL  bIn);  //0 输出, 1输入

int    ReadSYSGPIReg(unsigned short*  pGPI) ;

int    WriteSYSGPOReg(unsigned short  uGPO);





#ifdef __cplusplus
}
#endif

#endif
