#include "srio_syn_serial.h"
#include "../drv_srio.h"

typedef unsigned int  size_t   ;
extern void SetDAReg(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr, const Uint32* pBuffer,  AuroraPacket* pPkt  );

extern  void * MyMemSet(void *dst, int fill, size_t len);

extern Uint8 g_localAuroraPktBuffer [256];
extern Uint8  g_localAuroraPktBufferIndex[32] ;


int   ReadRegCommon(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannelSel, Uint8  cTransactionType, Uint8  uChannel, /*Uint32  uWriteRequestAddr, */ AuroraPacket* pPkt)
{
      int i = 0;
          // AD, DA测试
              //读光模块的槽位:
   //            unsigned int uSlotNum = 2;
               //////////////////////////////////////////////////////////////////////////////////

        MyMemSet(g_localAuroraPktBuffer,  0,  sizeof g_localAuroraPktBuffer);  //g_localAuroraPktBuffer

        Uint32 uReadAddr =  uChannel*0x10000 + 0x80100000;
        uReadAddr += uRegAddress*0x100;
        int  nNReadRet = SRIOReadData(0xff,  uReadAddr,  SRIO_PACKET_TYPE_NRead , g_localAuroraPktBuffer,  sizeof g_localAuroraPktBuffer);
        if (0 != nNReadRet)
                 return -1;


        //int  szDAReg[PacketBodyMax];
        //         AuroraPacket packet ;
        //         for (i = 0;  i <  PacketBodyMax -1;  i+=2)
                 {
         //            szDAReg[i] = 0 ;        szDAReg[i+1] =  0;
         //            pPkt->body[i] = 0;   pPkt->body[i+1] = 0;
                 }
                 {
 //                    Writeback_Cache(3, (void*)g_localAuroraPktBuffer ,  sizeof g_localAuroraPktBuffer, 1);
 //                    Invalidate_Cache(1, (void*)g_localAuroraPktBuffer ,  sizeof g_localAuroraPktBuffer, 1);

                     int nCount = 0;
                     volatile Uint32* pData = (Uint32*) (g_localAuroraPktBuffer  + 4);
                        while( 0 == (*pData  ))
                        {
                            nCount++;
                            if  (nCount >= 0xFFF)
                                return -1;
                        }
                        //nCount = 0;
                 }

                 pPkt->head     = (AuroraPacketHead*)g_localAuroraPktBuffer ;
                 for (i = 0;  i <= pPkt->head->packet_len;  i++)
                 {
                     pPkt->body[i] = (AuroraPacketBody*)(g_localAuroraPktBuffer + sizeof(AuroraPacketHead) + sizeof(AuroraPacketBody)*i);
          //           szDAReg[i]  = GetDataComplete(pPkt->body[0]->data );
                    // szDAReg[i]  = szDAReg[i] >> 4;
                    // szDAReg[i]  -= 0x80000;
                 }



      return 0;
}


int   WriteRegCommon(Uint32 uRegAddress, Uint8 cRegLen, Uint16  uChannelSel,   Uint8  cTransactionType,  Uint32  uReadAddr, Uint8 uChannel, const Uint32* pData, AuroraPacket* pPkt )
{

        //WriteDAReg( uRegAddress,  cRegLen,   uChannelSel,    cTransactionType,    uReadAddr,   uWriteRequestAddr,  pData,  pPkt );
        int i = 0;


        Uint8   szPktBuffer[256];
        _nassert((int)szPktBuffer %16 == 0);
        MyMemSet(szPktBuffer,  0,  sizeof szPktBuffer);  //g_localAuroraPktBuffer
        MyMemSet(g_localAuroraPktBufferIndex, 0, sizeof  g_localAuroraPktBufferIndex);

        Uint8    uFrameCount = CountOne(uChannelSel);
         int  nValidChannel[PacketBodyMax];
         int  nValidIndex = 0;
         {
             for (i = 0; i < PacketBodyMax;  i++)
             {
                 nValidChannel[i] = 0;
                 if (uChannelSel &  (1  << i))
                 {
                     nValidChannel[nValidIndex] = i+1;
                     nValidIndex++;
                 }
             }
         }

         SetAuroraPacketHead(uRegAddress, Flag_Write,  cRegLen, uChannelSel, cTransactionType, szPktBuffer);
         g_localAuroraPktBufferIndex[0] = 1;
         for( i = 0; i < nValidIndex; i++)
         {
             SetAuroraPacketData(pData[i],  nValidChannel[i], szPktBuffer);
         }
         //SetAuroraPacketData(0x1234,  0);
         //SetAuroraPacketData(0x2234,  1);
         //SetAuroraPacketData(0x3234,  2);
         //SetAuroraPacketData(0x4234,  3);
         //uReadAddr = 0x80110000 ;//+ (uSlotNum-1)*0x10000;
         //发送读请求的地址是0x110000, 0x120000,  按光模块的顺序递增
         //读结果数据的地址是0x80110x00, 0x80120y00, .....
         //uWriteRequestAddr = 0x110000; //
         AuroraPacketHead*  pHeader = (AuroraPacketHead*)szPktBuffer;
         pHeader->packet_len = uFrameCount; //4字节的倍数

         Uint32  uWriteRequestAddr=  uChannel*0x10000 + 0x100000 + uRegAddress*0x100;
//      printf("2.   Trans = 0x%x\n",  *(Uint32 *)(szPktBuffer + 3*sizeof(Uint32)));
         if (0 != WriteAuroraPacketWithBuffer(     uWriteRequestAddr,  (uFrameCount+1)*4 *2,  szPktBuffer))
         {
//             printf("Write request Send failed. \n");
             return -1;
         }
        return 0;
}


/****************************************************************************/
/*                                                                          */
/*              AD卡用户寄存器列表                                                 */
/*                                                                          */
/****************************************************************************/
//AD卡寄存器
int    ReadADRegSysVerTime(Uint16  uChannel, Uint8  cTransactionType, Uint32  uReadAddr, Uint32  uWriteRequestAddr,  AuroraPacket* pPkt)
{
    Uint32 uRegAddress =  SRIO_VERSIONTIME_REG;
        Uint8 cRegLen  = 3;
        return ReadADReg( uRegAddress,  cRegLen,   uChannel,    cTransactionType,    uReadAddr,   uWriteRequestAddr, pPkt );
}


