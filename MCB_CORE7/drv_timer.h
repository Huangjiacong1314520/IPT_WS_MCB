/*
 * Timer.h
 *
 *  Created on: 2020-9-2
 *      Author: linxi
 */

#ifndef TIMER_H_
#define TIMER_H_


#define TIMER_ENAMODE_ONCE   1 //计数一次
#define TIMER_ENAMODE_CONT   2 //连续计数

#ifdef __cplusplus
extern "C" {
#endif

/*
 * config
 */
int Timer_Init(void);
int Timer_Delay(unsigned char nTimer, unsigned int nPerHi, unsigned int nPerLo);
int Timer_Config(unsigned char nTimer, unsigned int nPerHi, unsigned int nPerLo);
int Timer_Start(unsigned char nTimer, unsigned char EnaMode);
int Timer_GetCount(unsigned char nTimer, unsigned int *pCntHi, unsigned int *pCntLo);
int Timer_ModifyCount(unsigned char nTimer, unsigned int nCntHi, unsigned int nCntLo);
int Timer_Close(unsigned char nTimer);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_H_ */
