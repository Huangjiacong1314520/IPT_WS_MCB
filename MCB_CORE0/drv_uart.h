/*
 * uart.h
 *
 *  Created on: 2019年10月19日
 *      Author: zhuyb
 */

#ifndef UART_H_
#define UART_H_

#define BAUD_115200         (115200)

 /*
 * UART工作模式
 */
enum
{
	UART_POLL = 0,  //查询模式
	UART_INTERRUPT, //中断模式
};

/*
* 数据位
*/
enum
{
	DATA_5_BIT = 0,
	DATA_6_BIT,
	DATA_7_BIT,
	DATA_8_BIT,
};


/*
* 停止位
*/
enum
{
	STOP_1_BIT = 0,
	STOP_2_BIT,
};

/*
* 校验位
*/
enum
{
	PARITY_ODD = 0,     //奇校验
	PARITY_EVEN,        //偶校验
	PARITY_NO,          //无校验
};

/*
*Over-Sanpling Mode Select
*/
enum
{
	OSM_SEL_16X = 0,
	OSM_SEL_13X,
};

 /*
 * UART 配置定义
 */
typedef struct _UART_CONFIG_
{
	unsigned char nUartMode;    //中断、轮训
	unsigned char nDataBit;     //数据位
	unsigned char nStopBit;     //停止位
	unsigned char nParityBit;   //校验位
	unsigned int nBaudRate;     //波特率
	unsigned int nOverSampRate; //分频因子
}UartCfg_st, *pUartCfg_st;


void InitUart(UartCfg_st uartcfg);

int Uart_Printf(char *pBuf, int nLen);

int Uart_RecvInfo(unsigned char *pBuf, int nLen);

int Uart_GetIntStatus();

#endif /* UART_H_ */
