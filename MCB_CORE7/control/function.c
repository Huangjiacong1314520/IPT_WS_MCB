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


SRIORWStatus SRIOReadDataLSU(unsigned char cRemoteDevID_8Bit, unsigned int uRdAddr, unsigned char cPackType, unsigned char *pLocalBuffer, int nLen)
{
    unsigned char cLSU = CORE_NUM;
    unsigned char cType=0;

    unsigned char cPort = 0;
    if (FPGA_DEVICE_ID_8BIT != cRemoteDevID_8Bit){
        cPort = 2;
    }
    cType = SRIOPacketType(cPackType);
    return SRIO_DirectIO((unsigned int)pLocalBuffer, uRdAddr,  cRemoteDevID_8Bit, nLen, cPort, cLSU,
                  cType , 0xa5a5);
}


SRIORWStatus  SRIOWriteDataLSU(unsigned char cRemoteDevID_8Bit, unsigned int uWrAddr, unsigned char cPktType, unsigned char *pLocalBuffer, int nLen)
{
    SRIOLSUConfig cfg;
    unsigned char cLSU =CORE_NUM;
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



#if CORE_NUM == 0
/*
 * 读AD卡
 */
int ReadAD(){
    int  i = 0;
    Uint32 uRegAddress = 0x4;
    Uint8  uChannel = 0x1; //通道序号
    AuroraPacket pkt;
    double* data = (double*)SensorData;
    int rawData = 0;
    double tmp = 0;

    MyMemSet(AuroraBufferAD,  0,  sizeof(AuroraBufferAD));

    Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
    uReadAddr += uRegAddress*0x100;
    SRIORWStatus nNReadRet = SRIOReadDataLSU(0xff,  uReadAddr, SRIO_PACKET_TYPE_NRead , AuroraBufferAD,  256);
    if (0 != nNReadRet)
        return -1;

    int nCount = 0;
    volatile Uint32* pData = (Uint32*)(AuroraBufferAD + 4);
    while(0 == (*pData)){
        nCount++;
        if(nCount >= 0xFFF)
            return -1;
    }

    pkt.head = (AuroraPacketHead*)AuroraBufferAD ;
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
int ReadRuler(){
    int  i = 0;
    Uint32 uRegAddress = 0x31;
    Uint8  uChannel = 0x3; //通道序号
    AuroraPacket pkt;
    double* data = (double*)SensorData;

    MyMemSet(AuroraBufferRuler,  0,  sizeof(AuroraBufferRuler));

    Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
    uReadAddr += uRegAddress*0x100;
    SRIORWStatus nNReadRet = SRIOReadDataLSU(0xff, uReadAddr, SRIO_PACKET_TYPE_NRead, AuroraBufferRuler, 256);
    if (0 != nNReadRet)
        return -1;

    int nCount = 0;
    volatile Uint32* pData = (Uint32*)(AuroraBufferRuler + 4);
    while(0 == (*pData)){
        nCount++;
        if(nCount >= 0xFFF)
            return -1;
    }

    pkt.head = (AuroraPacketHead*)AuroraBufferRuler;
    for (i = 0; i < pkt.head->packet_len; i++){
        pkt.body[i] = (AuroraPacketBody*)(AuroraBufferRuler + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
    }


//    for (i = 0; i < pkt.head->packet_len; i++){
//        *(data+i) = pkt.body[i]->data;
//    }

    int index = 0;
    for (i = 0; i < 8; i++){
        if(i < 3){
            index = 2*i + 1;
        }else{
            index = 2*i;
        }
        *(data+i) = pkt.body[index]->data;
    }
    return 0;
}

#elif CORE_NUM == 7
/****************************************************************************************************************************
 * DA卡输出
 */
int DAOut(Uint16 uChannelSel, const Uint32* pData){
    Uint32 uRegAddress = 1;
    Uint8 cRegLen = 2;
    Uint8  cTransactionType = Transaction_Type_Data;
    Uint32  uWriteRequestAddr = 0x120000;
    int i = 0;

    MyMemSet(AuroraBufferIndexDA, 0, sizeof(AuroraBufferIndexDA));
    MyMemSet(AuroraBufferDA, 0, sizeof(AuroraBufferDA));

    Uint8 uFrameCount = CountOne(uChannelSel);
    int nChannel[PacketBodyMax];
    int nValidIndex = 0;

    for (i = 0; i < PacketBodyMax;  i++){
        nChannel[i] = 0;

        if (uChannelSel &  (1  << i)){
            nChannel[nValidIndex] = i+1;
            nValidIndex++;
        }
    }

    SetAuroraPacketHead(uRegAddress, Flag_Write,  cRegLen, uChannelSel, cTransactionType, AuroraBufferDA);
    AuroraBufferIndexDA[0] = 1;
    for(i = 0; i < nValidIndex; i++){
        int j = 0;

        for(j = 1; j <= PacketBodyMax; j++){
            if ( 0 == AuroraBufferIndexDA[j]){
                AuroraBufferIndexDA[j] = 1;
                break;
            }
        }

        AuroraPacketBody* pkt = (AuroraPacketBody*)(AuroraBufferDA + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*(j-1));

        pkt->data = pData[i];
        pkt->channel_NO = nChannel[i];
    }

    AuroraPacketHead* pHeader = (AuroraPacketHead*)AuroraBufferDA;
    pHeader->packet_len = uFrameCount; //4字节的倍数

    if (0){
        #ifdef _LITTLE_ENDIAN

        Uint32* uipData = 0;
        for( i = 0;  i < (uFrameCount+1)*2 ; i++){
            uipData = (Uint32 *)(AuroraBufferDA + i*sizeof(Uint32));
            *uipData = Reverse32(*uipData);
        }
        #endif
    }

    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    unsigned char cPackType = SRIO_PACKET_TYPE_STREAMWRITE;
    SRIORWStatus status = 0;
    //SRIORWStatus status = SRIOWriteDataLSU(cFPGAID, uWriteRequestAddr, cPackType, AuroraBufferDA, (uFrameCount+1)*4*2);
    if (COMPLETED_NOERRORS == status){
        return 0;
    }
    else{
        return -1;
    }

}
/****************************************************************************************************************************/

void InsertCommitData(int count, int channel, double value){
    *(CommitData + 100*channel + count) = value;
}

#endif

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
    //
    runFlag = (unsigned char*)(0x0C200000 + 7*0x00040000);
    *runFlag = value;
}

void SetMultiCoreFreshFlag(){
    unsigned char* refreshFlag;
    int i = 0;

    for(i = 1; i <= AXIS_NUM; i++){
        refreshFlag = (unsigned char*)(0x0C200000 + i*0x00040000 + 3);
        *refreshFlag = 1;
    }
}

void SetMultiCoreSPara(int axis, SParaHandle sParaSrc){
    SParaHandle sParaDst = (SParaHandle)(0x0C000000 + 0x00080000*axis + 5);

    memcpy(sParaDst, sParaSrc, sizeof(SParaStruct));
}


void SetMultiCoreControlPara(int axis, ControlParaHandle ctrlParaSrc){
    ControlParaHandle ctrlParaDst = (ControlParaHandle)(0x0C000000 + 0x00080000*axis + 5 + sizeof(SParaStruct)*3);

    memcpy(ctrlParaDst, ctrlParaSrc, sizeof(ControlParaStruct));
}


//int VerifyMultiCoreFinishFlag(){
//
//}















