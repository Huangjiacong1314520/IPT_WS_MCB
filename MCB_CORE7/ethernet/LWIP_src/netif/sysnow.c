/*
 * sys.c
 *
 *  Created on: 2018-6-2
 *      Author: Administrator
 */

#include "lwip/sys.h"

extern unsigned long long int get_crt_time();

volatile int  timerISRCounter=1;
//void sys_arch_unprotect(sys_prot_t pval)
//{
//	return;
//}
//
//sys_prot_t sys_arch_protect(void)
//{
//	return;
//}

u32_t sys_now(void)
{
	 timerISRCounter=(unsigned int)(get_crt_time()/1000000);
	 return timerISRCounter;
}
