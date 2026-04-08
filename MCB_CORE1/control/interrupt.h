/*
 * interrupt.h
 *
 *  Created on: 2021Äê8ÔÂ22ÈÕ
 *      Author: lkx
 */

#ifndef CONTROL_INTERRUPT_H_
#define CONTROL_INTERRUPT_H_

#include "global.h"
#include "function.h"
#include "refcurve.h"
#include "encapsulation.h"

void IsrVxWorks();
void IsrMotionControl(size_t axis);
void calculateRefCurve(size_t axis);

void IsrMotionControlCore1();
void IsrMotionControlCore2();
void IsrMotionControlCore3();
void IsrMotionControlCore4();
void IsrMotionControlCore5();
void IsrMotionControlCore6();
#endif /* CONTROL_INTERRUPT_H_ */
