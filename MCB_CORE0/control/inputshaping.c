/*
 * inputshaping.c
 *
 *  Created on: 2022Äê5ÔÂ22ÈÕ
 *      Author: Administrator
 */
#include "inputshaping.h"
#define MAXBUFFERLENGTH 5u
static double buffer[MAXBUFFERLENGTH] = {0};
static double *pbuffer = buffer;
void ZeroBuffer()
{
    pbuffer = buffer;
    MyMemSet(buffer, 0, MAXBUFFERLENGTH*8);
}
double InputShapingCalculate(double ouput_now,double *param,int order)
{
    int i;
    double ret = 0.0;
    *pbuffer = ouput_now;
    double *old = pbuffer;
    for (i = 0; i < order;++i) {
        ret += *old * param[i];
        if (--old < &buffer[0])
            old = &buffer[order-1];
    }
    if (++pbuffer > &buffer[order-1])
        pbuffer = &buffer[0];
    return ret;
}


