#include  "srio_cfg.h"
#include "../drv_srio.h"
#include "string.h"
#include "stdio.h"
#include "cslr_cgem.h"
#include "csl_cacheAux.h"
//#include "csl_xmcAux.h"
#define CACHE_L1D 1

inline Uint32 Reverse32(Uint32  x)
{
    return ((x&0x000000ff)<<24)|((x&0x0000ff00)<<8)|((x&0x00ff0000)>>8)|((x&0xff000000)>>24);
}


int  ReadSYSVersionReg(unsigned char* pVersionTime,  unsigned char* pVersionNum)
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    Uint32   uRdAddr = SRIO_SYS_REG_BASE + SRIO_SYS_VERSION_REG;
    unsigned char cPackType = SRIO_PACKET_TYPE_NRead;
    unsigned char pLocalBuff[8] = {0};
    int nLen = 8;

    SRIORWStatus  status = SRIOReadData( cFPGAID, uRdAddr, cPackType ,   pLocalBuff,   nLen);
    if (COMPLETED_NOERRORS == status)
    {
        pVersionTime[0] =  pLocalBuff[0] ;
        pVersionTime[1] =  pLocalBuff[1] ;
        pVersionTime[2] =  pLocalBuff[2] ;
        pVersionTime[3] =  pLocalBuff[3] ;

        pVersionNum[0] =  pLocalBuff[4] ;
        pVersionNum[1] =  pLocalBuff[5] ;
        pVersionNum[2] =  pLocalBuff[6] ;
        pVersionNum[3] =  pLocalBuff[7] ;
        return 0;
    }
    else
    {
        pVersionTime[0] =  0x20  ;
        pVersionTime[1] =  0x21  ;
        pVersionTime[2] =  0x03  ;
        pVersionTime[3] =  0x29  ;

        pVersionNum[0] =  0x01 ;
        pVersionNum[1] =  0x00 ;
        pVersionNum[2] =  0x00 ;
        pVersionNum[3] =  0x00 ;
        return (int)status;
    }
}
int  WriteSYSVersionReg(unsigned char*  uVersionTime, unsigned char* uVersionNum)
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    Uint32   uWrAddr = SRIO_SYS_REG_BASE + SRIO_SYS_VERSION_REG;
    unsigned char cPackType = SRIO_PACKET_TYPE_NWRITE;
    unsigned char pLocalBuffer[8] ={0};
    int nLen = 8;

    pLocalBuffer[0] = uVersionTime[0];
    pLocalBuffer[1] = uVersionTime[1];
    pLocalBuffer[2] = uVersionTime[2];
    pLocalBuffer[3] = uVersionTime[3];

    pLocalBuffer[4] = uVersionNum[0] ;
    pLocalBuffer[5] = uVersionNum[1] ;
    pLocalBuffer[6] = uVersionNum[2] ;
    pLocalBuffer[7] = uVersionNum[3] ;
    SRIORWStatus  status = SRIOWriteData(cFPGAID,   uWrAddr,  cPackType,  pLocalBuffer,   nLen);
    if (COMPLETED_NOERRORS == status)
        return 0;
    else
        return (int)status;
}

int  ReadSlotNumReg(Uint32*  uNO)
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
        Uint32   uRdAddr = SRIO_SYS_REG_BASE + SRIO_SLOT_NUM_REG;
        unsigned char cPackType = SRIO_PACKET_TYPE_NRead;
        unsigned char pLocalBuff[8] = {0};
        int nLen = 8;

        SRIORWStatus  status = SRIOReadData( cFPGAID, uRdAddr, cPackType ,   pLocalBuff,   nLen);
        if (COMPLETED_NOERRORS == status)
        {
            *uNO =  (pLocalBuff[7] & 0x1F);
            //assert((*uNO)%2  ==  (pLocalBuff[1] & 0x01));
            return 0;
        }
        else
        {
            *uNO = 0;
            return (int)status;
        }
}

//uLED1 --绿, uLED2 --红
int  WriteFrontLEDReg(unsigned short  uLED1, unsigned short  uLED2)
{
        unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
        Uint32   uWrAddr = SRIO_SYS_REG_BASE + SRIO_FRONT_LED_REG;
        unsigned char cPackType = SRIO_PACKET_TYPE_NWRITE;
        unsigned char pLocalBuffer[8] ={0};
        int nLen = 8;

        pLocalBuffer[7] = ((uLED1&0x01) | (uLED1&0x02) | ((uLED2&0x01)<< 2 ) |  (( uLED2&0x2) << 2 ));
        SRIORWStatus  status = SRIOWriteData(cFPGAID,   uWrAddr,  cPackType,  pLocalBuffer,   nLen);
        if (COMPLETED_NOERRORS == status)
            return 0;
        else
            return (int)status;
}

int  ReadSFPSTAReg(unsigned char* pSFPLinkStatus )
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
           Uint32   uRdAddr = SRIO_SYS_REG_BASE + SRIO_SFP_STA_REG;
           unsigned char cPackType = SRIO_PACKET_TYPE_NRead;
           unsigned char pLocalBuff[8] = {0};
           int nLen = 8;

           SRIORWStatus  status = SRIOReadData( cFPGAID, uRdAddr, cPackType ,   pLocalBuff,   nLen);
           if (COMPLETED_NOERRORS == status)
           {
               *pSFPLinkStatus =  (pLocalBuff[7] & 0x3F);
               return 0;
           }
           else
           {
               return (int)status;
           }
}

int   WriteP2BUSModeReg(BOOL  bMasterMode) //0 从模式,  1 主模式
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    Uint32   uWrAddr = SRIO_SYS_REG_BASE + SRIO_P2BUS_MODE_REG;
    unsigned char cPackType = SRIO_PACKET_TYPE_NWRITE;
    unsigned char pLocalBuffer[8] ={0};
    int nLen = 8;

    pLocalBuffer[7] =  bMasterMode;
    SRIORWStatus  status = SRIOWriteData(cFPGAID,   uWrAddr,  cPackType,  pLocalBuffer,   nLen);
    if (COMPLETED_NOERRORS == status)
        return 0;
    else
        return (int)status;
}

int   WriteGPIOSelReg(BOOL  bIn)//0 输出, 1输入
{
        unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
        Uint32   uWrAddr = SRIO_SYS_REG_BASE + SRIO_GPIO_SEL_REG;
        unsigned char cPackType = SRIO_PACKET_TYPE_NWRITE;
        unsigned char pLocalBuffer[8] ={0};
        int nLen = 8;

        pLocalBuffer[7] =  bIn;
        SRIORWStatus  status = SRIOWriteData(cFPGAID,   uWrAddr,  cPackType,  pLocalBuffer,   nLen);
        if (COMPLETED_NOERRORS == status)
            return 0;
        else
            return (int)status;
}

int    ReadGPIReg(unsigned short*  pGPI)
{
      unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
       Uint32   uRdAddr = SRIO_SYS_REG_BASE + SRIO_GPI_REG;
       unsigned char cPackType = SRIO_PACKET_TYPE_NRead;
       unsigned char pLocalBuff[8] = {0};
       int nLen = 8;
       SRIORWStatus  status = SRIOReadData( cFPGAID, uRdAddr, cPackType ,   pLocalBuff,   nLen);
       if (COMPLETED_NOERRORS == status)
       {
           *pGPI =  (pLocalBuff[7] & 0xFF);
           return 0;
       }
       else
       {
           return (int)status;
       }
}


int    WriteGPOReg(unsigned short  uGPO)
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    Uint32   uWrAddr = SRIO_SYS_REG_BASE + SRIO_GPO_REG;
    unsigned char cPackType = SRIO_PACKET_TYPE_NWRITE;
    unsigned char pLocalBuffer[8] ={0};
    int nLen = 8;

    pLocalBuffer[7] =  uGPO& 0x00FF;
    pLocalBuffer[6] =  (uGPO& 0xFF00) >> 8;
    SRIORWStatus  status = SRIOWriteData(cFPGAID,   uWrAddr,  cPackType,  pLocalBuffer,   nLen);
    if (COMPLETED_NOERRORS == status)
        return 0;
    else
        return (int)status;
}

/*********************************************************************************************
*********                         Aurora 协议 基本操作函数                          ***************
*********************************************************************************************/
//Aurora 协议封包   拆包
#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma  DATA_ALIGN( CACHE_L2_LINESIZE)
#else
#pragma  DATA_SECTION(g_localAuroraPktBuffer,  "AuroraData")
#pragma  DATA_ALIGN( g_localAuroraPktBuffer,   CACHE_L2_LINESIZE)
#endif
Uint8  g_localAuroraPktBuffer[256] = {0};

#ifdef  __cplusplus
#pragma DATA_SECTION("AuroraData")
#pragma  DATA_ALIGN( 16)
#else
#pragma  DATA_SECTION(g_localAuroraPktBufferIndex,  "AuroraData")
#pragma  DATA_ALIGN( g_localAuroraPktBufferIndex,   16)
#endif
Uint8  g_localAuroraPktBufferIndex[32] = {0};

void   SetAuroraPacketHead(Uint32  uRegAddress,  BOOL bRead, Uint32 uLen, Uint32  uChannel, Uint32 uTransactionType, Uint8* pBuffer)
{
    AuroraPacketHead *   head =  (AuroraPacketHead * )pBuffer;

    head->reserved = head->packet_len = head->reserved2 = 0;

    head->reg_addr     = 0x7F & uRegAddress ;
    head->rw_flag           = 0x01 & bRead;
    head->data_reg_len                 = 0x03 & uLen;

    head->channel_sel          = 0xFFF & uChannel;
    head->transaction_type = 0x03 & uTransactionType;



//    memset(g_localAuroraPktBufferIndex, 0,  sizeof(g_localAuroraPktBufferIndex) );
//    memcpy(g_localAuroraPktBuffer,  &head,  sizeof head);



}

// 通道编号从1开始
void  SetAuroraPacketData(Uint32  uData,  Uint8  uChannelNO, Uint8* pBuffer)
{
    int  i = 1 ;
    for(i = 1; i <= PacketBodyMax; i++)
    {
        if ( 0 == g_localAuroraPktBufferIndex[i])
        {
            g_localAuroraPktBufferIndex[i] = 1;
            break;
        }
    }

    AuroraPacketBody*  pkt  = (AuroraPacketBody*  ) (pBuffer +  sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody) *(i-1));
    //pkt->reserved   = pkt->data = pkt->channel_NO = pkt->reserved1 = 0;


    pkt->data = uData;
    pkt->channel_NO = uChannelNO;

}
//发送数据
int  WriteAuroraPacket(Uint32  uWrAddr)
{
        unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
        unsigned char cPackType = SRIO_PACKET_TYPE_STREAMWRITE;

        int i = 1;
        for (i = 1;  i <= PacketBodyMax; i++)
        {
             if (0 == g_localAuroraPktBufferIndex[i] )
                 break;
        }
        int nLen = sizeof(AuroraPacketHead)  + sizeof(AuroraPacketBody) * (i - 1);

        AuroraPacketHead*  pHeader = (AuroraPacketHead*)g_localAuroraPktBuffer;
        pHeader->packet_len = i-1;
        SRIORWStatus  status = SRIOWriteData(cFPGAID, uWrAddr,  cPackType,   g_localAuroraPktBuffer,   nLen);
        if (COMPLETED_NOERRORS == status)
            return 0;
        else
            return (int)status;
}

int     WriteAuroraPacketWithBuffer(Uint32   uWrAddr, Uint32  uLen,  Uint8* pBuffer)
{
           unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
           unsigned char cPackType = SRIO_PACKET_TYPE_STREAMWRITE;


           SRIORWStatus  status = SRIOWriteData( cFPGAID, uWrAddr, cPackType ,   pBuffer,   uLen);
           if (COMPLETED_NOERRORS == status)
           {
               return 0;
           }
           else
           {
               return (int)status;
           }
}

//发送  读数据 请求
int   SendReadAuroraPacketRequest(Uint32   uWrAddr,  Uint32  uLen)
{
    return WriteAuroraPacketWithBuffer(uWrAddr,   uLen,  g_localAuroraPktBuffer);
}

// 读数据     .返回读取字节数
int   ReadAuroraPacket(Uint8  uFrameLen,  Uint32  uRdAddr)
{
       memset(g_localAuroraPktBuffer, 0, 256);//sizeof g_localAuroraPktBuffer);
       memset(g_localAuroraPktBufferIndex, 0, sizeof g_localAuroraPktBufferIndex);

#if 0
       int nLen = sizeof (AuroraPacketHead)  + (uFrameLen) *  sizeof (AuroraPacketBody) ;
       unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
       unsigned char cPackType = SRIO_PACKET_TYPE_NRead;
       SRIORWStatus  status = SRIOReadData( cFPGAID, uRdAddr, cPackType ,   g_localAuroraPktBuffer,   nLen);
       if (COMPLETED_NOERRORS == status)
       {
           return 0;
       }
       else
       {
           return (int)status;
       }
#else


         //  #ifdef _LITTLE_ENDIAN
         //
         //           volatile Uint32  *uipData =  (Uint32 *)( uRdAddr + sizeof(Uint32) );// (Uint32 *)( uRdAddr + i*sizeof(Uint32));
         //           *uipData = Reverse32( *uipData);
         //
         //  #endif
                     AuroraPacketHead*  head = (AuroraPacketHead*)uRdAddr;
                     if (head->transaction_type == 0)
                          return 0;

       //    #ifdef _LITTLE_ENDIAN
       //            int i = 0;
       //                for( i = 0;  i < (head->packet_len*2)  ; i++)
       //                {
       //                    uipData =  (Uint32 *)(uRdAddr + (i+2)*sizeof(Uint32));
       //                    //swap maintenance value for little endian
       //                    *uipData= _swap4(_packlh2(*uipData, *uipData));
       //                }
       //
       //     #endif



              memcpy(g_localAuroraPktBuffer, (void*)uRdAddr,  (head->packet_len+1)*8);
              return   (head->packet_len+1)*8;
#endif
}

//计算整数n的二进制形式中所含1的个数
Uint8  CountOne (int  n)
{
    Uint8  uCount = 0;
   // while(n)
   // {
   //     if (n%2 == 1)
   //         nCount++;
   //     n >>1;
   // }
        while(n)
        {
                n=n&(n-1);
                uCount++;
        }
    return uCount;
}

/*********************************************************************************************
*********                         Aurora 协议 应用                          ***************
*********************************************************************************************/
// 通用读DA寄存器操作函数

int   ReadADReg(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint32  uWriteRequestAddr, AuroraPacket* pPkt )
{
    if (0 == pPkt )  return -1;
    memset(g_localAuroraPktBuffer, 0,  256);//sizeof g_localAuroraPktBuffer);
    memset(g_localAuroraPktBufferIndex, 0, sizeof g_localAuroraPktBufferIndex);


    Uint8    uPacketSize = 1;  //4字节的倍数, 因为读只发一个数据头过去


    SetAuroraPacketHead(uRegAddress, Flag_Read, cRegLen, uChannel, cTransactionType, g_localAuroraPktBuffer);
    g_localAuroraPktBufferIndex[0] = 1;

      //      uReadAddr = 0x80110000 ;//+ (uSlotNum-1)*0x10000;
           //发送读请求的地址是0x110000, 0x120000,  按光模块的顺序递增
           //读结果数据的地址是0x80110x00, 0x80120y00, .....
    //        uWriteRequestAddr = 0x110000; //
           AuroraPacketHead*  pHeader = (AuroraPacketHead*)g_localAuroraPktBuffer;
           pHeader->packet_len = 0; //4字节的倍数, 0对应长度1, 1对应长度2, 2对应长度3
           {
               #ifdef _LITTLE_ENDIAN
                   volatile Uint32 * uipData = 0;
                   int i = 0;
                   for( i = 0;  i < 2 ; i++)
                   {
                       uipData =  (Uint32 *)(g_localAuroraPktBuffer + i*sizeof(Uint32));
                       //swap maintenance value for little endian
                       //*uipData= _swap4(_packlh2(*uipData, *uipData));
                       *uipData = Reverse32( *uipData);
                   }
               #endif
           }

           uReadAddr +=  (uRegAddress*0x100);
            //*(Uint32*)(uReadAddr +4)  = 0;
           memset((void*)uReadAddr, 0, 8*24);
          //*(Uint32*)(uReadAddr +4) = (*(Uint32*)(uReadAddr+4)) & 0x3FFFFFFF;

           if (0 != SendReadAuroraPacketRequest(    uWriteRequestAddr,  (1+uPacketSize)*4 ))
           {
               //
               printf("Read request Send Failed. \n");
               return -1;
           }

          int  nTryCount = 100;
           while ((0 == ReadAuroraPacket(/*uFrameLen*/ PacketBodyMax ,  uReadAddr)) &&  nTryCount >= 0)
           {
               //printf("Read packet Failed. \n");
               nTryCount--;
           }
          if (nTryCount <= 0)
              //memcpy(g_localAuroraPktBuffer, (void*)uRdAddr,  (head->packet_len+1)*8);
              printf("Read packet Failed. \n");
           if (1)
           { // 解析收到的数据
               pPkt->head     = (AuroraPacketHead*)g_localAuroraPktBuffer ;
               int i = 0;
               for (i = 0;  i <= pPkt->head->packet_len;  i++)
               {
                   pPkt->body[i] = (AuroraPacketBody*)(g_localAuroraPktBuffer + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
               }
           }
           return 0;
}


extern void * MyMemSet(void *dst, int fill, size_t len);
#ifdef  __cplusplus
#pragma CODE_SECTION("AuroraCode")
#else
#pragma  CODE_SECTION(WriteDAReg,  "AuroraCode")
#endif
int   WriteDAReg(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannelSel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pData, AuroraPacket* pPkt )
{
    // 写测试
             int  i = 0;
            //读光模块的槽位:
//            unsigned int uSlotNum = 2;
             //////////////////////////////////////////////////////////////////////////////////
             //Uint8  szPktBuffer[256];
             //Uint8  szPktBufferIndex[PacketBodyMax];
            // for ( i = 0;  i < sizeof g_localAuroraPktBufferIndex; i++)
            //     g_localAuroraPktBufferIndex[i] = '\0';
             MyMemSet(g_localAuroraPktBufferIndex, 0, sizeof  g_localAuroraPktBufferIndex);
             //memset(szPktBuffer, 0, sizeof  szPktBuffer);
             MyMemSet(g_localAuroraPktBuffer, 0, sizeof  g_localAuroraPktBuffer);


 //            printf("1.   pData[0] = 0x%x\n",  pData[0]);

             Uint8    uFrameCount = CountOne(uChannelSel);
             int  nChannel[PacketBodyMax];
             int  nValidIndex = 0;
             {
                 for (i = 0; i < PacketBodyMax;  i++)
                 {
                     nChannel[i] = 0;
                     if (uChannelSel &  (1  << i))
                     {
                         nChannel[nValidIndex] = i+1;
                         nValidIndex++;
                     }
                 }
             }

             SetAuroraPacketHead(uRegAddress, Flag_Write,  cRegLen, uChannelSel, cTransactionType, g_localAuroraPktBuffer);
             g_localAuroraPktBufferIndex[0] = 1;
             for( i = 0; i < nValidIndex; i++)
             {
                 SetAuroraPacketData(pData[i],  nChannel[i], g_localAuroraPktBuffer);
//                 printf("1.   pData[%d]=0x%x\n", i, (Uint32)pData[i] );
             }
             //SetAuroraPacketData(0x1234,  0);
             //SetAuroraPacketData(0x2234,  1);
             //SetAuroraPacketData(0x3234,  2);
             //SetAuroraPacketData(0x4234,  3);
             //uReadAddr = 0x80110000 ;//+ (uSlotNum-1)*0x10000;
             //发送读请求的地址是0x110000, 0x120000,  按光模块的顺序递增
             //读结果数据的地址是0x80110x00, 0x80120y00, .....
             //uWriteRequestAddr = 0x110000; //
             AuroraPacketHead*  pHeader = (AuroraPacketHead*)g_localAuroraPktBuffer;
             pHeader->packet_len = uFrameCount; //4字节的倍数
             if (0)
             {
                 #ifdef _LITTLE_ENDIAN
                     Uint32 * uipData = 0;
                     for( i = 0;  i < (uFrameCount+1)*2 ; i++)
                     {
                         uipData =  (Uint32 *)(g_localAuroraPktBuffer + i*sizeof(Uint32));
                         //swap maintenance value for little endian
                         //*uipData= _swap4(_packlh2(*uipData, *uipData));
                         *uipData = Reverse32( *uipData);
                     }
                 #endif
             }



//             printf("2.   Trans = 0x%x\n",  *(Uint32 *)(szPktBuffer + 3*sizeof(Uint32)));
             if (0 != WriteAuroraPacketWithBuffer(     uWriteRequestAddr,  (uFrameCount+1)*4 *2,  g_localAuroraPktBuffer))
             {
                 //
  //               printf("Write request Send failed. \n");
                 return -1;
             }

           //  int nTryCount = 0x2;
           //  uReadAddr +=  (uRegAddress*0x100);
           //  while (0  == ReadAuroraPacket(/*uFrameLen*/  PacketBodyMax ,  uReadAddr) &&  nTryCount >= 0)
           //  {
           //      nTryCount--;
           //      //printf("Write packet failed. \n");
           //      //return -1;
           //  }
           //  if (1)
           // { // 解析收到的数据
           //     pPkt->head     = (AuroraPacketHead*)g_localAuroraPktBuffer ;
           //     int i = 0;
           //     for (i = 0;  i < PacketBodyMax;  i++)
           //     {
           //              pPkt->body[i] =     (AuroraPacketBody*)(g_localAuroraPktBuffer + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
           //     }
           //
           // }
         return 0;
}

//版本时间寄存器
//ReadADReg();
// 0x0F,   Transaction_Type_Data, 0x80110000,  0x110000
int   ReadDAVersionTimeReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_VERSIONTIME_REG;
    Uint8 cRegLen  = 3;
    return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDAVersionTimeReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pVersionTimes,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_VERSIONTIME_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pVersionTimes,  pPkt );
}
//版本号寄存器
int   ReadDAVersionNOReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
        Uint32 uRegAddress =  SRIO_VERSION_NO_REG;
        Uint8 cRegLen  = 3;
        return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDAVersionNOReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pVersionNO,  AuroraPacket* pPkt  )
{
        Uint32 uRegAddress =  SRIO_VERSION_NO_REG;
        Uint8 cRegLen  = 3;
        WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pVersionNO,  pPkt );
}
//DAC接口逻辑复位寄存器
int   ReadDACLogicResetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_DAC_INTERFACE_RESET_REG;
    Uint8 cRegLen  = 3;
    return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDACLogicResetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pReset,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_DAC_INTERFACE_RESET_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pReset,  pPkt );
}

//SPI波特率设置寄存器
int   ReadSPIBaudSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_SPI_BAUD_SET_REG;
    Uint8 cRegLen  = 3;
    return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetSPIBaudSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pBaud,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_SPI_BAUD_SET_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pBaud,  pPkt );
}

//DAC测试模式使能寄存器
int   ReadDACTestModeEnReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
        Uint32 uRegAddress =  SRIO_DAC_TEST_MODE_EN_REG;
        Uint8 cRegLen  = 3;
        return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDACTestModeEnReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pModeEn,  AuroraPacket* pPkt  )
{
        Uint32 uRegAddress =  SRIO_DAC_TEST_MODE_EN_REG;
        Uint8 cRegLen  = 3;
        WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pModeEn,  pPkt );
}

//DAC测试波形选择寄存器
int   ReadDACTestWaveSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
        Uint32 uRegAddress =  SRIO_DAC_TEST_WAVE_SEL_REG;
        Uint8 cRegLen  = 3;
        return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDACTestWaveSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pWaveSel,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_WAVE_SEL_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pWaveSel,  pPkt );
}

//DAC测试数据更新速率寄存器
int   ReadDACTestUpdateVReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_UPDATE_V_REG;
    Uint8 cRegLen  = 3;
    return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDACTestUpdateVReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pUpdateV,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_UPDATE_V_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pUpdateV,  pPkt );
}

//DAC 测试波形频率设置寄存器
int   ReadDACTestFreqSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_FREQ_SET_REG;
    Uint8 cRegLen  = 3;
    return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDACTestFreqSetReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pFreqSet,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_FREQ_SET_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pFreqSet,  pPkt );
}

//DAC测试波形幅度选择寄存器
int   ReadDACTestRangeSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_RANGE_SEL_REG;
    Uint8 cRegLen  = 3;
    return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}
void SetDACTestRangeSelReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, Uint32* pRangeSel,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  SRIO_DAC_TEST_RANGE_SEL_REG;
    Uint8 cRegLen  = 3;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pRangeSel,  pPkt );
}

//AD 采样读
int   ReadADSampleReg(Uint16  uChannelSel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
     Uint32 uRegAddress =  4;
     Uint8 cRegLen  = 2;
     return ReadADReg( uRegAddress,  cRegLen,   uChannelSel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}

//DA  写
void SetDAReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pBuffer,  AuroraPacket* pPkt  )
{
    Uint32 uRegAddress =  1;
    Uint8 cRegLen  = 2;
    WriteDAReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pBuffer,  pPkt );
}

//AD 数据处理:  去掉最后的4位,  再求补码
inline int   GetDataComplete(Uint32   data)
{
        int  dataTmp = data >> 4;
        return dataTmp -0x80000;
}


//VME
//#define   SRIO_VME_AM_REG     0x0020                //VME地址修改码寄存器, [5:0]地址修改码,  [31:6]为保留
//#define   SRIO_VME_DS_REG      0x0028                //VME数据选通寄存器, [1:0]数据选通, [31:2]保留
//#define   SRIO_VME_LW_REG     0x002C                //VME字长选择寄存器, [0]字长选择, [31:1]保留
int  ReadVMEAMReg(Uint32 * pAddressModify )
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    Uint32   uRdAddr = 0x123100;   //SRIO_VME_BUS_BASE + SRIO_VME_AM_REG ;
    unsigned char cPackType = SRIO_PACKET_TYPE_NRead;
    unsigned char pLocalBuff[8] = {0};
    int nLen = 8;

    SRIORWStatus  status = SRIOReadData( cFPGAID, uRdAddr, cPackType ,   pLocalBuff,   nLen);
    if (COMPLETED_NOERRORS == status)
    {
        *pAddressModify = *((Uint32*)pLocalBuff);
        return 0;
    }
    else
    {
        *pAddressModify = 0;
        return (int)status;
    }
}
int  WriteVMESYSVersionReg(unsigned char*  uVersionTime, unsigned char* uVersionNum)
{
    unsigned char cFPGAID = FPGA_DEVICE_ID_8BIT;
    Uint32   uWrAddr = SRIO_VME_BUS_BASE + SRIO_VME_DS_REG;
    unsigned char cPackType = SRIO_PACKET_TYPE_NWRITE;
    unsigned char pLocalBuffer[8] ={0};
    int nLen = 8;

    pLocalBuffer[0] = uVersionTime[0];
    pLocalBuffer[1] = uVersionTime[1];
    pLocalBuffer[2] = uVersionTime[2];
    pLocalBuffer[3] = uVersionTime[3];

    pLocalBuffer[4] = uVersionNum[0] ;
    pLocalBuffer[5] = uVersionNum[1] ;
    pLocalBuffer[6] = uVersionNum[2] ;
    pLocalBuffer[7] = uVersionNum[3] ;
    SRIORWStatus  status = SRIOWriteData(cFPGAID,   uWrAddr,  cPackType,  pLocalBuffer,   nLen);
    if (COMPLETED_NOERRORS == status)
        return 0;
    else
        return (int)status;
}
