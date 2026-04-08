/**
 * @file
 *
 * lwIP 基于此硬件系统的配置
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
#include "stdlib.h"
#include "string.h"

#define NO_SYS  			  1    //关闭协议栈对于操作系统的支持
#define IP_FORWARD            0    //todo: ip.c中接口转发数据包

#define LWIP_IGMP             0	   //组播实现
//#define LWIP_DHCP             1
//#define LWIP_DNS              1
//#define LWIP_AUTOIP		  	1

#define LWIP_NETCONN 		  0
#define LWIP_SOCKET 		  0

#define LWIP_ARP              1
#define LWIP_IPV4             1

#define MEM_ALIGNMENT 		  4
#define MEM_SIZE 		      (1024*600)  	//与应用层的数据缓存有关，至少要大于MEMP_NUM_TCP_SEG*TMSS的空间
#define MEMP_NUM_PBUF         150     		//原先调试的为100，协议栈默认16 ,(used for PBUF_ROM and PBUF_REF)
#define PBUF_POOL_SIZE        (200) 		//PBUF_POOL(MEMP_PBUF_POOL)类型的buffer的数量

#define TCP_MSS               1460
#define TCP_SND_BUF 		  65535//2048*20//65535//(60*TCP_MSS)		 //65535//(40*TCP_MSS)65535
#define TCP_WND               65535//2048*10//65535             //TCP窗口的大小，用于流量控制   40960字节64240

#define TCP_SND_QUEUELEN      (4* (TCP_SND_BUF/TCP_MSS))
#define MEMP_NUM_TCP_SEG      TCP_SND_QUEUELEN       //MEMP_NUM_TCP_SEG >= TCP_SND_QUEUELEN

//#define TCP_OVERSIZE                    0
#define TCP_TMR_INTERVAL       250  /* The TCP timer interval in milliseconds. */

#define LWIP_TIMERS						1

#define LWIP_DISABLE_TCP_SANITY_CHECKS  0

#define IP_OPTIONS_ALLOWED              1    //default 1

#define MEMP_NUM_FRAG_PBUF   		    1       //the number of IP fragments simultaneously sent(fragments, not whole packets!).
#define MEMP_NUM_REASSDATA   			3 	    //2    10 100  MEMP_NUM_REASSDATA(whole packet) < IP_REASS_MAX_PBUFS
#define IP_REASS_MAX_PBUFS              125  	//128//Total maximum amount of pbufs waiting to be reassembled.PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS
#define IP_REASS_MAXAGE                 10       //3     Maximum time a fragmented IP packet waits for all fragments to arrive.



#define LWIP_NETIF_TX_SINGLE_PBUF		0    //单一pbuf发送，TCP和IP-FLAG不能打开
//#define IP_FRAG_USES_STATIC_BUF         0

#define LWIP_UDPLITE                    0    //default:0
#define LWIP_RAW                        0    //使应用层能够连接到IP层本身，默认值0
#define SYS_LIGHTWEIGHT_PROT            0    //enable inter-task protection 默认值为1，在无操作系统时这里应该为零

/******************************************************************************************************
 ************************************校验相关的开关*********************************************************
 *******************************************************************************************************/
#define LWIP_CHECKSUM_CTRL_PER_NETIF    0   //打开netif网络接口的校验和产生及校验，这个打开的前提为以下
#define LWIP_CHECKSUM_ON_COPY           0   //当从应用层buffer拷贝数据到pbuf时，对数据进行累加校验，默认值为0

//对于的各个协议层输入数据包的校验和的校验
#define CHECKSUM_CHECK_TCP              1//传入的TCP包校验    //这几个参数打开实现的功能可能已经由GMAC网口实现
#define CHECKSUM_CHECK_UDP              1//传入的UDP包校验
#define CHECKSUM_CHECK_IP               1//传入的IP报头校验
#define CHECKSUM_CHECK_ICMP             1
#define CHECKSUM_CHECK_ICMP6            1
//对于各个各个协议层输出数据包的校验和的产生
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_GEN_ICMP6              1
/*******************************************************************************************************/
/*******************************************************************************************************/
#define LWIP_STATS_LARGE			    1    //调试lwip_stats计数器的数据类型，此时为32bit，否则16bit
#define MEMP_OVERFLOW_CHECK             0    //内存泄露的检查开关

//#define LWIP_DEBUG 		  	   			LWIP_DBG_ON
//#define TCP_DEBUG                         LWIP_DBG_ON
//#define TCP_OUTPUT_DEBUG     	 		LWIP_DBG_ON
//#define TCP_INPUT_DEBUG		  			LWIP_DBG_ON
//#define TCP_QLEN_DEBUG					LWIP_DBG_ON
//#define TCP_WND_DEBUG       		  	LWIP_DBG_ON
//#define TCP_CWND_DEBUG        			LWIP_DBG_ON


#define LWIP_TCP_KEEPALIVE              0//does not require sockets.c, and will affect tcp.c

#if 0
/*调试开关*/
#define LWIP_DEBUG 		  	  LWIP_DBG_ON
#define	NETIF_DEBUG			  LWIP_DBG_ON
#define ETHARP_DEBUG   		  LWIP_DBG_ON
#define ICMP_DEBUG 			  LWIP_DBG_ON
#define IP_DEBUG 			  LWIP_DBG_ON
#define	TCP_DEBUG			  LWIP_DBG_ON
#define PBUF_DEBUG            LWIP_DBG_ON
#define INET_DEBUG            LWIP_DBG_ON
#define IP_REASS_DEBUG        LWIP_DBG_ON

#define RAW_DEBUG         	  LWIP_DBG_ON



////////////////////////////
#define TCP_INPUT_DEBUG		  LWIP_DBG_ON
/**
 * TCP_FR_DEBUG: Enable debugging in tcp_in.c for fast retransmit.
 */
#define TCP_FR_DEBUG		  LWIP_DBG_ON
/**
 * TCP_RTO_DEBUG: Enable debugging in TCP for retransmit
 * timeout.
 */
#define TCP_RTO_DEBUG         LWIP_DBG_ON
/**
 * TCP_CWND_DEBUG: Enable debugging for TCP congestion window.
 */
#define TCP_CWND_DEBUG        LWIP_DBG_ON

/**
 * TCP_WND_DEBUG: Enable debugging in tcp_in.c for window updating.
 */
#define TCP_WND_DEBUG         LWIP_DBG_ON
/**
 * TCP_OUTPUT_DEBUG: Enable debugging in tcp_out.c output functions.
 */
#define TCP_OUTPUT_DEBUG      LWIP_DBG_ON
/**
 * TCP_RST_DEBUG: Enable debugging for TCP with the RST message.
 */
#define TCP_RST_DEBUG         LWIP_DBG_ON

/**
 * TCP_QLEN_DEBUG: Enable debugging for TCP queue lengths.
 */
#define TCP_QLEN_DEBUG        LWIP_DBG_ON

/////////////////////////


/**
 * UDP_DEBUG: Enable debugging in UDP.
 */
#define UDP_DEBUG                       LWIP_DBG_ON
/**
 * TCPIP_DEBUG: Enable debugging in tcpip.c.
 */
#define TCPIP_DEBUG                     LWIP_DBG_ON

#endif


/* */

//#define MEM_DEBUG                       LWIP_DBG_ON
//#define MEMP_DEBUG                      LWIP_DBG_ON

//#define MEMP_MEM_MALLOC              1

#endif /* __LWIPOPTS_H__ */
