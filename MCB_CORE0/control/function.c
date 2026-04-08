/*
 * function.c
 *
 *  Created on: 2021年8月22日
 *      Author: lkx
 */



#include "function.h"

//任意阶离散控制器
//n:阶数
//Err_Now：控制器当前输入
//e_：输入向量 e_= [e(k) e(k-1) .. e(k-n)]
//U_：输出向量 U_= [U(k) U(K-1) ... U(k-n)];
//Param:控制器分子分母系数，先分母后分子 Parma = [a(n) a(n-1) .. a(0) b(n) b(n-1) ... b(0)]

double Controller(double Err_Now, double *U_, double *e_, double *Param, Uint16 n)
{
    double Fout = 0;       // 当前控制器输出
    double Fouttemp1 = 0;
    double Fouttemp2 = 0;

    unsigned int i = 0;

    e_[0] = Err_Now;

    for(i=0;i<n;i++)
        Fouttemp1 = Fouttemp1 + (-1.0*U_[i+1])*Param[i];
    for(i=0;i<n+1;i++)
        Fouttemp2 = Fouttemp2 + e_[i]*Param[i+n];

    Fout = Fouttemp1 + Fouttemp2;
    //更新
    U_[0] = Fout;
    for(i=n;i>0;i--)
        U_[i] = U_[i-1];

    for(i=n;i>0;i--)
        e_[i] = e_[i-1];

    return Fout;
}
unsigned char PowerCalculate(unsigned int number)
{
    unsigned char ret = 0;
    do {
        number = number >> 1;
        ++ret;
    }while(number);
    return ret-1;
}

//一维数组初始化，A的维数为n
void VectorSet(double* A, int n, double value)
{
    int i = 0;
    for(i=0;i<n;i++){
        *(A+i) = value;
    }
}

//二维数组初始化，A的维数为n
void Vector2Set(double* A, int row, int column, double value)
{
    int i = 0;
    int j = 0;

    for(i = 0; i < row; i++){
        for(j=0;j<column;j++){
            *(A+i*column+j) = value;
        }
    }

}


/*
计算数组前n个数据的最大值
*/
double MaxDouble(double* data, int num){
    double max = data[0];
    int i = 0;

    for(i = 0; i < num; i++){
        if(max < data[i]){
            max = data[i];
        }
    }

    return max;

}



// 控制器参数初始化
void ParamInit(){

}

/*
 * 用于8字节对齐的内存空间设置
 */
void * MyMemSet(void *dst, int fill, size_t len){
    int i;
    char *restrict dst1, *restrict dst2;
    dst1 = (char *)dst;
    dst2 = dst1 + 8;
    double dfill1 = _itod(fill, fill);
    for (i = 0; i < len >> 4; i++){
        _amemd8(dst1) = dfill1;        dst1 += 16;
        _amemd8(dst2) = dfill1;        dst2 += 16;
    }

    return dst;
}


SRIORWStatus SRIOReadDataLSU(unsigned char cRemoteDevID_8Bit, unsigned char nLSU,unsigned int uRdAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen)
{
    unsigned char cLSU = nLSU;
    unsigned char cType=0;

    unsigned char cPort = 0;
    if (FPGA_DEVICE_ID_8BIT != cRemoteDevID_8Bit){
        cPort = 2;
    }
    cType = SRIOPacketType(cPackType);
    return SRIO_DirectIO((unsigned int)pLocalBuffer, uRdAddr,  cRemoteDevID_8Bit, nLen, cPort, cLSU,
                  cType , 0xa5a5);
}


SRIORWStatus  SRIOWriteDataLSU(unsigned char cRemoteDevID_8Bit, unsigned char nLSU,unsigned int uWrAddr, unsigned char cPktType, unsigned char *pLocalBuffer, int nLen)
{
    SRIOLSUConfig cfg;
    unsigned char cLSU =nLSU;
    unsigned char cType=0;
    unsigned int  uRetryCount = 0xFFFF;
    unsigned char cPort = 0;
    if (FPGA_DEVICE_ID_8BIT != cRemoteDevID_8Bit)
    {
        cPort = 2;
    }

    unsigned char cStatus = SRIOLSUFullAndBusyCheck(cLSU);
    while(cStatus != 0)
    {
        cStatus = SRIOLSUFullAndBusyCheck(cLSU);
        uRetryCount--;
        if (!uRetryCount)
        {
            if (cStatus & 0x0F)
                return TIMEOUT_LSU_BUSY;
            else
                return TIMEOUT_LSU_FULL;
        }
    }


    cType = SRIOPacketType(cPktType);

    SRIO_InitLsuCfg(&cfg, (Uint32)pLocalBuffer, uWrAddr, cType, nLen,  cRemoteDevID_8Bit, cPort);

    if(cType == SRIO_DoorBell)
    {
        cfg.DoorBell.Enable = TRUE;//是否需要在传输完成后发送doorbell，0表示不需要
        cfg.DoorBell.Info = pLocalBuffer[0]<<8 | pLocalBuffer[1];
    }
    else
    {
        cfg.DoorBell.Enable = FALSE;
        cfg.DoorBell.Info = 0xa5a5;
    }


    uRetryCount=0xFFFFF;
    SRIORWStatus nStatus=COMPLETED_NOERRORS;
    SRIODirectIOTransfer(cLSU, &cfg);

    nStatus = SRIOLSUStatusGet(cLSU, &cfg);//得到完成码（调试用）
    while(nStatus != COMPLETED_NOERRORS){
        nStatus = SRIOLSUStatusGet(cLSU, &cfg);//得到完成码（调试用）
        uRetryCount--;
        if(!uRetryCount){
            return TIMEOUT;
        }
    }

    return COMPLETED_NOERRORS;
}


/*
 * 读AD卡
 */

int ReadAD(unsigned char nSfpChannel){
    int  i = 0;
    Uint32 uRegAddress = 0x4;
    Uint8  uChannel = nSfpChannel;
    AuroraPacket pkt;
    double* data = (double*)SensorData;
    int rawData = 0;
    double tmp = 0;

    MyMemSet(AuroraBufferAD,  0,  sizeof(AuroraBufferAD));

    Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
    uReadAddr += uRegAddress*0x100;
    SRIORWStatus nNReadRet = SRIOReadDataLSU(0xff,0x1,uReadAddr, SRIO_PACKET_TYPE_NRead , AuroraBufferAD,  sizeof(AuroraBufferAD));
    if (0 != nNReadRet)
        return -1;

    int nCount = 0;
    //
    volatile Uint32* pData = (Uint32*)(AuroraBufferAD + 4);

    while(0 == (*pData)){
        nCount++;
        if(nCount >= 0xFFF)
            return -1;
    }

    pkt.head = (AuroraPacketHead*)(AuroraBufferAD);
    for (i = 0; i < pkt.head->packet_len; i++){
        pkt.body[i] = (AuroraPacketBody*)(AuroraBufferAD + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
    }

    for (i = 0; i < pkt.head->packet_len; i++){
        rawData = pkt.body[i]->data;
        tmp = 2.0*rawData;
        tmp /= (0x1000000-1);
        tmp = tmp -1;
        tmp = tmp*12.5;
        *(data+8+i) = tmp;
    }

    return 0;
}


/*
 * 读光栅卡
 */

int ReadRuler(unsigned char nSfpChannel){
    int  i = 0;
    Uint32 uRegAddress = 0x31;
    Uint8  uChannel = nSfpChannel;
    AuroraPacket pkt;
    double* data = (double*)SensorData;
    MyMemSet(AuroraBufferRuler,  0,  sizeof(AuroraBufferRuler));

    Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
    uReadAddr += uRegAddress*0x100;
    //开始测试
    SRIORWStatus nNReadRet = SRIOReadDataLSU(0xff,0x2, uReadAddr, SRIO_PACKET_TYPE_NRead, AuroraBufferRuler, sizeof(AuroraBufferRuler));

    if (0 != nNReadRet)
        return -1;

    int nCount = 0;
    volatile Uint32* pData = (Uint32*)(AuroraBufferRuler + 4);

    while(0 == (*pData)){
       nCount++;

        CACHE_invL1d((void *)pData, 256, CACHE_NOWAIT);

        if(nCount >= 0x1FF)
            return -1;
    }
    pkt.head = (AuroraPacketHead*)AuroraBufferRuler;
    for (i = 0; i < pkt.head->packet_len; i++){
        pkt.body[i] = (AuroraPacketBody*)(AuroraBufferRuler + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
    }


    int index = 0;
//    int v = 0;
//    int b = 0;
    for (i = 0; i < 8; i++){
        index = 2*i;
//        b = *((int*)(&pkt.body[index]->reserved));
//        v = *((int*)(&pkt.body[index]->reserved) + 1);
        *(data+i) = *((int*)(&pkt.body[index]->reserved) + 1);
        RulerState[i] = *((unsigned int*)(&pkt.body[index+1]->reserved) + 1);
//        *(data+i) = *(data+i) * 25.0 / 1000000000.0;
    }
    //

    return 0;
}
int ReadRulerTopHalf(unsigned char nSfpChannel)
{
    Uint32 uRegAddress = 0x31;
    Uint8  uChannel = nSfpChannel;
    MyMemSet(AuroraBufferRuler,  0,  sizeof(AuroraBufferRuler));

    Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
    uReadAddr += uRegAddress*0x100;
    //开始测试
    SRIORWStatus nNReadRet = SRIOReadDataLSU(0xff, 0x0,uReadAddr, SRIO_PACKET_TYPE_NRead, AuroraBufferRuler, sizeof(AuroraBufferRuler));

    return -nNReadRet;

}
int ReadRulerBottomHalf(unsigned char nSfpChannel)
{
    int i;
    AuroraPacket pkt;
   // int nCount = 0;
    double* data = (double*)SensorData;
    volatile Uint32* pData = (Uint32*)(AuroraBufferRuler + 4);
    if (0 == (*pData)) {
        SetErrorCode(Ruleroffline);
        return -1;
    }
    pkt.head = (AuroraPacketHead*)AuroraBufferRuler;
    for (i = 0; i < pkt.head->packet_len; i++){
        pkt.body[i] = (AuroraPacketBody*)(AuroraBufferRuler + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
    }
    int index = 0;
//    int v = 0;
//    int b = 0;
    for (i = 0; i < 8; i++){
        index = 2*i;
//        b = *((int*)(&pkt.body[index]->reserved));
//        v = *((int*)(&pkt.body[index]->reserved) + 1);
        *(data+i) = *((int*)(&pkt.body[index]->reserved) + 1);
        RulerState[i] = *((unsigned int*)(&pkt.body[index+1]->reserved) + 1);
//        *(data+i) = *(data+i) * 25.0 / 1000000000.0;
    }
    return 0;
}
int ReadADTopHalf(unsigned char nSfpChannel)
{
    Uint32 uRegAddress = 0x4;
    Uint8  uChannel = nSfpChannel;


    MyMemSet(AuroraBufferAD,  0,  sizeof(AuroraBufferAD));

    Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
    uReadAddr += uRegAddress*0x100;
    SRIORWStatus nNReadRet = SRIOReadDataLSU(0xff,0x0,uReadAddr, SRIO_PACKET_TYPE_NRead , AuroraBufferAD,  sizeof(AuroraBufferAD));
    return -nNReadRet;
}
int ReadADBottomHalf(unsigned char nSfpChannel)
{
    int i;
    AuroraPacket pkt;
  //  int nCount = 0;
    double* data = (double*)SensorData;
    int rawData = 0;
    double tmp = 0;
    //
    volatile Uint32* pData = (Uint32*)(AuroraBufferAD + 4);

//    while(0 == (*pData)){
//        nCount++;
//        if(nCount >= 0xFFF)
//            return -1;
//    }
    if (0 == (*pData)) {
        SetErrorCode(ADoffline);
        return -1;
    }

    pkt.head = (AuroraPacketHead*)(AuroraBufferAD);
    for (i = 0; i < pkt.head->packet_len; i++){
        pkt.body[i] = (AuroraPacketBody*)(AuroraBufferAD + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
    }

    for (i = 0; i < pkt.head->packet_len; i++){
        rawData = pkt.body[i]->data;
        tmp = 2.0*rawData;
        tmp /= (0x1000000-1);
        tmp = tmp -1;
        tmp = tmp*12.5;
        *(data+8+i) = tmp;
    }
    return 0;
}
int ReadInfTopHalf()
{
    MyMemSet(Interferometerbuf, 0, 256);
    P2busRead(Interferometerbuf, 8);
    return 0;
}
int ReadInfBottomHalf()
{
    int i = 0;
    unsigned long long mData = 4294967296;
    double tData = 0;
    unsigned int uData = 0;
    unsigned int lData = 0;
    unsigned int sData = 0;
    unsigned long long rData = 0;
    double* data = (double*)SensorData;
    for (i = 0;i < 3;i++) {
        uData = *(unsigned int*)(Interferometerbuf + 8*i);
        lData = *(unsigned int*)(Interferometerbuf + 8*i + 4);
        sData = uData&0x00000008;
        rData = (unsigned long long)(uData&0x00000007)*mData + lData;

        if(sData == 0){
            tData = rData;
        }else{
            tData = -1.0*((~(rData-1))&0x00000007FFFFFFFF);
        }

        *(data+16+i) = tData*LASER_RES;
    }

    for (i = 4;i < 7;i++) {
        uData = *(unsigned int*)(Interferometerbuf + 8*i);
        lData = *(unsigned int*)(Interferometerbuf + 8*i + 4);
        sData = uData&0x00000008;
        rData = (unsigned long long)(uData&0x00000007)*mData + lData;

        if(sData == 0){
            tData = rData;
        }else{
            tData = -1.0*((~(rData-1))&0x00000007FFFFFFFF);
        }
        *(data+16+i-1) = tData*LASER_RES;
    }
    return 0;
}
void P2busCfg(unsigned char nP2busMode, unsigned char nStartAddr, unsigned char nReadNum)
{
    unsigned char SendBuf[8] = {0};
    SendBuf[4] = nP2busMode;
    SendBuf[5] = nStartAddr;
    SendBuf[6] = nReadNum;
    //int SRIO_WriteData(unsigned char uLSU, unsigned int uWrAddr, unsigned char *pData,int nLen)
    SRIO_WriteData(0, 0x28, SendBuf, 8);//0X28

    return;
}

void P2busRead(unsigned char *nP2busRecvData, unsigned char nReadNum)
{
    //int SRIO_WriteData(unsigned char uLSU, unsigned int uWrAddr, unsigned char *pData,int nLen)
    int a = 0;

    a = SRIOReadDataLSU(0xFF,0x3, 0x100000, SRIO_PACKET_TYPE_NRead , nP2busRecvData,  nReadNum*8);
//    a = SRIO_ReadData(0, 0x100000, nP2busRecvData, nReadNum*8); //0x100000

    if(a != 0){
        printf("error, a=%d\n", a);
    }
    return;
}

void ReadInterferometer()
{
    /*unsigned char RecvData[256] = {0};
    double* data = (double*)SensorData;

    P2busRead(RecvData, 8);//RecvData
    for (i = 0; i < 6; i++){
        *(data+16+i) = *(double*)(RecvData+8*i)*LASER_RES;
    }*/

    int i = 0;
    unsigned long long mData = 4294967296;
    double tData = 0;
    unsigned int uData = 0;
    unsigned int lData = 0;
    unsigned int sData = 0;
    unsigned long long rData = 0;
    double* data = (double*)SensorData;

    unsigned char RecvData[256] = {0};
    P2busRead(RecvData, 8);//RecvData

    delay_us(2);

//    for (i = 0; i < 6; i++){
//        tData = 0;
//        rData = 0;
//        uData = *(unsigned int*)(RecvData + 8*i);
//        lData = *(unsigned int*)(RecvData + 8*i + 4);
//        sData = uData&0x00000008;
//        rData = (unsigned long long)(uData&0x00000007)*mData + lData;
//
//        if(sData == 0){
//            tData = rData;
//        }else{
//            tData = -1.0*((~(rData-1))&0x00000007FFFFFFFF);
//        }
//        *(data+16+i) = tData*LASER_RES;
//        *(data+1+i) = tData*6.179184399358623e-10;
//    }

    for (i = 0;i < 3;i++) {
        uData = *(unsigned int*)(RecvData + 8*i);
        lData = *(unsigned int*)(RecvData + 8*i + 4);
        sData = uData&0x00000008;
        rData = (unsigned long long)(uData&0x00000007)*mData + lData;

        if(sData == 0){
            tData = rData;
        }else{
            tData = -1.0*((~(rData-1))&0x00000007FFFFFFFF);
        }

        *(data+16+i) = tData*LASER_RES;
    }

    for (i = 4;i < 7;i++) {
        uData = *(unsigned int*)(RecvData + 8*i);
        lData = *(unsigned int*)(RecvData + 8*i + 4);
        sData = uData&0x00000008;
        rData = (unsigned long long)(uData&0x00000007)*mData + lData;

        if(sData == 0){
            tData = rData;
        }else{
            tData = -1.0*((~(rData-1))&0x00000007FFFFFFFF);
        }
        *(data+16+i-1) = tData*LASER_RES;
    }
}

void SetMultiCoreInterruptFlag(){
    unsigned char* interruptFlag;
    int i = 0;

    for(i = 1; i <= AXIS_NUM; i++){
        interruptFlag = (unsigned char*)(0x0C000000 + i*0x00080000);
        *interruptFlag = 1;
    }
}

void SetMultiCoreRunFlag(unsigned char value){
    unsigned char* runFlag;
    int i = 0;

    for(i = 1; i <= AXIS_NUM; i++){
        runFlag = (unsigned char*)(0x0C200000 + i*0x00040000);
        *runFlag = value;
    }
    //core 7
    runFlag = (unsigned char*)(0x0C200000 + 7*0x00040000);
    *runFlag = value;
    // motor_EP
    //runFlag = (unsigned char *)(0x0C200000 + 1*0x00040000 + sizeof(ConfigStruct_motion_core));
    *runFlag = value;
}

void SetMultiCoreFreshFlag(unsigned char value){
    unsigned char* refreshFlag;
    int i = 0;

    for(i = 1; i <= AXIS_NUM; i++){
        refreshFlag = (unsigned char*)(0x0C200000 + i*0x00040000 + 2);
        *refreshFlag = value;
    }
    //refreshFlag = (unsigned char *)(0x0C200000 + 1*0x00040000 + 2 + sizeof(ConfigStruct_motion_core));
    *refreshFlag = value;
}

void SetMultiCoreSPara(int axis, SParaHandle sParaSrc){
    SParaHandle sParaDst = (SParaHandle)(0x0C200000 + 0x00040000*(axis+1) + 8);

    memcpy(sParaDst, sParaSrc, sizeof(SParaStruct));
}


void SetMultiCoreControlPara(int axis, ControlParaHandle ctrlParaSrc){
    ControlParaHandle ctrlParaDst = (ControlParaHandle)(0x0C200000 + 0x00040000*(axis+1) + 8 + sizeof(SParaStruct)*3);

    memcpy(ctrlParaDst, ctrlParaSrc, sizeof(ControlParaStruct));
}

void InsertCommitData(int count, int channel, double value){
   // CommitBuf.Channel[channel][count] = value;
    pCurrentBuf->Channel[channel][count] = value;
}












