/****************************************************************************/
/*                                                                          */
/*              广州创龙电子科技有限公司                                    */
/*                                                                          */
/*              Copyright (C) 2015 Tronlong Electronic Technology Co.,Ltd   */
/*                                                                          */
/****************************************************************************/
/*
 *   - 希望缄默(bin wang)
 *   - bin@tronlong.com
 *   - DSP C665x 项目组
 *
 *   官网 www.tronlong.com
 *   论坛 51dsp.net
 *
 *   2015年11月24日
 *
 */

#ifndef _HW_SRIO_H_
#define _HW_SRIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_types.h"

/****************************************************************************/
/*                                                                          */
/*              SRIO 寄存器                                                 */
/*                                                                          */
/****************************************************************************/
// 外设配置
#define SRIO_PeripheralControl                (0x0004)

// 全局与块使能
#define SRIO_GBL_EN                           (0x0024)
#define SRIO_GBL_EN_STAT                      (0x0028)
#define SRIO_BLK_EN(n)                        (0x002C + (n * 8))
#define SRIO_BLK_EN_STAT(n)                   (0x0030 + (n * 8))

// 外设配置
#define SRIO_PER_SET_CNTL                     (0x0014)
#define SRIO_PER_SET_CNTL1                    (0x0018)

// 加载存储单元（LSU）
#define SRIO_LSU_REG0(n)                      (0x0D00 + (n * 28))
#define SRIO_LSU_REG1(n)                      (0x0D04 + (n * 28))
#define SRIO_LSU_REG2(n)                      (0x0D08 + (n * 28))
#define SRIO_LSU_REG3(n)                      (0x0D0C + (n * 28))
#define SRIO_LSU_REG4(n)                      (0x0D10 + (n * 28))
#define SRIO_LSU_REG5(n)                      (0x0D14 + (n * 28))
#define SRIO_LSU_REG6(n)                      (0x0D18 + (n * 28))

#define SRIO_LSU_Status(n)                    (0x0DE8 + (n * 4))

// 中断
#define SRIO_LSU0_ICSR                        (0x01C0)
#define SRIO_LSU0_ICCR                        (0x01C8)
#define SRIO_LSU1_ICSR                        (0x01D0)
#define SRIO_LSU1_ICCR                        (0x01D8)
#define SRIO_DoorBell_ICSR(n)                 (0x0180 + (n * 16))
#define SRIO_DoorBell_ICCR(n)                 (0x0188 + (n * 16))
#define SRIO_DoorBell_ICRR1(n)                (0x0200 + (n * 12))
#define SRIO_DoorBell_ICRR2(n)                (0x0204 + (n * 12))
#define SRIO_ERR_RST_EVNT_ICCR            (0x01E8)
#define SRIO_InterruptControl                 (0x0264)


#define SRIO_INTDST_Rate_Cnt(n)				  (0x02d0 + (n*4))

// 设备 ID, DEV_ID
#define SRIO_DeviceID                         (0xB000)
#define SRIO_DeviceINFO                       (0xB004)
#define SRIO_AssemblyID                       (0xB008)
#define SRIO_AssemblyINFO                     (0xB00C)

#define SRIO_HOSTDeviceID                     (0xB068)
#define SRIO_ComponentTag                     (0xB06C)

// 处理单元特性（Processing Element Features）
#define SRIO_PEFeature                        (0xB010)
//Switch Port Information CAR
#define SRIO_SWPort                            (0xB014)
// 源操作 CAR（Source Operations CAR）
#define SRIO_SourceOP                         (0xB018)
#define SRIO_DestinationOP                    (0xB01C)
// 设备 ID CSR, BASE_ID
#define SRIO_BaseDeviceID                     (0xB060)
//1x/4x LP-Serial Port Maintenance  Block Header
#define SRIO_SP_MB_HEAD                      (0xB100)

//#define SRIO_SPn_ACKID_STAT(n)                (0xB148 + (n * 0x20))
#define SRIO_SPn_ERR_STAT(n)                  (0xB158 + (n * 0x20))


#define SRIO_SPCTL2(n)                        (0xB154 + n*32)
// 端口控制
#define SRIO_PortGeneralControl               (0xB13C)
#define SRIO_PortControl(n)                   (0xB15C + n * 32)
// 端口超时
#define SRIO_PortLinkTimeout                  (0xB120)
//端口响应超时
#define  SRIO_SP_RT_CTL                           (0xB124)
//Port n Link Maintenance Request CSR
#define SRIO_SPn_LM_REQ(n)                     (0xB140 + n*0x20)
//Port n Link Maintenance Response CSR
#define SRIO_SPn_LM_RESP(n)                    (0xB144 + n*0x20)
//Port n Local AckID Status CSR
#define SRIO_SPn_ACKID_STAT(n)               (0xB148 + n*0x20)
// 端口 Write Reception Capture
#define SRIO_PortWriteRxCapture(n)            (0x1BA20 + n * 4)
// Port Write 目标 ID
#define SRIO_PortWriteTargetDeviceID          (0xC028)
//Port Write Enable   EM_DEV_PW_EN
#define SRIO_PortWriteEnable                        (0x1B934)
// 数据流逻辑层控制
#define SRIO_DataDtreamingLogicalLayerControl (0xB048)
// 端口错误状态
#define SRIO_PortErrorStatus(n)               (0xB158 + n * 32)
//流控制寄存器
#define  SRIO_FlowCntl(n)                         (0x0E50 + n*4)


//错误管理寄存器
#define SRIO_ERR_RST_EVNT_ICSR           (0x01E0)
#define SRIO_ERR_RST_EVNT_ICCR           (0x01E8)
#define SRIO_ERR_RPT_BH                       (0xC000)       //Error Reporting Block Header
#define SRIO_ERR_DET                             (0xC008)      //Logical/Transport Layer Error Detect CSR
#define SRIO_ERR_EN                               (0xC00C)      //Logical/Transport Layer Error Enable CSR
#define SRIO_H_ADDR_CAPT                    (0xC010)     //Logical/Transport Layer High Address Capture CSR
#define SRIO_ADDR_CAPT                       (0xC014)      //Logical/Transport Layer Address Capture CSR
#define SRIO_ID_CAPT                             (0xC018)      //Logical/Transport Layer Device ID Capture CSR
#define SRIO_CTRL_CAPT                         (0xC01C)     //Logical/Transport Layer Control Capture CSR

#define SRIO_SPn_ERR_DET(n)                   (0xC040+n*0x40)    //Port n Error Detect CSR(physical layer (port) errors)
#define SRIO_SPn_RATE_EN(n)                   (0xC044+n*0x40)    //Port n Error Enable CSR
#define SRIO_SPn_ERR_ATTR_CAPT(n)          (0xC048 + (n * 0x40))
#define SRIO_SPn_ERR_CAPT_0(n)                (0xC04C + (n * 0x40))
#define SRIO_SPn_ERR_CAPT_1(n)                (0xC050 + (n * 0x40))
#define SRIO_SPn_ERR_CAPT_2(n)                (0xC054 + (n * 0x40))
#define SRIO_SPn_ERR_CAPT_3(n)                (0xC058 + (n * 0x40))
#define SRIO_SPn_ERR_THRESH(n)                (0xC06C + n * 0x40)  //Port n Error Rate Threshold CSR
#define SRIO_SPn_ERR_RATE(n)                     (0xC068 + (n * 0x40))  //Port n Error Rate CSR



// TLM
#define SRIO_TLM_SP_BRR_Control(n, m)         (0x1B3A0 + n * 128 + (m * 16))
#define SRIO_TLM_SP_BRR_Pattern_Match(n, m)   (0x1B3A4 + n * 128 + (m * 16))

// PLM
#define SRIO_PLM_SP_DiscoveryTimer(n)         (0x1B0B4 + n * 128)
#define SRIO_PLM_SP_SilenceTimer(n)           (0x1B0B8 + n * 128)
#define SRIO_PLM_SP_PathModeControl(n)        (0x1B0B0 + n * 128)
#define SRIO_PLM_SP_LONG_CS_TX1(n)          (0x1B0E0 + n*0x80)
#define SRIO_PLM_SP_VMIN_EXP(n)               (0x1B0BC + n*0x80)
#define SRIO_PLM_SP_IMP_SPEC_Ctrl(n)          (0x1B080 + n * 128)
// 服务时钟分频
#define SRIO_ServerClockPortIPPrescalar       (0x1BD30)
// 寄存器复位控制
#define SRIO_RegisterResetControl             (0x1BD34)



#ifdef __cplusplus
}
#endif

#endif
