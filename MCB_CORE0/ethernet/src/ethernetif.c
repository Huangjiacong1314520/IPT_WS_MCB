/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
//#include "netif/ppp_oe.h"

#include "Keystone_common.h"
#include "KeyStone_GE_Init_drv.h"
#include "KeyStone_Serdes_init.h"
#include "GE_test.h"
#include "GE_PktDMA_init.h"
#include "../../drv_intc.h"
#include "../../drv_uart.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'G'
#define IFNAME1 'b'

/*select between internal/external loopback test or test between two DSPs*/
GE_Test_Data_Path test_data_path= GE_TEST_DSP0_TO_DSP1;

/*select between 10/100/1000Mbps or auto negotiation mode*/
Ethernet_Mode ethernet_mode = ETHERNET_AUTO_NEGOTIAT_MASTER;

//The port connection state for the test
GE_Port_Connection port_connect[GE_NUM_ETHERNET_PORT]=
{
	GE_PORT_CABLE_CONNECT, //SGMII port 0
	GE_PORT_NOT_USED //SGMII port 1
};

/*use long long type (8 bytes) for MAC address, but only lower 6 bytes are valid.
Please note the byte order, MAC address byte 5 is in the lowest bits.
Each MAC address corresponding to a Ethernet port*/
extern unsigned char g_MyMacID[6];
unsigned long long Source_MAC_address[GE_NUM_ETHERNET_PORT]=
{
	0x112233445566,
	0x0
};
unsigned long long Dest_MAC_address[GE_NUM_ETHERNET_PORT]=
{
	0xF875a418fef0,
	0x0
};

Ethernet_Port_Config ethernet_port_cfg[GE_NUM_ETHERNET_PORT];

Ethernet_ALE_Config ale_cfg;

Ethernet_MDIO_Config mdio_cfg;

SerdesLinkSetup serdesLinkSetup;

KeyStone_GE_Config ge_cfg;



/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};



#define  revPktNum  (64)
#define  PKT_SIZE_FRAMEBUF   (1664)

#define  sndPktNum  (8)
#pragma  DATA_SECTION(sndBufMem, "DESCMEM_BUFFER_SEND");
#pragma  DATA_ALIGN(sndBufMem, 128);
volatile unsigned char    sndBufMem[sndPktNum][PKT_SIZE_FRAMEBUF];

volatile unsigned int sendcount=0;



Uint32 TxDescriptorTempBuffer;

volatile unsigned int        PktFlgLen;	//当前接收的数据包长度
volatile unsigned char       *pRcvData;	//当前接收的数据的地址

extern struct netif g_TI6678_netif;
extern unsigned int g_VectID;

void GE_Message_ISR(void* param);


//64左移len 位
unsigned long long move_left64(unsigned long long a, int len)
{
	 unsigned int *p = (unsigned int*) &a;
	 if (len <32)
	 {
		 *(p+1) <<= len;
		 unsigned int tmp = (*p) >> (32-len);
		 *(p+1) |= tmp;
		 *p <<= len;
	 }
	 else
	 {
		  *(p+1) = *p;
		  *p = 0x00000000;
		  *(p+1) <<= (len-32);
	 }
	 return a;
}

//64右移len 位
unsigned long long move_right64(unsigned long long a, int len)
{
	 unsigned int *p = (unsigned int*) &a;
	 if (len<32)
	 {
		  *p >>= len;
		  unsigned int tmp = *(p+1) << (32-len);
		  *p |= tmp;
		  *(p+1) >>= len;
	 }
	 else
	 {
		  *p = *(p+1);
		  *(p+1) = 0x00000000;
		  *p >>= (len-32);
	 }
	 return a;
}


unsigned long long int get_crt_time()
{
	return _itoll(TSCH,TSCL);
}

/*wait for PHY ready for the ports connected through ethernet cable*/
void Wait_PHY_link()
{
	int i,j;
	Uint32 uiStartTSC;

	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		if(GE_PORT_CABLE_CONNECT != port_connect[i])
			continue;

		uiStartTSC= TSCL;
		if(i == 0)
		{
			j = ge_cfg.mdio_cfg->link_INT0_PHY_select;
		}
		else
		{
			j = ge_cfg.mdio_cfg->link_INT1_PHY_select;
		}
		while(0==(gpMDIO_regs->LINK_REG&(1<<j)))
		{
			if(TSC_count_cycle_from(uiStartTSC)>0x3FFFFFFF)
			{
				printf("Wait for port %d PHY link...\n", i);
				uiStartTSC= TSCL;
			}
		}
		/* Clear Interrupt events in MDIO*/
		gpMDIO_regs->LINK_INT_RAW_REG= 1<<j;
	}
}

/*For all tests, the ALE is setup to receive all packets.
For these tests, transmit will use direct packet (indpendent of ALE).*/
CSL_CPSW_3GF_ALE_UNICASTADDR_ENTRY ALE_entries[GE_NUM_ETHERNET_PORT];

//setup ALE entries for the 4 destination MAC addresses
void ALE_Entries_Init(ALE_Test_Mode ALE_test_mode)
{
	int i;

	if(ALE_BYPASS==ALE_test_mode)
		return;

	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		ALE_entries[i].macAddress[0]= (_hill(Dest_MAC_address[i])>>8)&0xFF;
		ALE_entries[i].macAddress[1]= (_hill(Dest_MAC_address[i])>>0)&0xFF;
		ALE_entries[i].macAddress[2]= (_loll(Dest_MAC_address[i])>>24)&0xFF;
		ALE_entries[i].macAddress[3]= (_loll(Dest_MAC_address[i])>>16)&0xFF;
		ALE_entries[i].macAddress[4]= (_loll(Dest_MAC_address[i])>>8)&0xFF;
		ALE_entries[i].macAddress[5]= (_loll(Dest_MAC_address[i])>>0)&0xFF;

		ALE_entries[i].portNumber= 0; //all packect are received to host port (0)

		ALE_entries[i].ucastType    = ALE_UCASTTYPE_UCAST_NOAGE;
		ALE_entries[i].secureEnable = 0;
		ALE_entries[i].blockEnable  = 0;
	}

	ale_cfg.unicastEntries= ALE_entries;
	ale_cfg.num_unicastEntries= GE_NUM_ETHERNET_PORT;
	ge_cfg.ale_cfg= &ale_cfg;
}



void TI_C6678_GE_Init()
{
	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information.
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);


	/*clear configuration structrue to make sure unused field is 0 (default value)*/
	memset(ethernet_port_cfg, 0, sizeof(ethernet_port_cfg));
	memset(&ale_cfg, 0, sizeof(ale_cfg));
	memset(&mdio_cfg, 0, sizeof(mdio_cfg));
	memset(&serdesLinkSetup, 0, sizeof(serdesLinkSetup));
	memset(&ge_cfg, 0, sizeof(ge_cfg));

	ge_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 250;//156.25;


	ge_cfg.serdes_cfg.commonSetup.loopBandwidth= SERDES_PLL_LOOP_BAND_MID;

	serdesLinkSetup.txOutputSwing    = 15; /*0~15 represents between 110 and 1310mVdfpp*/
	serdesLinkSetup.txInvertPolarity = SERDES_TX_NORMAL_POLARITY;
	serdesLinkSetup.rxLos            = SERDES_RX_LOS_DISABLE;
	serdesLinkSetup.rxAlign          = SERDES_RX_COMMA_ALIGNMENT_ENABLE;
	serdesLinkSetup.rxInvertPolarity = SERDES_RX_NORMAL_POLARITY;
	serdesLinkSetup.rxEqualizerConfig= SERDES_RX_EQ_ADAPTIVE;
    serdesLinkSetup.rxCDR            = 1;

	//MDIO is enabled in negotiation mode
	ge_cfg.mdio_cfg= &mdio_cfg;

	/*The MDIO clock can operate at up to 2.5 MHz,
	but typically operates at 1.0 MHz.*/
	mdio_cfg.clock_div= 350; 	/*350MHz/350= 1MHz*/
	mdio_cfg.link_INT0_PHY_select= MDIO_INT_SELECT_PHY_2;
	mdio_cfg.link_INT1_PHY_select= MDIO_INT_SELECT_PHY_1;


	/*Serdes lanes are enabled for internal loopback or
	when external connection is availible*/

	ge_cfg.serdes_cfg.linkSetup[0]= &serdesLinkSetup;

	ethernet_port_cfg[0].mode= ethernet_mode;
	ethernet_port_cfg[0].CPPI_Src_ID= 1;
	ethernet_port_cfg[0].RX_FIFO_Max_blocks= 8;
	ethernet_port_cfg[0].RX_flow_control_enable= TRUE;
	ethernet_port_cfg[0].TX_flow_control_enable= TRUE;
	ethernet_port_cfg[0].flow_control_MAC_Address= Source_MAC_address[0];
	ethernet_port_cfg[0].ethenet_port_statistics_enable= TRUE;
	ethernet_port_cfg[0].host_port_statistics_enable= TRUE;
	ethernet_port_cfg[0].prmiscuous_mode = ETHERNET_RX_CMF_EN;
	ge_cfg.ethernet_port_cfg[0]= &ethernet_port_cfg[0];


	ge_cfg.RX_MAX_length= 9504;

	ge_cfg.loopback_mode= ETHERNET_LOOPBACK_DISABLE;

	//setup ALE to receive all packets to host port (0)
	ALE_Entries_Init(ALE_RECEIVE_ALL);

	KeyStone_GE_Init(&ge_cfg);
	Wait_PHY_link();

	GE_PktDMA_init();

	//GE_Interrupts_Init();

	INTC_Open(48, g_VectID, GE_Message_ISR, NULL);

	GE_QMSS_Accumulation_config(0, 1, Qmss_AccPacingMode_LAST_INTERRUPT);
}



/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
  
	Source_MAC_address[0]= move_left64(g_MyMacID[0],40) | move_left64(g_MyMacID[1],32) | move_left64(g_MyMacID[2],24) | move_left64(g_MyMacID[3],16) | move_left64(g_MyMacID[4],8) | g_MyMacID[5];

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP ;// | NETIF_FLAG_IGMP;//js modified 2021.7.9

	/* Do whatever else is needed to initialize interface. */
	TI_C6678_GE_Init();

	/* set MAC hardware address */
	memcpy(netif->hwaddr, g_MyMacID, 6);
}


/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
#if 1
static struct pbuf *low_level_input(struct netif *netif)
{

	  struct pbuf *p, *q;
	  unsigned int i = 0;

	#if ETH_PAD_SIZE
	  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
	#endif

	  /* We allocate a pbuf chain of pbufs from the pool. */
	   p = pbuf_alloc(PBUF_RAW, PktFlgLen, PBUF_POOL);
	  if (p != NULL) {

	#if ETH_PAD_SIZE
	    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
	#endif

	    for(q = p; q != NULL; q = q->next)
	    {
	    	memcpy((u8_t*)q->payload, (u8_t*)&pRcvData[i], q->len);
	    	i = i + q->len;

	    	if(i>=PktFlgLen)
	    		break;
	    }
	#ifdef LINK_STATS    //协议栈统计信息
	      LINK_STATS_INC(link.recv);
	#endif/*LINK_STATS*/

	#if ETH_PAD_SIZE
	    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
	#endif

	  } else {
	#ifdef LINK_STATS    //协议栈统计信息
		  LINK_STATS_INC(link.memerr);
		  LINK_STATS_INC(link.drop);
	#endif/*LINK_STATS*/
	  }
	  return p;
}
#endif
/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
#if 1
void ethernetif_input(struct netif *netif)
{
	  struct pbuf *p;


	  /* move received packet into a new pbuf */
	  p = low_level_input(netif);
	  /* if no packet could be read, silently ignore this */
	  if (p != NULL)
	  {
	    /* pass all packets to ethernet_input, which decides what packets it supports */
	    if (netif->input(p, netif) != ERR_OK) {
	      LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
	      pbuf_free(p);
	      p = NULL;
	    }
	  }
}
#endif






//parser the received packet
void parserRxPacket(HostPacketDescriptor * hostDescriptor)
{
	/*invalid cache before read descriptor RAM*/
	InvalidCache((void *)hostDescriptor, 64);
	PktFlgLen=hostDescriptor->packet_length;
	pRcvData=(unsigned char*)hostDescriptor->buffer_ptr;
	InvalidCache((void*)(unsigned int)pRcvData,PktFlgLen);
	ethernetif_input(&g_TI6678_netif);
	InvalidCache((void*)(unsigned int)pRcvData,PktFlgLen);

#if 0
	int i=0;
	char buf[64]={0};

	Uart_Printf("receive Packet:\r\n",strlen("receive Packet:\r\n"));
	for(i=0;i<PktFlgLen;i++)
	{
		sprintf(buf,"%.2X ",pRcvData[i]);
		Uart_Printf(buf,strlen(buf));
	}
	Uart_Printf("\r\n",strlen("\r\n"));
#endif
}


void GE_Message_ISR(void* param)
{
	int i;
	Uint32 qmIntStatus0;
	Uint32 uiMask;
	Uint32 * uipAccList;
	HostPacketDescriptor * hostDescriptor;

	/*read interrupt status*/
	qmIntStatus0= gpQM_INTD_regs->STATUS_REG0;

	/*clear interrupt status*/
	gpQM_INTD_regs->STATUS_CLR_REG0= qmIntStatus0;

	/*high priority interrupt*/
	uiMask=1;
	for(i=0; i<32; i++)
	{
		if(uiMask&qmIntStatus0)
		{

			gpQM_INTD_regs->INTCNT_REG[i]= 1 ;
			gpQM_INTD_regs->EOI_REG= i+2;
			//printf("high priority queue channel %d interrupt happens at %u\n", GE_INT_TSCL);
		}
		uiMask<<=1;
	}

	/*read the acuumulation list.*/
	uipAccList= &uiaDescriptorAccumulationList[(uiAccPingPong&1)*(uiAccPageSize+1)];
	uiAccPingPong++;

	/*The first list entry is used to store the total list entry count*/
	for(i=0; i<uipAccList[0]; i++)
	{
		//read descriptor pointer
		hostDescriptor= (HostPacketDescriptor *)(uipAccList[1+i]&0xFFFFFFF0);
		if(NULL==hostDescriptor)
			continue;

		parserRxPacket(hostDescriptor);

		/*descriptor Reclamation*/
		KeyStone_queuePush(RECLAMATION_QUEUE, (Uint32)hostDescriptor|FETCH_SIZE_32);
	}

}



/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */



void Net_Send_Packet(unsigned char* pPacket, int nLen)
{
	Uint8 * ucpBuffer;
	HostPacketDescriptor * hostDescriptor;

	if(nLen < 64)
	{
		memset(pPacket+nLen,0,64-nLen);
		nLen=64;
	}
	hostDescriptor= (HostPacketDescriptor *)KeyStone_queuePop(DDR_HOST_SIZE0_FDQ);
	if(NULL==hostDescriptor)
	{
		printf("Source queue %d is NULL\n", DDR_HOST_SIZE0_FDQ);
		GE_Check_Free_Queues(); 	//for debug
		return;
	}

	/*invalid cache before read descriptor RAM*/
	InvalidCache((void *)hostDescriptor, 64);

	/*Directed packet to port. Setting these bits to a non-zero value
	indicates that the packet is a directed packet. Packets with the
	these bits set will bypass the ALE and send the packet directly
	to the port indicated.*/
	hostDescriptor->ps_flags= 1;

	/*initialize the source buffer*/
	ucpBuffer= (Uint8 *)hostDescriptor->buffer_ptr;

	memcpy(ucpBuffer,pPacket,nLen);
	hostDescriptor->packet_length= nLen;

	/*write back data from cache to descriptor RAM*/
	WritebackCache((void *)hostDescriptor, 64);
	WritebackCache((void *)ucpBuffer, nLen);

	//save descriptors to temp buffer
	TxDescriptorTempBuffer= (Uint32)hostDescriptor;

	/*push the packet descriptor to Packet DMA TX queue*/
	KeyStone_queuePush(GE_DIRECT_TX_QUEUE,
		TxDescriptorTempBuffer|FETCH_SIZE_64);


#if 0
	int i=0;
	char buf[64]={0};

	Uart_Printf("send Packet:\r\n",strlen("send Packet:\r\n"));
	for(i=0;i<nLen;i++)
	{
		sprintf(buf,"%.2X ",pPacket[i]);
		Uart_Printf(buf,strlen(buf));
	}
	Uart_Printf("\r\n",strlen("\r\n"));
#endif

}




static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q = NULL;

  u32_t templen = 0;

  for(q = p; q != NULL; q = q->next)
  {
	 memcpy((void *)&sndBufMem[sendcount%8][templen], q->payload, q->len);//将需要发送的数据从协议栈封装pbuf结构中拷贝至网口的发送buffer中
	 templen += q->len;
	 if((templen >1514)|| (templen > p->tot_len))
	 {
		 printf("Packet send err!!!\n");
		 return ERR_BUF;
	 }
	 if(templen == p->tot_len)
	 {
		 Net_Send_Packet((unsigned char*)sndBufMem[sendcount%8],templen);
	 }
  }

#ifdef LINK_STATS
  lwip_stats.link.xmit++;
#endif /* LINK_STATS */

  return ERR_OK;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

#endif
