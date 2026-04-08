/*
 * list.c
 *
 *  Created on: 2022Äê7ÔÂ9ÈÕ
 *      Author: Administrator
 */
#include "list.h"
#include <stdlib.h>
static struct ListRoot root = {.head = NULL};
static struct ListNode *AllocNewNode(SParaStruct *param,struct ControlCommand *cmd)
{
    struct ListNode *temp = (struct ListNode *)malloc(sizeof(struct ListNode));
    memset(temp, 0,sizeof(struct ListNode));
    temp->next = NULL;
    if (param != NULL)
        memcpy(&temp->param,param,sizeof(SParaStruct)*AxisCount);
    else
        memset(&temp->param, 0, sizeof(SParaStruct)*AxisCount);
    if (cmd != NULL)
        memcpy(&temp->cmd,cmd,sizeof(struct ControlCommand));
    else
        memset(&temp->cmd,0,sizeof(struct ControlCommand));
    return temp;
}
void QueuePush(SParaStruct *param,struct ControlCommand *cmd)
{
    if (root.head == NULL) {
        struct ListNode *temp = AllocNewNode(param,cmd);
        root.head = temp;
        return;
    }
    struct ListNode *tail = root.head;
    while(tail->next != NULL)
        tail = tail->next;
    tail->next = AllocNewNode(param,cmd);
    return;
}
struct ListNode* GetHead()
{
    return root.head;
}
void QueuePop()
{
    if (root.head == NULL)
        return;
    struct ListNode *old = root.head;
    root.head = root.head->next;
    free(old);
}
void DeleteQueue()
{
    if (root.head == NULL)
        return;
    struct ListNode *iter = root.head;
    struct ListNode *temp = root.head;
    root.head = NULL;
    while(iter) {
        temp = iter->next;
        free(iter);
        iter = temp;
    }
    return;
}
