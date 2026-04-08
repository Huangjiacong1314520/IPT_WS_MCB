#ifndef _INTERFACE_DRIVE_H
#define _INTERFACE_DRIVE_H

#include "drv_srio.h"

unsigned int Syn422Serial;
unsigned int Asyn422Serial;
unsigned int Can0Serial;
unsigned int Can1Serial;
unsigned int CamLinkSerial;

//函数返回值
#define EC_OK						 0 
#define EC_PARAM1_ERROR				-1   //参数1错误
#define EC_PARAM2_ERROR				-2   //参数2错误
#define EC_PARAM3_ERROR				-3   //参数3错误
#define EC_SRIO_WRITE_ERROR		    -4   //SRIOWrite失败

#define EC_SYS_VER               0
#define EC_SYN422_RECVE_DATA     1
#define EC_HDSYNC_RECVE_DATA     2
#define EC_ASYN422_RECVE_DATA    3
#define EC_CAN0_RECVE_DATA       4
#define EC_CAN1_RECVE_DATA       5
#define EC_CAMLINK_RECVE_DATA    6

#pragma pack (4)
typedef struct _RECVE_DATA_MANAGER_STRUCT_
{
	char blDataping;
	char blDatapong;
	char blping;
	char Reserver;
	unsigned char* pRxFifoPing;			//接收缓存1
	unsigned char* pRxFifoPong;        //接收缓存2
}RecveData_st, *pRecveData_st;


typedef struct _IR_CAN_MESSAGE_STRUCT_
{
	unsigned int	nId;		/*消息ID*/
	unsigned char	blExtId;	/*0---标准帧，1---扩展帧*/
	unsigned char	Reserve;
	unsigned short	len;		/*数据长度*/
	unsigned char	pData[8];	/*数据buf*/

} EC_CanMsg_st, *pEC_CanMsg_st;
#pragma pack ()


#ifdef __cplusplus
extern "C" {
#endif


/**********EC_OpenDevice函数说明************/
//功能：打开设备，初始化SRIO
//返回值：返回设备句柄EC1900DEV：
void EC_OpenDevice();



/**********GetSysVerInfo函数说明************/
//功能：获取版本信息
//参数---pData：接收数据指针
//参数---nLen：接收数据数组长度
//返回值：成功返回接收数据长度,失败返回错误码
int GetSysVerInfo(unsigned char* pData, unsigned int nLen);



/**********Syn422_RecvData函数说明************/
//功能：同步422接收数据
//参数---pData：接收数据指针
//参数---nLen：接收数据数组长度
//返回值：成功接收数据长度
int Syn422_RecvData(unsigned char* pData, unsigned int nLen);



/**********Syn422_SendData函数说明************/
//功能：同步422发送数据
//参数---pData：发送数据指针(内容为2字节的ADDR + 27字节的DATA)
//参数---nLen：发送数据数组长度
//返回值：成功返回EC_OK,失败返回错误码
int Syn422_SendData(unsigned char* pData, unsigned int nLen);




/**********HDSYNC_RecvData函数说明************/
//功能：硬同步接收数据
//参数---pData：接收数据指针
//参数---nLen：接收数据数组长度
//返回值：成功接收数据长度
int HDSYNC_RecvData(unsigned char* pData, unsigned int nLen);




/**********Asyn422_RecvData函数说明************/
//功能：异步422接收数据
//参数---pData：接收数据指针
//参数---nLen：接收数据数组长度
//返回值：成功接收数据长度
int Asyn422_RecvData(unsigned char* pData, unsigned int nLen);



/**********Asyn422_SendData函数说明************/
//功能：异步422发送数据
//参数---pData：发送数据指针（内容为1字节的ID + 6字节的DATA）
//参数---nLen：发送数据数组长度
//返回值：成功返回EC_OK,失败返回错误码
int Asyn422_SendData(unsigned char* pData, unsigned int nLen);



/**********Can_RecvData函数说明************/
//功能：Can接收数据
//参数---nChl：通道号，（0--1）
//参数---pData：接收数据指针
//参数---nLen：接收数据数组长度
//返回值：成功接收数据长度
int Can_RecvData(char nChl, pEC_CanMsg_st pCanMsg);



/**********Can_SendData函数说明************/
//功能：Can发送数据
//参数---nChl：通道号，（0--1）
//参数---pCanMsg：Can发送数据结构指针
//返回值：成功返回EC_OK,失败返回错误码
int Can_SendData(char nChl, pEC_CanMsg_st pCanMsg);



/**********CamLink_RecvData函数说明************/
//功能：图像传输接收数据
//参数---pData：接收数据指针
//参数---nLen：接收数据数组长度
//返回值：成功接收数据长度
int CamLink_RecvData(unsigned char* pData, unsigned int nLen);



/**********Syn485_SendData函数说明************/
//功能：同步485发送数据
//参数---pData：发送数据指针（内容为3072字节的DATA）
//参数---nLen：发送数据数组长度
//返回值：成功返回EC_OK,失败返回错误码
int Syn485_SendData(unsigned char* pData, unsigned int nLen);




/**********SendSubtitleInfo函数说明************/
//功能：发送字幕信息
//参数---pData：发送字幕信息内容40*256
//参数---nLen：发送数据数组长度
//返回值：成功返回EC_OK,失败返回错误码
int SendSubtitleInfo(unsigned char* pData, unsigned int nLen);




#ifdef __cplusplus
}
#endif


#endif
