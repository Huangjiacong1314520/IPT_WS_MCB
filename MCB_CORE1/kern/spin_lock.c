/*
 * spin_lock.c
 *
 *  Created on: 2022Äê2ÔÂ20ÈÕ
 *      Author: Administrator
 */

#include "spin_lock.h"
#include <ti/csl/csl_cacheAux.h>
#include <c6x.h>
#include <stdio.h>
void lock_init(struct spinlock *lock,size_t semnum)
{
    lock->owner = 0;
    lock->sem = semnum;
}
void acquire(struct spinlock *lock)
{
    while(CSL_semAcquireDirect(lock->sem) != 1)
        ;
    lock->owner = DNUM;
}
void release(struct spinlock *lock)
{
    lock->owner = 0;
    CSL_semReleaseSemaphore(lock->sem);
}
