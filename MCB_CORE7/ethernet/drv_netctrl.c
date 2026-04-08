/*
 * HwNetCtrl.c
 *
 *  Created on: 2020-11-28
 *      Author: zhty
 */

#include "../drv_netctrl.h"

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "arch/cc.h"

#include "./src/KeyStone_common.h"


#define NET_WORK_UDP  0
#define NET_WORK_TCP  1

unsigned int g_VectID = 5;

unsigned char g_Type = NET_WORK_UDP;

//自定义的网口mac地址，LWIP底层代码需要引用的全局变量，注意不要修改名称
unsigned char g_MyMacID[6]={0x01,0x80,0x48,0x12,0x34,0x56};

//LWIP底层代码需要引用的全局变量，注意不要修改名称
struct netif g_TI6678_netif;

struct udp_pcb* g_udpPcb = NULL;//创建UDP服务器的控制块
struct tcp_pcb*   g_tcpPcb = NULL;//创建TCP服务器的控制块
struct tcp_pcb*   g_tcpSeverListenSocketPcb = NULL;//创建TCP服务器监听控制器的控制块

static funOnNetRecv g_funOnNetRecv = NULL;	//给上层模块的接收处理回调函数的指针
static void* g_pfunParam = NULL;	//给上层模块的接受处理回调函数的参数


//LWIP底层代码的函数，初始化时需要引用（原样照抄的demo）
extern  err_t ethernetif_init(struct netif *netif);
//------------------------------------------数据传输相关   start      ------------------------
typedef struct{
    MSG_TYPE type;               //当前消息类型
    unsigned int     tot_len;          //消息的裸数据总长度, 序号0裸数据长度+序号1裸数据长度+......
//    unsigned short  seq;         //数据分块序号,从0开始
    unsigned short  len;             //本消息长度
//    unsigned short  cur_len;   //本块数据长度
    unsigned short  recved;   //被分块之后已经收到的长度
}DataTransStatus_st, *pDataTransStatus_st;
DataTransStatus_st  g_lastDataTransStatus = { MSG_TYPE_RESERVED,0,0,0 };
uint8_t g_isMsgBlockFinished = 0;  //块数据传输完成标识0未完成,1完成


#pragma DATA_SECTION(g_packetBuffer_DDR_Seq,   "PacketData.buffer_DDR")
#pragma DATA_SECTION(g_packetBuffer_DDR_Update,"PacketData.buffer_DDR")
Uint8 g_packetBuffer_DDR_Seq[1024*9] = {0};  //单条数据缓存
Uint8 g_packetBuffer_DDR_Update[1024*1024*4] = {0};  //升级数据缓存
unsigned int  g_packetBuffer_DDR_Seq_Index = 0;


//------------------------------------------数据传输相关   end      ------------------------
//-----------------------------------------错误处理相关    start      ------------------------
typedef struct {
    int ec;
    const char *str;
} ErrorString_st, *pErrorString_st;

static const  ErrorString_st faults[] = {
    {E_OK, "Success"},
    {E_FAIL, "General Failed"},
    {E_INNER, "Internal error"},
    {E_POINTER, "Invalid Pointer"},
    {E_INVALARG, "Invalid argument"},
    {E_NOTIMPL, "Not implemented"},
    {E_OUTOFMEM, "Out of memory"},
    {E_BUFERROR, "Buffer error"},
    {E_PERM, "Permission denied"},
    {E_TIMEOUT, "Timed out"},
    {E_NOTINIT, "Object not init"},
    {E_INITFAIL, "Object init failed"},
    {E_ALREADY, "Operation already in progress"},
    {E_INPROGRESS, "Operation now in progress"},
    {E_EXIST, "Object exist"},
    {E_NOTEXIST, "Object not exist"},
    {E_BUSY, "Device or resource busy"},
    {E_FULL, "Device or resource full"},
    {E_EMPTY, "Device or resource empty"},
    {E_OPENFAIL, "Device or resource open failed"},
    {E_READFAIL, "Device or resource read failed"},
    {E_WRITEFAIL, "Device or resource write failed"},
    {E_DELFAIL, "Device or resource delete failed"},
    {E_CODECFAIL, "Encode or decode failed"},
    {E_CRC_FAIL, "CRC failed"},
    {E_TOOMANY, "Object too many"},
    {E_TOOSMALL, "Object too small"},
    {E_NETNOTREACH, "Network is unreachable"},
    {E_NETDOWN, "Network is down"},
    {E_MSGTYPE_UNKNOW, "Message type error"},
    {E_END, "Unknow error"},
};


const char* GetErrString(int errorcode)
{
    if (errorcode < 0) errorcode = -errorcode;
    int i = 0;
    for ( ; i < sizeof(faults)/sizeof(faults[0]); i++)
        if (faults[i].ec == errorcode)
            return faults[i].str;
    return "Error code unknown";
}
//修改的strstr, 字符串搜索, 依赖长度
static char* strnstr( const char * src, uint32_t src_len, const char * substr, uint32_t substr_len )
{
    const char * p;
    const char * pend;

    if ( src == NULL || 0==*src || substr == NULL || 0==*substr || src_len <= 0 || substr <= 0 )
    {
        return NULL;
    }

    p = src;
    pend = p + src_len - substr_len;
    while ( p < pend )
    {
        if ( *p == *substr )
        {
            if ( memcmp( p, substr, substr_len ) == 0)
            {
                return (char*)p;
            }
        }
        p++;
    }
    return NULL;
}
//逆序版查找字符串, 依赖长度
static char* strnstr_r( const char * src, uint32_t src_len, const char * substr, uint32_t substr_len )
{
    const char * p;
    const char * pend;

    if ( src == NULL || 0==*src || substr == NULL || 0==*substr || src_len <= 0 || substr <= 0 )
    {
        return NULL;
    }

    p = src + src_len - substr_len;
    pend = src;
    while ( p >= pend )
    {
        if ( *p == *substr )
        {
            if ( memcmp( p, substr, substr_len ) == 0)
            {
                return (char*)p;
            }
        }
        p--;
    }
    return NULL;
}

///pbuf链表数据提取
//   p:  数据包pbuf链表;    pBuffer : 数据接收缓冲区;  maxLen : 最大接收长度
// 返回 实际接收长度
static uint32_t GetPbufPayload(struct pbuf* p, uint8_t* pBuffer, uint32_t  maxLen)
{
    uint32_t remainLen;
    uint32_t readLen = 0;
    //遍历链表
    for (; p != NULL;  p = p->next)
    {
        remainLen = maxLen - readLen ;//
        if (p->len >= remainLen)
        {
            memcpy(&pBuffer[readLen], p->payload, remainLen);
            readLen += remainLen;
            break;
        }
        memcpy(&pBuffer[readLen],  p->payload, p->len);;
        readLen += p->len;
    }
    return readLen;
}
//-----------------------------------------错误处理相关    end      ------------------------

//工具函数: uint型IP地址 ==>字符串
void GetIpFormUint(unsigned int nIp, unsigned char pIpGet[4])
{
    pIpGet[0] = (unsigned char)(nIp & 0x000000FF);
    pIpGet[1] = (unsigned char)((nIp & 0x0000FF00) >> 8);
    pIpGet[2] = (unsigned char)((nIp & 0x00FF0000) >> 16);
    pIpGet[3] = (unsigned char)((nIp & 0xFF000000) >> 24);
}


//LWIP的UDP接收回调函数
void OnUdpDataRecved(void *arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port)
{
	unsigned char pIp[4] = { 0 };

	if (NULL == p)
	{
		return;
	}

	if (NULL == g_funOnNetRecv)
	{
		pbuf_free(p);
		return;
	}

	if (p->tot_len != p->len)	//目前我们处理的都是单包UDP报文，不会超过最大链路层的MAC包长，所里这里如此判断（临时测试，不知道底层的内存分配机制）
	{
	    printf("p->tot_len != p->len\r\n");
		pbuf_free(p);
		return;
	}

	GetIpFormUint(addr->addr, pIp);
	g_funOnNetRecv(pIp, port, (unsigned char*)p->payload, (int)p->len, g_pfunParam);

	pbuf_free(p);
}

//LWIP的TCP接收回调函数
err_t OnTcpDataRecved(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	int i=0;
	unsigned char pIp[4] = { 0 };

	if (NULL == p)
	{
		return 0;
	}

	if (NULL == g_funOnNetRecv)
	{
		pbuf_free(p);
		return 0;
	}

	if(p != NULL)
	{
		for(i = 0; i < 4; i++)
		{
			pIp[i] = *((unsigned char*)&tpcb->remote_ip.addr + i);
		}
        //TODO 协议分析
		{
		     uint32_t  uReadLen = GetPbufPayload(p, g_packetBuffer_DDR_Seq,  sizeof(g_packetBuffer_DDR_Seq));

             int nStartPos = 0;
             while((nStartPos  +  sizeof(MsgHeader_st))<= uReadLen)
             {
                 MsgPacket_st* pPayload = (MsgPacket_st*)((unsigned char*)g_packetBuffer_DDR_Seq + nStartPos);
                 uint8_t  isNewSeq = 0;
                 isNewSeq =  ((pPayload->head.magic == 0x3c4d) && (g_lastDataTransStatus.recved == 0) &&\
                         (pPayload->head.type>=(unsigned short) E_OK && pPayload->head.type<(unsigned short)E_END));

                  //g_lastDataTransStatus.recved == g_lastDataTransStatus.seq_len;
                 //数据从0x3C 0x4D开始的
                 if (isNewSeq)
                 {
                     int nLen = 0;//本包中实际所含数据长度
                     unsigned char* pPos = NULL;
                     int nRemainLen = uReadLen - nStartPos;
                     if ( nRemainLen <  sizeof(MsgPacket_st)) //包被截断了
                     {
                         g_lastDataTransStatus.recved = nRemainLen;
                         //len = g_lastDataTransStatus.recved = remainLen - ((payload->buff) - &(payload->head.magic));
                         int nTmp = sizeof(MsgPacket_st) -sizeof(uint32_t) - sizeof(uint8_t*);
                         if (nRemainLen > nTmp)//说明有数据
                         {
                             pPos = pPayload->buff;
                             //复制保存数据到别处
                             nLen = nRemainLen - nTmp;
                             if (pPayload->head.type == MSG_TYPE_UPDATE)
                             {
                                 memcpy((char*)g_packetBuffer_DDR_Update+g_packetBuffer_DDR_Seq_Index, pPos, nLen );
                                 g_packetBuffer_DDR_Seq_Index += nLen;
                                 if (g_packetBuffer_DDR_Seq_Index >= pPayload->tot_len)
                                     ;//TODO   由外界函数来处理升级的事情
                             }
                         }
                         else{  //没有数据, 只有部分协议字段
                             pPos = NULL;
                             nLen = 0;
                         }
                         //remainLen = 0;
                         nStartPos = uReadLen;
                     }
                     else  //可能至少一个完整的包
                     {
                         //逆序找结束标识
                         char* pTail = strnstr_r( (const char*)pPayload, pPayload->len ,  "\r\n\r\n", strlen("\r\n\r\n") );
                         if (pTail != NULL) //找到了结束符, 是一个完整包
                         {
                             g_lastDataTransStatus.recved = pPayload->len - sizeof(MsgPacket_st) + sizeof(uint8_t*)- strlen("\r\n\r\n");


                              nLen = g_lastDataTransStatus.recved;//有效数据长度
                              pPos = pPayload->buff;
                              nStartPos += pPayload->len;
                              if (pPayload->head.type == MSG_TYPE_UPDATE)
                              {
                                  //memcpy((char*)g_packetBuffer_DDR_Update + g_packetBuffer_DDR_Seq_Index,payload->buff, len);
                                  int iTmp = 0;
                                  for (; iTmp < nLen; iTmp++)
                                  {
                                      g_packetBuffer_DDR_Update[iTmp + g_packetBuffer_DDR_Seq_Index] = ((char*)(&pPayload->len + 1))[iTmp];
                                  }
                                  g_packetBuffer_DDR_Seq_Index += nLen;
                                  if (g_packetBuffer_DDR_Seq_Index >= pPayload->tot_len)
                                      ;//TODO   由外界函数来处理升级的事情
                              }
                              g_lastDataTransStatus.recved = 0;
                         }
                         else//包数据被截断了
                         {
                             ////////////////////////
                             g_lastDataTransStatus.recved = nRemainLen;
                              //len = g_lastDataTransStatus.recved = remainLen - ((payload->buff) - &(payload->head.magic));
                              int tmp = sizeof(MsgPacket_st) -sizeof(uint32_t) - sizeof(uint8_t*);
                              if (nRemainLen > tmp)//说明有数据
                              {
                                  pPos = pPayload->buff;
                                  //复制保存数据到别处
                                  nLen = nRemainLen - tmp;
                                  if (pPayload->head.type == MSG_TYPE_UPDATE)
                                  {
                                      memcpy((char*)g_packetBuffer_DDR_Update+g_packetBuffer_DDR_Seq_Index, pPos, nLen );
                                      g_packetBuffer_DDR_Seq_Index += nLen;
                                      if (g_packetBuffer_DDR_Seq_Index >= pPayload->tot_len)
                                          ;//TODO   由外界函数来处理升级的事情
                                  }
                              }
                              else{  //没有数据, 只有部分协议字段
                                  pPos = NULL;
                                  nLen = 0;
                              }
                              //remainLen = 0;
                              nStartPos = uReadLen;
                             ///////////////////////
                         }
                     }
                     //      if (g_lastDataTransStatus.type != MSG_TYPE_UPDATE)
                     //          printf("hello, \r\n");
                     //          g_funOnNetRecv(pIp, tpcb->remote_port, (unsigned char*)g_packetBuffer_DDR_Seq, (int)readLen, g_pfunParam);

                     g_lastDataTransStatus.type = (MSG_TYPE)pPayload->head.type;
                     g_lastDataTransStatus.tot_len = pPayload->tot_len;
                     g_lastDataTransStatus.len = pPayload->len /*- sizeof(MsgPacket_st) + sizeof(uint8_t*) - strlen("\r\n\r\n")*/;;
                 }
                 else//数据被截断了
                 {
                     int len = (g_lastDataTransStatus.len - g_lastDataTransStatus.recved );
                     if (g_lastDataTransStatus.type == MSG_TYPE_UPDATE )
                     {
                         memcpy((char*)g_packetBuffer_DDR_Update + g_packetBuffer_DDR_Seq_Index, 0, len);
                         g_packetBuffer_DDR_Seq_Index += len;
                         if (g_packetBuffer_DDR_Seq_Index >= pPayload->tot_len)
                             ;//TODO   由外界函数来处理升级的事情
                     }
                     else
                     {
                         g_funOnNetRecv(pIp, tpcb->remote_port, (unsigned char*)g_packetBuffer_DDR_Seq, (int)uReadLen, g_pfunParam);
                     }
                     nStartPos += (len + sizeof(uint32_t));
                     g_lastDataTransStatus.recved = g_lastDataTransStatus.len;
                 }
                 if (g_lastDataTransStatus.recved == g_lastDataTransStatus.len)
                 {
                     g_lastDataTransStatus.recved = 0;
                 }
             } //end of while((nStartPos  +  sizeof(struct MsgHeader_st))<= readLen)

		}

		tcp_recved(tpcb,p->tot_len);//数据处理后需要调用,更新接收窗口

		pbuf_free(p);
	}
	if (tpcb->state != ESTABLISHED) //连接断开了
	{
	    printf("Disconnected! \r\n");
	}
	if(err == ERR_OK)
	{
		tcp_recved(tpcb,p->tot_len);
        printf("%s   recved error. \r\n", __FUNCTION__);
        //客户端可重新初始化后开始连接目标服务器
		return tcp_close(tpcb);
	}


	return ERR_OK;
}


static err_t OnTcpClientConnected(void *arg, struct tcp_pcb *pcb, err_t err)
{
	tcp_recv(pcb, OnTcpDataRecved);//指定收到数据的回调函数
	/* 发送一个建立连接的问候字符串*/
	//char clientString[]="This is a new client connection.\r\n";
	//tcp_write(pcb,clientString, strlen(clientString),0);
	return ERR_OK;
}


static err_t OnTcpServerAccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	tcp_recv(pcb, OnTcpDataRecved);//指定收到数据的回调函数
	return ERR_OK;

}
//回调函数
static void OnTcpConnError(void *arg, err_t err)
{
    printf("Connect Failed:  %c\r\n", (unsigned  char)err);
    //对于TCP客户端连接服务器错误回调函数，它是tcp_err_fn类型，
    //在这个程序中主要完成连接异常结束时的一些处理，可以释放一些必要的资源。
    //在这个函数被内核调用时，连接实际上已经断开，相关控制块也已经被删除。
    //所以在这个函数中我们可以重新初始化连接及其资源。在这里额我们就是使用它来重新初始化TCP客户端。
}
static void CloseTcpRaw(struct tcp_pcb* pcb)
{
    if (pcb == NULL)
            return;
      tcp_recv(pcb, NULL);
      tcp_close(pcb);
      /* closing succeeded */
      tcp_arg(pcb, NULL);
      tcp_sent(pcb, NULL);
}
void HwNet_CloseTCPServer()
{
    CloseTcpRaw(g_tcpSeverListenSocketPcb);
}

void  HwNet_CloseTCPConn( )
{
    CloseTcpRaw(g_tcpPcb);
}



//UDP初始化
int HwNet_Udp_Init(unsigned char pMac[6], unsigned char pIp[4], unsigned char pMask[4], unsigned short nPort, unsigned int nVectID, funOnNetRecv pOnRecv, void* pFunParam)
{
	g_Type = NET_WORK_UDP;
	g_VectID = nVectID;
	ip4_addr_t local_ip;									//本地的IP地址
	ip4_addr_t netmask;										//子网掩码
	ip4_addr_t gw;											//默认网关
	int i = 0;

	for (i = 0; i < 6; i++)
	{
		g_MyMacID[i] = pMac[i];
	}

	g_pfunParam = pFunParam;
	g_funOnNetRecv = pOnRecv;


	/*使能DSP的节拍计数器，用于协议栈的内核时钟，此为必须代码，在TCP协议栈中尤其重要*/
	TSCL=0;
	TSCH=0;

	lwip_init();

	 //调用内核函数，初始化内核
	IP4_ADDR(&gw, pIp[0], pIp[1], pIp[2], 1); 				//网关地址
	IP4_ADDR(&local_ip, pIp[0], pIp[1], pIp[2], pIp[3]);		//开发板IP地址
	IP4_ADDR(&netmask, pMask[0], pMask[1], pMask[2], pMask[3]);	//子网掩码

	netif_add(&g_TI6678_netif, &local_ip, &netmask, &gw, NULL, ethernetif_init,ethernet_input);
	netif_set_default(&g_TI6678_netif);  			//设置协议找栈默认网络接口为 6678_netif
	netif_set_up(&g_TI6678_netif);  				//使能该网络接口

	
	g_udpPcb = udp_new();
	udp_bind(g_udpPcb, &local_ip, nPort);
	udp_recv(g_udpPcb, OnUdpDataRecved, NULL);

	return 0;
}


//UDP发送数据
int HwNet_SendUdpData( unsigned char pIpDest[4], unsigned short nPortDest, unsigned char* pData, unsigned int nDataLen)
{
	if (NET_WORK_TCP == g_Type)
	{
		return -1;
	}

	err_t temp_err = 0;
	struct pbuf *udp_pbuf = NULL;
	ip4_addr_t addr;

	/*TODO:注意：这个封装UDP数据包的过程是必须的，它将应用层的数据封装成UDP的数据结构，否则将导致UDP的数据包无法发送
	* 		请不要修改PBUF_TRANSPORT，PBUF_RAM这两个参数；否则将导致严重后果
	* 		PBUF_TRANSPORT PBUF_RAM类型的pbuf结构为非链状的pbuf，此pbuf->len == pbuf->tot_len
	*/


	udp_pbuf = pbuf_alloc(PBUF_TRANSPORT, nDataLen, PBUF_RAM); //申请需要发送的UDP数据包结构
	if(NULL == udp_pbuf)
	{
		return -1;
	}
	memcpy(udp_pbuf->payload, pData, udp_pbuf->tot_len);			//将发送数据拷贝到udp_pbuf->payload

	IP4_ADDR(&addr, pIpDest[0], pIpDest[1], pIpDest[2], pIpDest[3]);
	temp_err=udp_sendto(g_udpPcb , udp_pbuf, &addr, nPortDest); //调用udp_send向远端发送数据，也可以使用udp_sendto函数，两个函数的区别请看注释
	if(ERR_OK != temp_err)
	{
		pbuf_free(udp_pbuf);
		return -1;
	}

	pbuf_free(udp_pbuf);

	return 0;
}


//TCP初始化
int HwNet_Tcp_Init(NETWORK_TCP_TYPE nType, unsigned char pLocalMac[6], unsigned char pLocalIp[4], unsigned char pLocalMask[4], unsigned short nLocalPort,
	unsigned char pRemoteIp[4], unsigned short nRemotePort, unsigned int nVectID, funOnNetRecv pOnRecv, void * pFunParam)
{
	unsigned int nWatie = 0xFFFFFF;
	g_Type = NET_WORK_TCP;
	g_VectID = nVectID;

	ip4_addr_t serverIp;                                //作客户端时, 对方服务器的IP地址
	ip4_addr_t local_ip;									//本地的IP地址
	ip4_addr_t netmask;										//子网掩码
	ip4_addr_t gateway;											//默认网关
	int i = 0;

	for (i = 0; i < 6; i++)
	{
		g_MyMacID[i] = pLocalMac[i];
	}

	g_pfunParam = pFunParam;
	g_funOnNetRecv = pOnRecv;


	/*使能DSP的节拍计数器，用于协议栈的内核时钟，此为必须代码，在TCP协议栈中尤其重要*/
	TSCL = 0;
	TSCH = 0;

	//调用内核函数，初始化内核
	lwip_init();

	IP4_ADDR(&gateway, pLocalIp[0], pLocalIp[1], pLocalIp[2], 1); 				//网关地址
	IP4_ADDR(&local_ip, pLocalIp[0], pLocalIp[1], pLocalIp[2], pLocalIp[3]);		//开发板IP地址
	IP4_ADDR(&netmask, pLocalMask[0], pLocalMask[1], pLocalMask[2], pLocalMask[3]);	//子网掩码

	netif_add(&g_TI6678_netif, &local_ip, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
	netif_set_default(&g_TI6678_netif);  			//设置协议找栈默认网络接口为 6678_netif
	netif_set_up(&g_TI6678_netif);  				//使能该网络接口
	if (nType != NETWORK_TCP_CLIENT)   //作为服务端
	{
	    HwNet_CloseTCPConn( );
	    HwNet_CloseTCPServer();

	    /* 为tcp服务器分配一个tcp_pcb结构体 */
	    g_tcpSeverListenSocketPcb = tcp_new();
		/* 绑定本地端号和IP地址 */
		tcp_bind(g_tcpSeverListenSocketPcb, IP_ADDR_ANY, nLocalPort);

    	while(nWatie)
    	{
    		nWatie--;
    	}
		if (g_tcpSeverListenSocketPcb != NULL)
		{
		    /* 监听之前创建的结构体g_tcpSeverListenSocketPcb, 不可逆 */
            g_tcpSeverListenSocketPcb = tcp_listen(g_tcpSeverListenSocketPcb);

            /* 初始化结构体接收回调函数 */
            tcp_accept(g_tcpPcb, OnTcpServerAccept);
		}
	}
	else    //做客户端
	{
	    /* 将目标服务器的IP写入一个结构体，为本地连接IP地址 */
		IP4_ADDR(&serverIp, pRemoteIp[0], pRemoteIp[1], pRemoteIp[2], pRemoteIp[3]);
		/* 为tcp客户端分配一个tcp_pcb结构体    */
		g_tcpPcb = tcp_new();
		/* 绑定本地端号和IP地址 */
		tcp_bind(g_tcpPcb, &local_ip, nLocalPort);


		while(nWatie)
		{
			nWatie--;
		}
		/* 与目标服务器进行连接，参数包括了目标端口和目标IP */
		if (tcp_connect(g_tcpPcb, &serverIp, nRemotePort, OnTcpClientConnected))
		{
			printf("Connect Failed\r\n");
			return -1;
		}
		tcp_err(g_tcpPcb, OnTcpConnError);
	}


	return 0;
}

//TCP发送数据
int HwNet_SendTcpData(unsigned char * pData, unsigned int nDataLen)
{
	if (NET_WORK_UDP == g_Type)
	{
		return -1;
	}

	struct tcp_pcb* pcb = g_tcpPcb;

	tcp_write(pcb, pData, nDataLen, TCP_WRITE_FLAG_COPY);
	tcp_output(pcb);

	return 0;
}



//static ERR_TYPE  ResolvePacket(unsigned char* pData, int nLen, void *pFunParam)
//{
//    if (NULL == pData || 0 == nLen)
//        return E_EMPTY;
//    if( sizeof(MsgPacket_st) > nLen)
//        return E_TOOSMALL;
//    struct pMsgPacket_st *  pMsg = (struct pMsgPacket_st *)pData;
//    if (pMsg->head.type == MSG_TYPE_UPDATE)
//    {
//        //拷贝升级消息的数据部分到缓冲区
//    }
//    else if (pMsg->head.type == MSG_TYPE_RESERVED)
//    {
//
//    }
//
//    return  E_OK ;
//}
