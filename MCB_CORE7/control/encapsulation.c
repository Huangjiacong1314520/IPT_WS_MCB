/*
 * encapsulation.c
 *
 *  Created on: 2022年5月26日
 *      Author: Administrator
 */
#include "encapsulation.h"
#include "../kern/macros.h"
#include "../srio/srio_cfg.h"
#include "function.h"
static double DAChannelCalibration(double value,DAChannel nChannel);
void IsrPrologue()
{

    SEMDOWN(DNUM);
    CSL_intcEventDisable(91);
    delay_us(1);
    // 判断是否所有轴计算完成
   /*flag = AxisCalculateFinished();*/
    WAITIPC(1);
    WAITIPC(2);
    WAITIPC(3);
    WAITIPC(4);
    WAITIPC(5);
    WAITIPC(6);
}
void IsrEpilogue()
{
    SEMUP(DNUM);
    CSL_intcEventClear(91);
    CSL_intcEventEnable(91);
    CSL_intcInterruptClear((CSL_IntcVectId)5);
}
void RefreshShareData(double *Motoroutput,int n)
{
    memcpy(MultiCoreShareData->MotorOutput, Motoroutput, sizeof(double)*n);
}
void GBcompensate(double Motoroutput[MotorCount])
{
    double macro_logicalx = GetAxisControlOutput(MacroLogicalX);
    double macro_logicaly = GetAxisControlOutput(MacroLogicalY);
    double macro_logicalrz = GetAxisControlOutput(MacroLogicalRz);

    double micro_logicalx = GetAxisControlOutput(MicroLogicalX);
    double micro_logicaly = GetAxisControlOutput(MicroLogicalY);
    double micro_logicalrz = GetAxisControlOutput(MicroLogicalRz);
    double micro_logicalz = GetAxisControlOutput(MicroLogicalZ);
    double micro_logicalrx = GetAxisControlOutput(MicroLogicalRx);
    double micro_logicalry = GetAxisControlOutput(MicroLogicalRy);

    Motoroutput[LongStrokeX] = - macro_logicalx * 1.0;
    Motoroutput[LongStrokeY1] = (macro_logicaly * 0.468250377073907) + (macro_logicalrz * 1.508295625942685);
    Motoroutput[LongStrokeY2] = (macro_logicaly * 0.531749622926094) - (macro_logicalrz * 1.508295625942685);

    Motoroutput[ShortStrokeX] = (micro_logicalx * 1.0);
    Motoroutput[ShortStrokeY1] = (micro_logicalx * 0.0798746321) + (micro_logicaly * 0.594574146) + (micro_logicalrz * 5.404236921);
    Motoroutput[ShortStrokeY2] = -(micro_logicalx * 0.0798746321) + (micro_logicaly * 0.405425853) - (micro_logicalrz * 5.404236921);

    Motoroutput[ShortStrokeZ1] = (micro_logicalz * 0.355343283582090) + (micro_logicalrx * 0.0)               - (micro_logicalry * 5.970149253731343);
    Motoroutput[ShortStrokeZ2] = (micro_logicalz * 0.323411388064770) + (micro_logicalrx * 4.851778176701761) + (micro_logicalry * 2.985219456064975);
    Motoroutput[ShortStrokeZ3] = (micro_logicalz * 0.321245328353140) - (micro_logicalrx * 4.851778176701761) + (micro_logicalry * 2.984929797666367);

}
static unsigned int MotorOutput[ChannelCount] = {0};
static unsigned char ChannelOutput[ChannelCount] = {0};
static uint16_t nDAChannelSelect = 0;
static uint16_t nDAChannelCount = 0;
inline void ClearDARecord()
{
    memset(MotorOutput, 0, sizeof(int)*ChannelCount);
    memset(ChannelOutput, 0, sizeof(unsigned char)*ChannelCount);
    nDAChannelCount = 0;
    nDAChannelSelect = 0;
}
static double DAChannelCalibration(double value,DAChannel nChannel)
{
    switch(nChannel) {
    case Channel1:
        return value;
    case Channel2:
        return value;
    case Channel3:
        return value;
    case Channel4:
        return value;
    case Channel5:
        return value;
    case Channel6:
        return value;
    case Channel7:
        return value;
    case Channel9:
        return value;
    case Channel11:
        return value;
    default:
        return value;
    }
}
inline unsigned int Double2DA(const double value)
{
    int temp = 0;
    if(value < 0){
        temp = -value;
    }else{
        temp = value;
        temp = ~temp;
        temp = (0x00080000 | (0x0007FFFF & temp)) + 1;
    }
    return temp;
}
void DAChannelAssign(double  value,DAChannel nChannel)
{
    double CailbrationValue = DAChannelCalibration(value, nChannel);
    unsigned int ConvertValue = Double2DA(CailbrationValue);

    MotorOutput[nChannel] = ConvertValue;
    ChannelOutput[nChannel] = 1;
    nDAChannelSelect |= 1<<((int)nChannel);
    ++nDAChannelCount;
}

int DAOutput(unsigned char nSFPChannel)
{
    Uint32 uRegAddress = 1;
    Uint8 cRegLen = 2;
    Uint8  cTransactionType = Transaction_Type_Data;
    Uint32  uWriteRequestAddr = 0x100000 + 0x10000*nSFPChannel;
    MyMemSet(AuroraBufferDA, 0, sizeof(AuroraBufferDA));
    SetAuroraPacketHead(uRegAddress,Flag_Write,cRegLen,nDAChannelSelect,cTransactionType,AuroraBufferDA);//写入包头
    char *pbody = (char *)(AuroraBufferDA + sizeof(AuroraPacketHead));
    int i;
    for(i = 0;i < ChannelCount;++i) {
        if (ChannelOutput[i] == 1) {
            AuroraPacketBody *temp = (AuroraPacketBody *)pbody;
            temp->data = MotorOutput[i];
            temp->channel_NO = i+1;
            pbody += sizeof(AuroraPacketBody);
        }
    }//写入包体
    AuroraPacketHead* pHeader = (AuroraPacketHead*)AuroraBufferDA;
    pHeader->packet_len = nDAChannelCount;
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    unsigned char cPackType = SRIO_PACKET_TYPE_STREAMWRITE;
    SRIORWStatus status = SRIOWriteDataLSU(cFPGAID, uWriteRequestAddr, cPackType, AuroraBufferDA, (nDAChannelCount+1)*4*2);//获取状态数据
    ClearDARecord();
    return -status;
}



void ProtectOut(double value[MotorCount],double limit)
{
    int i = 0;

    double outlimit[9];
    outlimit[0] = 1.0 ;       //LongStrokeX
    outlimit[1] = 1.0 ;       //LongStrokeY1
    outlimit[2] = 1.0 ;       //LongStrokeY2
    outlimit[3] = 10.0 ;       //ShortStrokeX
    outlimit[4] = 10.0 ;       //ShortStrokeY1
    outlimit[5] = 10.0 ;       //ShortStrokeY2
    outlimit[6] = 10.0 ;       //ShortStrokeZ1
    outlimit[7] = 10.0 ;       //ShortStrokeZ2
    outlimit[8] = 10.0 ;       //ShortStrokeZ3


    for(i = 0; i < MotorCount; i++){
        if (value[i] > outlimit[i]*52428.7)
            value[i] = outlimit[i]*52428.7 ;
        if (value[i] < -outlimit[i]*52428.7)
            value[i] = -outlimit[i]*52428.7 ;
    }

//    for(i = 0; i < MotorCount; i++){
//        if(value[i] > limit){
//            value[i] = limit;
//        }
//        if(value[i] < -limit){
//            value[i] = -limit;
//        }
//    }
}

void ConvertDouble2DA(const double fOut[MotorCount], Uint32 uOut[MotorCount]){
    int i = 0;
    int temp = 0;
    for(i = 0; i < MotorCount; i++){
        if(fOut[i] < 0){
            temp = -fOut[i];
        }else{
            temp = fOut[i];
            temp = ~temp;
            temp = (0x00080000 | (0x0007FFFF & temp)) + 1;
        }
        uOut[i] = temp;
    }

}
