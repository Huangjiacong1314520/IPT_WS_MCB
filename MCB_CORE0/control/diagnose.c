/*
 * diagnose.c
 *
 *  Created on: 2022Äê5ÔÂ31ÈÕ
 *      Author: Administrator
 */
#include "diagnose.h"
#include "global.h"
void SetErrorCode(SystemError error)
{
   // ErrorCode |= (1 << ((unsigned int)error -1));
    MultiCoreShareData->ErrorCode |= (1<<((unsigned int)error - 1));
}


