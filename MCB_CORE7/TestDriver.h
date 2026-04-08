/*
 * TestDriver.h
 *
 *  Created on: 2020-8-31
 *      Author: linxi
 */

#ifndef TESTDRIVER_H_
#define TESTDRIVER_H_
#include "drv_edma3.h"
#include "drv_cache.h"
#include "drv_pll.h"
#include "drv_srio.h"
#include "drv_timer.h"
#include "drv_spiflash.h"
#include "drv_gpio.h"
#include "drv_nand.h"
#include "drv_uart.h"
#include "drv_intc.h"
#include "drv_psc.h"
#include "drv_ipc.h"
#include "drv_netctrl.h"
#include "interfacedrive.h"

unsigned int SwapDWordByte(unsigned int nData);

//全局默认初始化配置
void Global_Default_Init();

//所有中断配置
int Init_All_Intc();

//测试EDMA
void Test_EDMA3();

//测试定时器
void Test_Timer();

//测试SPI-Nor-Flash
void Test_SPI_Nor_Flash();

//测试EMIF-Nand-Flash
void Test_EMIF_Nand_Flash();

//测试GPIO
void Test_GPIO();

//测试DDR读写
void Test_DDR();

//测试多核启动、同步（IPC、共享内存）
void Test_Core();

//测试SRIO同时接收7种流时不丢中断
void Test_SRIO_Interrupt_Deprecated();

//测试EDMAE3 链接触发功能
void Test_EDMA3_CCode();

//测试串口收发功能
void Test_Uart();

//测试网口通信
//void Test_Ethernet();

#endif /* TESTDRIVER_H_ */
