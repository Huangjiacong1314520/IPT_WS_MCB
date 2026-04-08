/*
 * spin_lock.h
 *
 *  Created on: 2022Äê2ÔÂ20ÈÕ
 *      Author: Administrator
 */

#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_
#include <ti/csl/csl_sem.h>
#include <ti/csl/csl_semAux.h>
#include <stdio.h>
struct spinlock {
    size_t owner;
    size_t  sem;
};

extern void lock_init(struct spinlock *lock,size_t semnum);
extern void acquire(struct spinlock *lock);
extern void release(struct spinlock *lock);


#endif /* SPIN_LOCK_H_ */
