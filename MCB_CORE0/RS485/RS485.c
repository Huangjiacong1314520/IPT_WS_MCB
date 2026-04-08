#include <string.h>
#include <stdio.h>
#include "RS485.h"
#include "../srio/srio.h"

/****************************************************************************
SfpRegisterRead: 通过SFP读取AD/DA/光栅卡的寄存器
    nSfpCh:     SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nAddr：                     寄存器地址，AD/DA/光栅卡上寄存器地址
             返回值：                          寄存器值
****************************************************************************/
int SfpRegisterRead(unsigned char nSfpCh, unsigned int nAddr)
{
    int i = 0;
    unsigned char SrioReadBuf[256] = {0};
    unsigned int SfpRegVal = 0;
    unsigned int SrioAddr = 0;

    SrioAddr = 0x100000 + (nSfpCh << 16) + (nAddr << 8);

    SRIO_ReadData(0, SrioAddr, SrioReadBuf, 256);
    //添加延迟，否则读取会出问题
    for(i = 0; i < 1000; i++)
    {
        i++;
    }

    memcpy(&SfpRegVal, SrioReadBuf+12, 4);

    return SfpRegVal;
}

/****************************************************************************
SfpRegisterWrite: 通过SFP写AD/DA/光栅卡的寄存器
    nSfpCh:     SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nAddr：                     寄存器地址，AD/DA/光栅卡上寄存器地址
    nVal：                        写入寄存器值
             返回值：                          1
****************************************************************************/
int SfpRegisterWrite(unsigned char nSfpCh, unsigned int nAddr, unsigned int nVal)
{
    unsigned char SrioSendBuf[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0xCF};
    SrioSendBuf[4] = nAddr & 0x7F;
    SrioSendBuf[5] |= nAddr >> 7;
    memcpy(SrioSendBuf+12, &nVal, 4);
    SRIO_WriteData(0, (0x100000 + (nSfpCh << 16)), SrioSendBuf, 16);

    return 1;
}

/****************************************************************************
OgRs485ReadReg:     读RS485寄存器值
    nSfpCh:         SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nRs485Ch：                       RS485通道号，1~12对应光栅卡1~12路输入端口
    nAddr：                                 寄存器偏移地址
             返回值：                                     寄存器值
****************************************************************************/
int OgRs485ReadReg(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned int nAddr)
{
    int iVal = 0, iRegAddr = 0;

    iRegAddr = nAddr + (nRs485Ch-1) * 16;
    iVal = SfpRegisterRead(nSfpCh, iRegAddr);

    return iVal;
}

/****************************************************************************
OgRs485WriteReg:    写RS485寄存器值
    nSfpCh:         SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nRs485Ch：                        RS485通道号，1~12对应光栅卡1~12路输入端口
    nAddr：                                  寄存器偏移地址
    nVal：                                      写入寄存器值
             返回值：                                       1
****************************************************************************/
int OgRs485WriteReg(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned int nAddr, unsigned int nVal)
{
    unsigned char iRegAddr = 0;

    iRegAddr = nAddr + (nRs485Ch-1) * 16;
    SfpRegisterWrite(nSfpCh, iRegAddr, nVal);

    return 1;
}

/****************************************************************************
OgVerdateRead:  读取FPGA版本时间
    nSfpCh:     SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块

       返回值为16进制表示的版本时间，例如返回值0x20211203代表2021年12月3日
****************************************************************************/
int OgVerdateRead(unsigned char nSfpCh)
{
    int OgVerdate = 0;

    OgVerdate = OgRs485ReadReg(nSfpCh, 1, 0x20);

    return OgVerdate;
}

/****************************************************************************
OgVersionRead:  读取FPGA版本号
    nSfpCh:     SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块

       返回值为16进制表示的版本号，例如返回值0x01000000代表1.0.0.0版本
****************************************************************************/
int OgVersionRead(unsigned char nSfpCh)
{
    int OgVersion = 0;

    OgVersion = OgRs485ReadReg(nSfpCh, 1, 0x21);

    return OgVersion;
}

/****************************************************************************
OgRs485Cfg:         设置RS485通信协议格式
    nSfpCh:         SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nRs485Ch：                       RS485通道号，1~12对应光栅卡1~12路输入端口
    nBaud：                                  波特率
    nParityEn：                  奇偶校验使能
    nParitySel：              奇偶选择
    nStopWidth：              停止位，1~3
             返回值：                                       1
****************************************************************************/
int OgRs485Cfg(unsigned char nSfpCh, unsigned char nRs485Ch, int nBaud, unsigned char nParityEn, unsigned char nParitySel, unsigned char nStopWidth)
{
    //Baud Rate Set
    int Rs485BaudSet = 0;
    Rs485BaudSet = (62500000 % nBaud >= (nBaud/2)) ? 62500000/nBaud  + 1 : 62500000/nBaud;
    OgRs485WriteReg(nSfpCh, nRs485Ch, 0x40, Rs485BaudSet);

    //Parity Enable
    OgRs485WriteReg(nSfpCh, nRs485Ch, 0x41, nParityEn);

    //Parity Select
    OgRs485WriteReg(nSfpCh, nRs485Ch, 0x42, nParitySel);

    //Stop Width Set
    OgRs485WriteReg(nSfpCh, nRs485Ch, 0x43, nStopWidth);

    return 1;
}

/****************************************************************************
OgRs485SendData:    发送RS485数据
    nSfpCh:         SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nRs485Ch：                       RS485通道号，1~12对应光栅卡1~12路输入端口
    dataBuf：                           发送数据
    dataLen：                           发送数据长度
             返回值：                                       1
****************************************************************************/
int OgRs485SendData(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned char* dataBuf, unsigned int dataLen)
{
    int i = 0;

    for(i = 0; i < dataLen; i++)
    {
        OgRs485WriteReg(nSfpCh, nRs485Ch, 0x46, (unsigned int)dataBuf[i]);
    }

    return 1;
}

/****************************************************************************
OgRs485SendData:    接收RS485数据
    nSfpCh:         SFP通道号，1~6对应计算卡6路光模块，1~2对应时钟卡2路光模块
    nRs485Ch：                       RS485通道号，1~12对应光栅卡1~12路输入端口
    dataBuf：                           接收数据
             返回值：                                       接收数据长度
****************************************************************************/
int OgRs485RecvData(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned char* dataBuf)
{
    int i = 0;
    unsigned int iSfpRegVal = 0, idataLen = 0;

    iSfpRegVal = OgRs485ReadReg(nSfpCh, nRs485Ch, 0x47);
    idataLen = iSfpRegVal>>16;//获取当前接收FIFO数据个数

    for(i = 0; i < idataLen; i++)
    {
        dataBuf[i] = OgRs485ReadReg(nSfpCh, nRs485Ch, 0x49);
    }

    return idataLen;
}
