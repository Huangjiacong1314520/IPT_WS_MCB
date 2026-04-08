/*
 * intc.h
 *
 *  Created on: 2020Äê9ÔÂ4ÈÕ
 *      Author: zhuyb
 */

#ifndef INTC_INTC_H_
#define INTC_INTC_H_

typedef void (* funcIntcEventHandler)(void *arg);

#ifdef __cplusplus
extern "C" {
#endif

int INTC_Init(void);
int INTC_Uninit(void);

int INTC_Open(int nEventID, int nVectID, funcIntcEventHandler func, void *pParam);
int INTC_Close(int nEventID);

int INTC_CIC_Open(char nCIC);
int INTC_CIC_Init(char nCIC, int nSysInter, int nHostInter);
int INTC_CIC_ClearSysInter(char nCIC, int nSysInter, int nHostInter);
int INTC_CIC_Colse(char nCIC);

#ifdef __cplusplus
}
#endif
#endif /* INTC_INTC_H_ */
