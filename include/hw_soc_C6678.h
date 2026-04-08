/****************************************************************************/
/*                                                                          */
/* 广州创龙电子科技有限公司                                                 */
/*                                                                          */
/* Copyright (C) 2014-2016 Guangzhou Tronlong Electronic Technology Co.,Ltd */
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
 */

#ifndef _SOC_C665X_H_
#define _SOC_C665X_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_types.h"

/****************************************************************************/
/*                                                                          */
/*              类型定义                                                    */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*              设备状态控制寄存器                                          */
/*                                                                          */
/****************************************************************************/
#define SOC_DSC_BASE_REGS                   (0x02620000)
#define SOC_DSC_JTAGID                      (0x18)
#define SOC_DSC_DEVSTAT                     (0x20)
#define SOC_DSC_KICK0                       (0x38)
#define SOC_DSC_KICK1                       (0x3C)
#define SOC_DSC_DSP_BOOT_ADDR0              (0x40)
#define SOC_DSC_DSP_BOOT_ADDR1              (0x44)
#define SOC_DSC_MACID                       (0x110)
#define SOC_DSC_LRSTNMIPINSTAT_CLR          (0x130)
#define SOC_DSC_RESET_STAT_CLR              (0x134)
#define SOC_DSC_BOOTCOMPLETE                (0x13C)
#define SOC_DSC_RESET_STAT                  (0x144)
#define SOC_DSC_LRSTNMIPINSTAT              (0x148)
#define SOC_DSC_DEVCFG                      (0x14C)
#define SOC_DSC_PWRSTATECTL                 (0x150)
#define SOC_DSC_SRIO_SERDES_STS             (0x154)
#define SOC_DSC_SMGII_SERDES_STS            (0x158)
#define SOC_DSC_PCIE_SERDES_STS             (0x15C)
#define SOC_DSC_HYPERLINK_SERDES_STS        (0x160)
#define SOC_DSC_UPP_CLOCK                   (0x16C)

#define SOC_DSC_NMIGR(n)                    (0x200 + (n * 4))
#define SOC_DSC_IPCGR(n)                    (0x240 + (n * 4))
#define SOC_DSC_IPCGRH                      (0x27C)
#define SOC_DSC_IPCAR(n)                    (0x280 + (n * 4))

#define SOC_DSC_IPCARH                      (0x2BC)
#define SOC_DSC_TINPSEL                     (0x300)
#define SOC_DSC_TOUTPSEL                    (0x304)

#define SOC_DSC_RSTMUX(n)                   (0x308 + (n * 4))

#define SOC_DSC_MAINPLLCTL0                 (0x328)
#define SOC_DSC_MAINPLLCTL1                 (0x32C)
#define SOC_DSC_DDR3PLLCTL0                 (0x330)
#define SOC_DSC_DDR3PLLCTL1                 (0x334)

#define SOC_DSC_SGMII_SERDES_CFGPLL         (0x340)
#define SOC_DSC_SGMII_SERDES_CFGRX0         (0x344)
#define SOC_DSC_SGMII_SERDES_CFGTX0         (0x348)

#define SOC_DSC_PCIE_SERDES_CFGPLL          (0x358)

#define SOC_DSC_SRIO_SERDES_CFGPLL          (0x360)
#define SOC_DSC_SRIO_SERDES_CFGRX0          (0x364)
#define SOC_DSC_SRIO_SERDES_CFGTX0          (0x368)
#define SOC_DSC_SRIO_SERDES_CFGRX1          (0x36C)
#define SOC_DSC_SRIO_SERDES_CFGTX1          (0x370)
#define SOC_DSC_SRIO_SERDES_CFGRX2          (0x374)
#define SOC_DSC_SRIO_SERDES_CFGTX2          (0x378)
#define SOC_DSC_SRIO_SERDES_CFGRX3          (0x37C)
#define SOC_DSC_SRIO_SERDES_CFGTX3          (0x380)

#define SOC_DSC_HYPERLINK_SERDES_CFGPLL     (0x3BC)
#define SOC_DSC_HYPERLINK_SERDES_CFGRX0     (0x3B8)
#define SOC_DSC_HYPERLINK_SERDES_CFGTX0     (0x3BC)
#define SOC_DSC_HYPERLINK_SERDES_CFGRX1     (0x3C0)
#define SOC_DSC_HYPERLINK_SERDES_CFGTX1     (0x3C4)
#define SOC_DSC_HYPERLINK_SERDES_CFGRX2     (0x3C8)
#define SOC_DSC_HYPERLINK_SERDES_CFGTX2     (0x3CC)
#define SOC_DSC_HYPERLINK_SERDES_CFGRX3     (0x3D0)
#define SOC_DSC_HYPERLINK_SERDES_CFGTX3     (0x3D4)

#define SOC_DSC_DEVSPEED                    (0x3F8)
#define SOC_DSC_CHIP_MISC_CTL               (0x400)
#define SOC_DSC_PIN_CONTROL_0               (0x580)
#define SOC_DSC_PIN_CONTROL_1               (0x584)
#define SOC_DSC_EMAC_UPP_PRI_ALLOC          (0x588)

#define KickUnlock()										\
{                                                           \
    HWREG(SOC_DSC_BASE_REGS + SOC_DSC_KICK0) = 0x83e70b13;  \
    HWREG(SOC_DSC_BASE_REGS + SOC_DSC_KICK1) = 0x95a4f1e0;  \
}

#define KickLock()											\
{                                                           \
    HWREG(SOC_DSC_BASE_REGS + SOC_DSC_KICK0) = 0x00000001;  \
    HWREG(SOC_DSC_BASE_REGS + SOC_DSC_KICK1) = 0x00000001;  \
}

/****************************************************************************/
/*                                                                          */
/*              外设寄存器基址                                              */
/*                                                                          */
/****************************************************************************/
#define SOC_EDMA30CC_0_REGS_C667x           (0x02700000)
#define SOC_EDMA30TC_0_REGS_C667x           (0x02760000)
#define SOC_EDMA30TC_1_REGS_C667x           (0x02768000)

#define SOC_EDMA31CC_0_REGS_C667x           (0x02720000)
#define SOC_EDMA31TC_0_REGS_C667x           (0x02770000)
#define SOC_EDMA31TC_1_REGS_C667x           (0x02778000)
#define SOC_EDMA31TC_2_REGS_C667x           (0x02780000)
#define SOC_EDMA31TC_3_REGS_C667x           (0x02788000)

#define SOC_EDMA32CC_0_REGS_C667x           (0x02740000)
#define SOC_EDMA32TC_0_REGS_C667x           (0x02790000)
#define SOC_EDMA32TC_1_REGS_C667x           (0x02798000)
#define SOC_EDMA32TC_2_REGS_C667x           (0x027A0000)
#define SOC_EDMA32TC_3_REGS_C667x           (0x027A8000)

#define SOC_INTC_0_REGS                     (0x01800000)

#define SOC_PLLC_0_REGS                     (0x02310000)

#define SOC_EDMA30CC_0_REGS                 (0x02740000)
#define SOC_EDMA30TC_0_REGS                 (0x02790000)
#define SOC_EDMA30TC_1_REGS                 (0x02798000)
#define SOC_EDMA30TC_2_REGS                 (0x027A0000)
#define SOC_EDMA30TC_3_REGS                 (0x027A8000)

#define SOC_I2C_0_REGS                      (0x02530000)

#define SOC_SPI_0_REGS                      (0x20BF0000)

#define SOC_UART_0_REGS                     (0x02540000)
#define SOC_UART_1_REGS                     (0x02550000)

#define SOC_MCBSP_0_CTRL_REGS               (0x021B4000)
#define SOC_MCBSP_0_FIFO_REGS               (0x021B6000)
#define SOC_MCBSP_0_DATA_REGS               (0x22000000)
#define SOC_MCBSP_1_CTRL_REGS               (0x021B8000)
#define SOC_MCBSP_1_FIFO_REGS               (0x021BA000)
#define SOC_MCBSP_1_DATA_REGS               (0x22400000)

#define SOC_MPU_0_REGS                      (0x02360000)
#define SOC_MPU_1_REGS                      (0x02368000)
#define SOC_MPU_2_REGS                      (0x02370000)
#define SOC_MPU_3_REGS                      (0x02378000)
#define SOC_MPU_4_REGS                      (0x02380000)

#define SOC_UPP_0_REGS                      (0x02580000)

#define SOC_EMAC_SUBSYSTEM                  (0x02C08000)

#define SOC_GPIO_0_REGS                     (0x02320000)

#define SOC_PSC_0_REGS                      (0x02350000)

#define SOC_TMR_0_REGS                      (0x02200000)
#define SOC_TMR_1_REGS                      (0x02210000)
#define SOC_TMR_2_REGS                      (0x02220000)
#define SOC_TMR_3_REGS                      (0x02230000)
#define SOC_TMR_4_REGS                      (0x02240000)
#define SOC_TMR_5_REGS                      (0x02250000)
#define SOC_TMR_6_REGS                      (0x02260000)
#define SOC_TMR_7_REGS                      (0x02270000)
#define SOC_TMR_8_REGS                      (0x02280000)
#define SOC_TMR_9_REGS                      (0x02290000)
#define SOC_TMR_10_REGS                     (0x022A0000)
#define SOC_TMR_11_REGS                     (0x022B0000)
#define SOC_TMR_12_REGS                     (0x022C0000)
#define SOC_TMR_13_REGS                     (0x022D0000)
#define SOC_TMR_14_REGS                     (0x022E0000)
#define SOC_TMR_15_REGS                     (0x022F0000)

#define SOC_XMC_CONFIG_REGS                 (0x08000000)



#define SOC_DDR3_0_CTRL_REGS                (0x20C00100)
#define SOC_DDR3_0_DATA_REGS                (0x80000000)

#define SOC_SRIO_0_REGS                     (0x02900000)
#define SOC_PCIE_0_REGS                     (0x21800000)

#define SOC_SEMAPHORE2_0_REGS               (0x02640000)

/****************************************************************************/
/*                                                                          */
/*              EDMA 相关宏定义                                             */
/*                                                                          */
/****************************************************************************/
/* 与平台有关的参数 */
#define SOC_EDMA30_NUM_DMACH_C667x          16
#define SOC_EDMA31_NUM_DMACH_C667x          64
#define SOC_EDMA32_NUM_DMACH_C667x          64

#define SOC_EDMA3_NUM_DMACH                 64
#define SOC_EDMA3_NUM_QDMACH                8
#define SOC_EDMA3_NUM_PARAMSETS             512
#define SOC_EDMA3_NUM_EVQUE                 4
#define SOC_EDMA3_CHMAPEXIST                0
#define SOC_EDMA3_NUM_REGIONS               8
#define SOC_EDMA3_MEMPROTECT                0

/* 通道实例数 */
#define SOC_EDMA3_CHA_CNT                   (SOC_EDMA3_NUM_DMACH + \
                                             SOC_EDMA3_NUM_QDMACH)

/* QDMA 通道 */
#define SOC_EDMA3_QCHA_BASE                 (SOC_EDMA3_NUM_DMACH)       /* QDMA 基通道 */
#define SOC_EDMA3_QCHA_0                    (SOC_EDMA3_QCHA_BASE + 0)   /* QDMA 通道 0 */
#define SOC_EDMA3_QCHA_1                    (SOC_EDMA3_QCHA_BASE + 1)   /* QDMA 通道 1 */
#define SOC_EDMA3_QCHA_2                    (SOC_EDMA3_QCHA_BASE + 2)   /* QDMA 通道 2 */
#define SOC_EDMA3_QCHA_3                    (SOC_EDMA3_QCHA_BASE + 3)   /* QDMA 通道 3 */

/* EDMA 通道控制器数 */
#define SOC_EDMACC_ANY                     -1
#define SOC_EDMACC_0                        0 /* EDMACC 实例 0 */

/* EDMA 事件队列数 */
#define SOC_EDMA3_QUE_0                     0 /* 队列 0 */
#define SOC_EDMA3_QUE_1                     1 /* 队列 1 */
#define SOC_EDMA3_QUE_2                     2 /* 队列 2 */
#define SOC_EDMA3_QUE_3                     3 /* 队列 3 */

/* 传输控制器数，4个传输控制器一般与4个事件队列一一对应 */
#define SOC_EDMATC_ANY                     -1
#define SOC_EDMATC_0                        0  /* EDMATC 实例 0 */
#define SOC_EDMATC_1                        1  /* EDMATC 实例 1 */
#define SOC_EDMATC_2                        2  /* EDMATC 实例 2 */
#define SOC_EDMATC_3                        3  /* EDMATC 实例 3 */

#define SOC_EDMA3_REGION_GLOBAL            -1
#define SOC_EDMA3_REGION_0                  0
#define SOC_EDMA3_REGION_1                  1
#define SOC_EDMA3_REGION_2                  2
#define SOC_EDMA3_REGION_3                  3
#define SOC_EDMA3_REGION_4                  4
#define SOC_EDMA3_REGION_5                  5
#define SOC_EDMA3_REGION_6                  6
#define SOC_EDMA3_REGION_7                  7

/****************************************************************************/
/*                                                                          */
/*              外设时钟                                                    */
/*                                                                          */
/****************************************************************************/
/* CPU 核心 */
#define SOC_SYSCLK_1_FREQ                   (1000000000)
/* （可编程时钟）CPU 核心仿真 */
#define SOC_SYSCLK_2_FREQ                   (SOC_SYSCLK_1_FREQ/3)
/* MSMC HyperLink DDR EMIF */
#define SOC_SYSCLK_3_FREQ                   (SOC_SYSCLK_1_FREQ/2)
/* Switch fabrics 高速外设 Debug_SS ETB */
#define SOC_SYSCLK_4_FREQ                   (SOC_SYSCLK_1_FREQ/3)
/* （可编程时钟）System trace 模块 */
#define SOC_SYSCLK_5_FREQ                   (SOC_SYSCLK_1_FREQ/5)
/* DDR3 EMIF PVT-compensated buffer */
#define SOC_SYSCLK_6_FREQ                   (SOC_SYSCLK_1_FREQ/64)
/* 低速外设（GPIO, UART, Timer, I2C, SPI, EMIF16, McBSP 等） */
#define SOC_SYSCLK_7_FREQ                   (SOC_SYSCLK_1_FREQ/6)
/* （可编程时钟） 低速系统时钟 */
#define SOC_SYSCLK_8_FREQ                   (SOC_SYSCLK_1_FREQ/64)
/* SmartReflex */
#define SOC_SYSCLK_9_FREQ                   (SOC_SYSCLK_1_FREQ/12)
/* SRIO */
#define SOC_SYSCLK_10_FREQ                  (SOC_SYSCLK_1_FREQ/3)
/* PSC */
#define SOC_SYSCLK_11_FREQ                  (SOC_SYSCLK_1_FREQ/6)



/****************************************************************************/
/*                                                                          */
/*              EMIF接口定义                                                 */
/*                                                                          */
/****************************************************************************/
#define SOC_EMIFA_0_REGS                    (0x20C00000)        //配置寄存器

#define SOC_EMIFA_CS0_ADDR                  (0x70000000)
#define SOC_EMIFA_CS1_ADDR                  (0x74000000)
#define SOC_EMIFA_CS2_ADDR                  (0x78000000)
#define SOC_EMIFA_CS3_ADDR                  (0x7c000000)


/****************************************************************************/
/*                                                                          */
/*              外设定义                                                    */
/*                                                                          */
/****************************************************************************/
/* 定时器 */
#define Timer_TIMI0 0
#define Timer_TIMI1 1

#define Timer_TIMO0 0
#define Timer_TIMO1 1


#ifdef __cplusplus
}
#endif

#endif
