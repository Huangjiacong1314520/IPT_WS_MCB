/*
 * inoutshaping.h
 *
 *  Created on: 2022Äê5ÔÂ22ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_INPUTSHAPING_H_
#define CONTROL_INPUTSHAPING_H_
#include "function.h"

void ZeroBuffer();
double InputShapingCalculate(double ouput_now,double *param,int order);



#endif /* CONTROL_INPUTSHAPING_H_ */
