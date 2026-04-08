#ifndef _RS485_H_
#define _RS485_H_

int SfpRegisterRead(unsigned char nSfpCh, unsigned int nAddr);
int SfpRegisterWrite(unsigned char nSfpCh, unsigned int nAddr, unsigned int nVal);

int OgRs485ReadReg(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned int nAddr);
int OgRs485WriteReg(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned int nAddr, unsigned int nVal);

int OgVerdateRead(unsigned char nSfpCh);
int OgVersionRead(unsigned char nSfpCh);

int OgRs485Cfg(unsigned char nSfpCh, unsigned char nRs485Ch, int nBaud, unsigned char nParityEn, unsigned char nParitySel, unsigned char nStopWidth);
int OgRs485SendData(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned char* dataBuf, unsigned int dataLen);
int OgRs485RecvData(unsigned char nSfpCh, unsigned char nRs485Ch, unsigned char* dataBuf);

#endif
