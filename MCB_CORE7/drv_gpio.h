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
 *   2015年05月06日
 *
 */

#ifndef __GPIO_H__
#define __GPIO_H__

//#include "hw_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/*                                                                          */
/*              宏定义                                                      */
/*                                                                          */
/****************************************************************************/

// DSP C6678 无管脚复用而且只有 16 个（0 - 15） GPIO 口

enum
{
    GPIO_PIN_0 = 0,
    GPIO_PIN_1,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
};


/* GPIO 管脚功能选择 */
/* 普通 IO 口 */
#define GPIO_NORMAL_ENABLED           1
/* 外设输入输出口 */
#define GPIO_FUNCTION_ENABLED         0

/* GPIO 管脚方向 */
/* 输入 */
#define GPIO_DIR_INPUT                1
/* 输出 */
#define GPIO_DIR_OUTPUT               0

/* 中断触发类型 */
/* 禁用管脚边沿中断触发 */
#define GPIO_INT_TYPE_NOEDGE          0
/* 使能下降沿触发 */
#define GPIO_INT_TYPE_FALLEDGE        1
/* 使能上升沿触发 */
#define GPIO_INT_TYPE_RISEDGE         2
/* 使能上升沿及下降沿触发 */
#define GPIO_INT_TYPE_BOTHEDGE        3

/* 输出状态 */
/* 逻辑 0 */
#define GPIO_PIN_LOW                  0
/* 逻辑 1 */
#define GPIO_PIN_HIGH                 1


/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
void GPIO_DirModeSet(unsigned int pinNumber, unsigned int pinDir);
unsigned int GPIO_DirModeGet(unsigned int pinNumber);

void GPIO_PinWrite(unsigned int pinNumber, unsigned int bitValue);
int GPIO_PinRead(unsigned int pinNumber);

void GPIO_IntTypeSet( unsigned int pinNumber, unsigned int intType);
unsigned int GPIO_IntTypeGet( unsigned int pinNumber);

void GPIO_IntcEnable(void);
void GPIO_IntcDisable(void);


#ifdef __cplusplus
}
#endif
#endif
