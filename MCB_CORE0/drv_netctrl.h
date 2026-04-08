/*
 * drv_netctrl.h
 *
 *  Created on: 2020-11-28
 *      Author: zhty
 */

#ifndef DRV_NETCTRL_H_
#define DRV_NETCTRL_H_

typedef enum
{
    NETWORK_TCP_CLIENT,  //TCP客户端
    NETWORK_TCP_SERVER, //TCP服务器
}NETWORK_TCP_TYPE;


#pragma pack(1)
typedef struct {     //消息头结构体
    unsigned short     magic;     //固定数0x3C4D
    unsigned short     type;       //消息类型
    unsigned short     version;   //消息版本
    unsigned short     appid;     //应用标识
    unsigned char     srcMac[6];     //源设备mac地址
    unsigned char     dstMac[6];    //目标mac地址
}MsgHeader_st, *pMsgHeader_st;
typedef struct{
    MsgHeader_st   head;
    unsigned int       tot_len;          //消息的裸数据总长度, 序号0裸数据长度+序号1裸数据长度+......
//    unsigned short  seq;         //数据分块序号,从0开始
    unsigned int    len;   //本消息长度
//    unsigned short  cur_len;   //传输的本块数据长度

    unsigned char *  buff;       //数据, 指示起始位置, 结尾0x0d 0x0a 0x0d 0x0a
//    unsigned int  tail;        //0x0d 0x0a 0x0d 0x0a
}MsgPacket_st, *pMsgPacket_st;
#pragma pack ()

typedef enum{
    MSG_TYPE_RESERVED,     //0,  保留
    MSG_TYPE_UPDATE,        //升级
    MSG_TYPE_DISCONN,     //断开
    MSG_TYPE_RETRANS,     //重传请求
    //MSG_TYPE_PINGREQ,      //心跳请求
    //MSG_TYPE_PINGRSP,      //心跳响应
    //MSG_TYPE_ECHO,          //回声测试
    //MSG_TYPE_DEV_NAME, //设备名
    //MSG_TYPE_DEV_ID,       //设备标识
    //MSG_TYPE_RESET,        //恢复出厂设置

}MSG_TYPE;
typedef enum{
    E_OK,                 ///< 成功
    E_FAIL,               ///< 一般性错误
    E_INNER,           ///< 内部错误（一般在同一个模块内使用，不对外公开
    E_POINTER,        ///< 非法指针
    E_INVALARG,       ///< 非法参数
    E_NOTIMPL    ,    ///< 功能未实现
    E_OUTOFMEM,       ///< 内存申请失败/内存不足
    E_BUFERROR,       ///< 缓冲区错误（不足，错乱）
    E_PERM,           ///< 权限不足，访问未授权的对象
    E_TIMEOUT,        ///< 超时
    E_NOTINIT ,        ///< 未初始化
    E_INITFAIL,       ///< 初始化失败
    E_ALREADY,        ///< 已初始化，已经在运行
    E_INPROGRESS,     ///< 已在运行、进行状态
    E_EXIST,          ///< 申请资源对象(如文件或目录)已存在
    E_NOTEXIST,       ///< 资源对象(如文件或目录)、命令、设备等不存在
    E_BUSY,           ///< 设备或资源忙（资源不可用）
    E_FULL,           ///< 设备/资源已满
    E_EMPTY,          ///< 对象/内存/内容为空
    E_OPENFAIL,       ///< 资源对象(如文件或目录、socket)打开失败
    E_READFAIL,       ///< 资源对象(如文件或目录、socket)读取、接收失败
    E_WRITEFAIL,      ///< 资源对象(如文件或目录、socket)写入、发送失败
    E_DELFAIL,           ///< 资源对象(如文件或目录、socket)删除、关闭失败
    E_CODECFAIL,       ///< 加解密、编码解密失败
    E_CRC_FAIL,           ///< crc校验错误
    E_TOOMANY,         ///< 消息、缓冲区、内容太多
    E_TOOSMALL,        ///< 消息、缓冲区、内容太少
    E_NETNOTREACH,    ///< 网络不可达（无路由，网关错误）
    E_NETDOWN,         ///< 网络不可用（断网）

    E_MSGTYPE_UNKNOW,  ///<网络消息类型错误
    E_END,            ///< 占位，无实际作用
}ERR_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 网口接收回调函数
 */
typedef void (*funOnNetRecv)(unsigned char pIp[4],unsigned short nPort, unsigned char* pData, int nLen, void* pFunParam);


/*
 * UDP通信
 */
extern int HwNet_Udp_Init(unsigned char  pMac[6],    //DSP设备MAC地址 
						  unsigned char  pIp[4],     //DSP设备IP地址
						  unsigned char  pMask[4],   //DSP设备掩码
						  unsigned short nPort,      //DSP设备端口号
						  unsigned int   nVectID,    //网口接收中断号
						  funOnNetRecv   pOnRecv,    //网口接收回调函数
						  void*          pFunParam); //网口接收回调函数参数

extern int HwNet_SendUdpData(unsigned char pIpDest[4], unsigned short nPortDest, unsigned char* pData, unsigned int nDataLen);


/*
 * TCP通信
 */
extern int HwNet_Tcp_Init(NETWORK_TCP_TYPE  nType,         //TCP类型，服务器/客户端
						  unsigned char  pLocalMac[6],  //DSP设备MAC地址
						  unsigned char  pLocalIp[4],   //DSP设备IP地址
						  unsigned char  pLocalMask[4], //DSP设备掩码
						  unsigned short nLocalPort,    //DSP设备端口号
						  unsigned char  pRemoteIp[4],  //对端IP地址，仅客户端有效
						  unsigned short nRemotePort,   //对端端口号，仅客户端有效
						  unsigned int   nVectID,       //网口接收中断号
						  funOnNetRecv   pOnRecv,       //网口接收回调函数
						  void*          pFunParam);    //网口接收回调函数参数

extern int HwNet_SendTcpData(unsigned char* pData, unsigned int nDataLen);


#ifdef __cplusplus
}
#endif

#endif /* HWNETCTRL_H_ */
