/*
 * interruptcore5.c
 *
 *  Created on: 2022Äê5ÔÂ27ÈÕ
 *      Author: Administrator
 */
#include "interrupt.h"
void IsrMotionControlCore5()
{
    int i;
    for (i = 0;i < 3; ++i) {
        CommandHandle temp = &pCommandPackage->CommandArray[i];
        if (MultiCoreShareData->CurState == Default) {
            Clear((Slot)i, temp->axis);
            continue;
        }
        switch (temp->task){
        case Task1:
            break;
        case Task2:
            break;
        case Task3:
            break;
        default:
            break;
        }
    }
}


