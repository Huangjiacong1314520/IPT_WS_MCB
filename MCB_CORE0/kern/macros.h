/*
 * interrupt.h
 *
 *  Created on: 2022Äê1ÔÂ17ÈÕ
 *      Author: CXY
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_ipcAux.h>
#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/src/intc/csl_intcAux.h>
#include <c6x.h>
#include <stdio.h>

#define FOREVER 1
#define SRIO_INTC 20
#define IPC_INTC  91
#define SEM_INTC  16
#define MAXSLAVENUM 6
#define CLOCALID 0X30

#define WAIT(x) while(x)

#define ISRPROLOGUE(eventid) do{ \
        CSL_intcEventDisable(eventid); \
        }while(0)
#define ISREPILOGUE(eventid) do{ \
        CSL_intcEventClear(eventid); \
        CSL_intcEventEnable(eventid); \
        }while(0)
#define SEMUP(sem) do{ \
        CSL_semReleaseSemaphore(sem); \
        }while(0)
#define SEMDOWN(sem) do{ \
         CSL_semAcquireCombined(sem); \
        }while(0)
#define SEMENABLE(masterid) do{ \
        uint32_t flag = CSL_semGetFlags(masterid); \
        CSL_semClearFlags(masterid, flag); \
        CSL_semSetEoi(masterid); \
        }while(0)
#define INTCENABLE(x) do{\
        CSL_IntcVectId temp = (CSL_IntcVectId)4;\
        CSL_intcInterruptEnable(temp);\
        }while(0)

#define CORE0FINISHED !CSL_semIsFree(10)

#define CORE1INIT    !CSL_semIsFree(1)
#define CORE2INIT    !CSL_semIsFree(2)
#define CORE3INIT    !CSL_semIsFree(3)
#define CORE4INIT    !CSL_semIsFree(4)
#define CORE5INIT    !CSL_semIsFree(5)
#define CORE6INIT    !CSL_semIsFree(6)
#define CORE7INIT    !CSL_semIsFree(7)


#define CORE1FINISHED !CSL_semIsFree(11)
#define CORE2FINISHED !CSL_semIsFree(12)
#define CORE3FINISHED !CSL_semIsFree(13)
#define CORE4FINISHED !CSL_semIsFree(14)
#define CORE5FINISHED !CSL_semIsFree(15)
#define CORE6FINISHED !CSL_semIsFree(16)
#define CORE7FINISHED !CSL_semIsFree(17)

#define OFFSET(TYPE,MEMBER) (size_t)(&(((TYPE *)0)->MEMBER))


#endif /* INTERRUPT_H_ */
