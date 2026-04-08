/*
 * ipc.h
 *
 *  Created on: 2020-9-20
 *      Author: linxi
 */

#ifndef IPC_H_
#define IPC_H_

void IPC_SendInterruptToCoreX(unsigned char nNextCore, unsigned int nInfo);

void IPC_GetCoreSoureInfo(unsigned char nCore, unsigned int* pInfo);

void IPC_ClearSourceInfo(unsigned char nCore);


#endif /* IPC_H_ */
