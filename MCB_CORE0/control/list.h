/*
 * list
 *
 *  Created on: 2022Äê7ÔÂ9ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_LIST_H_
#define CONTROL_LIST_H_
#include "global.h"
struct ListNode{
//    struct ProberReg reg;
    struct ControlCommand cmd;
    SParaStruct param[AxisCount];
    struct ListNode *next;
};
struct ListRoot{
    struct ListNode *head;
};
struct ListNode* GetHead();
void Addtail(SParaStruct *param,struct ControlCommand *cmd);
void QueuePush(SParaStruct *param,struct ControlCommand *cmd);
void QueuePop();
void DeleteQueue();
#endif /* CONTROL_LIST_H_ */
