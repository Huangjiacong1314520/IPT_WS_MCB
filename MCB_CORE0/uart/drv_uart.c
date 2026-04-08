/*
 * uart.c
 *
 *  Created on: 2019年10月19日
 *      Author: zhuyb
 */

#include"../drv_uart.h"
#include <ti/csl/cslr_uart.h>
//#include "evmc66x_uart.h"

#define UART_RBR         (0x0)
#define UART_THR         (0x0)
#define UART_IER         (0x4)
#define UART_IIR         (0x8)
#define UART_FCR         (0x8)
#define UART_LCR         (0xC)
#define UART_MCR         (0x10)
#define UART_LSR         (0x14)
#define UART_MSR         (0x18)
#define UART_SCR         (0x1C)
#define UART_DLL         (0x20)
#define UART_DLH         (0x24)
#define UART_REVID1      (0x28)
#define UART_REVID2      (0x2C)
#define UART_PWREMU_MGMT (0x30)
#define UART_MDR         (0x34)

/****************************************************************************/
/*                                                                          */
/*              位域定义                                                    */
/*                                                                          */
/****************************************************************************/
/* RBR */
#define UART_RBR_DATA           (0x000000FFu)
#define UART_RBR_DATA_SHIFT          (0x00000000u)

/* THR */
#define UART_THR_DATA           (0x000000FFu)
#define UART_THR_DATA_SHIFT          (0x00000000u)

/* IER */
#define UART_IER_EDSSI          (0x00000008u)
#define UART_IER_EDSSI_SHIFT         (0x00000003u)
#define UART_IER_ELSI           (0x00000004u)
#define UART_IER_ELSI_SHIFT          (0x00000002u)
#define UART_IER_ETBEI          (0x00000002u)
#define UART_IER_ETBEI_SHIFT         (0x00000001u)
#define UART_IER_ERBI           (0x00000001u)
#define UART_IER_ERBI_SHIFT          (0x00000000u)

/* IIR */
#define UART_IIR_FIFOEN         (0x000000C0u)
#define UART_IIR_FIFOEN_SHIFT        (0x00000006u)
#define UART_IIR_INTID          (0x0000000Eu)
#define UART_IIR_INTID_SHIFT         (0x00000001u)
/*----INTID Tokens----*/
#define UART_IIR_INTID_MODSTAT       (0x00000000u)
#define UART_IIR_INTID_THRE          (0x00000001u)
#define UART_IIR_INTID_RDA           (0x00000002u)
#define UART_IIR_INTID_RLS           (0x00000003u)
#define UART_IIR_INTID_CTI           (0x00000006u)
#define UART_IIR_IPEND          (0x00000001u)
#define UART_IIR_IPEND_SHIFT         (0x00000000u)

/* FCR */
#define UART_FCR_RXFIFTL        (0x000000C0u)
#define UART_FCR_RXFIFTL_SHIFT       (0x00000006u)
/*----RXFIFTL Tokens----*/
#define UART_FCR_RXFIFTL_CHAR1       (0x00000000u)
#define UART_FCR_RXFIFTL_CHAR4       (0x00000001u)
#define UART_FCR_RXFIFTL_CHAR8       (0x00000002u)
#define UART_FCR_RXFIFTL_CHAR14      (0x00000003u)
#define UART_FCR_DMAMODE1       (0x00000008u)
#define UART_FCR_DMAMODE1_SHIFT      (0x00000003u)
#define UART_FCR_TXCLR          (0x00000004u)
#define UART_FCR_TXCLR_SHIFT         (0x00000002u)
#define UART_FCR_RXCLR          (0x00000002u)
#define UART_FCR_RXCLR_SHIFT         (0x00000001u)
#define UART_FCR_FIFOEN         (0x00000001u)
#define UART_FCR_FIFOEN_SHIFT        (0x00000000u)

/* LCR */
#define UART_LCR_DLAB           (0x00000080u)
#define UART_LCR_DLAB_SHIFT          (0x00000007u)
#define UART_LCR_BC             (0x00000040u)
#define UART_LCR_BC_SHIFT            (0x00000006u)
#define UART_LCR_SP             (0x00000020u)
#define UART_LCR_SP_SHIFT            (0x00000005u)
#define UART_LCR_EPS            (0x00000010u)
#define UART_LCR_EPS_SHIFT           (0x00000004u)
/*----EPS Tokens----*/
#define UART_LCR_EPS_ODD             (0x00000000u)
#define UART_LCR_EPS_EVEN            (0x00000001u)
#define UART_LCR_PEN            (0x00000008u)
#define UART_LCR_PEN_SHIFT           (0x00000003u)
#define UART_LCR_STB            (0x00000004u)
#define UART_LCR_STB_SHIFT           (0x00000002u)

#define UART_LCR_WLS            (0x00000003u)
#define UART_LCR_WLS_SHIFT           (0x00000000u)
/*----WLS Tokens----*/
#define UART_LCR_WLS_5BITS           (0x00000000u)
#define UART_LCR_WLS_6BITS           (0x00000001u)
#define UART_LCR_WLS_7BITS           (0x00000002u)
#define UART_LCR_WLS_8BITS           (0x00000003u)

/* MCR */
#define UART_MCR_AFE            (0x00000020u)
#define UART_MCR_AFE_SHIFT           (0x00000005u)
#define UART_MCR_LOOP           (0x00000010u)
#define UART_MCR_LOOP_SHIFT          (0x00000004u)
#define UART_MCR_OUT2           (0x00000008u)
#define UART_MCR_OUT2_SHIFT          (0x00000003u)
#define UART_MCR_OUT1           (0x00000004u)
#define UART_MCR_OUT1_SHIFT          (0x00000002u)
#define UART_MCR_RTS            (0x00000002u)
#define UART_MCR_RTS_SHIFT           (0x00000001u)
#define UART_MCR_DTR            (0x00000001u)
#define UART_MCR_DTR_SHIFT           (0x00000000u)

/* LSR */
#define UART_LSR_RXFIFOE        (0x00000080u)
#define UART_LSR_RXFIFOE_SHIFT       (0x00000007u)
#define UART_LSR_TEMT           (0x00000040u)
#define UART_LSR_TEMT_SHIFT          (0x00000006u)
#define UART_LSR_THRE           (0x00000020u)
#define UART_LSR_THRE_SHIFT          (0x00000005u)
#define UART_LSR_BI             (0x00000010u)
#define UART_LSR_BI_SHIFT            (0x00000004u)
#define UART_LSR_FE             (0x00000008u)
#define UART_LSR_FE_SHIFT            (0x00000003u)
#define UART_LSR_PE             (0x00000004u)
#define UART_LSR_PE_SHIFT            (0x00000002u)
#define UART_LSR_OE             (0x00000002u)
#define UART_LSR_OE_SHIFT            (0x00000001u)
#define UART_LSR_DR             (0x00000001u)
#define UART_LSR_DR_SHIFT            (0x00000000u)

/* MSR */
#define UART_MSR_CD             (0x00000080u)
#define UART_MSR_CD_SHIFT            (0x00000007u)
#define UART_MSR_RI             (0x00000040u)
#define UART_MSR_RI_SHIFT            (0x00000006u)
#define UART_MSR_DSR            (0x00000020u)
#define UART_MSR_DSR_SHIFT           (0x00000005u)
#define UART_MSR_CTS            (0x00000010u)
#define UART_MSR_CTS_SHIFT           (0x00000004u)
#define UART_MSR_DCD            (0x00000008u)
#define UART_MSR_DCD_SHIFT           (0x00000003u)
#define UART_MSR_TERI           (0x00000004u)
#define UART_MSR_TERI_SHIFT          (0x00000002u)
#define UART_MSR_DDSR           (0x00000002u)
#define UART_MSR_DDSR_SHIFT          (0x00000001u)
#define UART_MSR_DCTS           (0x00000001u)
#define UART_MSR_DCTS_SHIFT          (0x00000000u)

/* SCR */
#define UART_SCR_SCR            (0x000000FFu)
#define UART_SCR_SCR_SHIFT           (0x00000000u)

/* DLL */
#define UART_DLL_DLL            (0x000000FFu)
#define UART_DLL_DLL_SHIFT           (0x00000000u)

/* DLH */
#define UART_DLH_DLH            (0x000000FFu)
#define UART_DLH_DLH_SHIFT           (0x00000000u)

/* PID1 */
#define UART_REVID1_REV           (0xFFFFFFFFu)
#define UART_REVID1_REV_SHIFT          (0x00000000u)

/* PID2 */


#define UART_REVID2_REV           (0xFFFFFFFFu)
#define UART_REVID2_REV_SHIFT          (0x00000000u)

/* PWREMU_MGMT */

#define UART_PWREMU_MGMT_UTRST  (0x00004000u)
#define UART_PWREMU_MGMT_UTRST_SHIFT (0x0000000Eu)

#define UART_PWREMU_MGMT_URRST  (0x00002000u)
#define UART_PWREMU_MGMT_URRST_SHIFT (0x0000000Du)
#define UART_PWREMU_MGMT_FREE   (0x00000001u)
#define UART_PWREMU_MGMT_FREE_SHIFT  (0x00000000u)

/* MDR */
#define UART_MDR_OSM_SEL        (0x00000001u)
#define UART_MDR_OSM_SEL_SHIFT       (0x00000000u)


#undef USE_PLATFORM_LIB

#define UART_TX_FIFO_LENGTH           16
#define UART_RX_FIFO_LENGTH           16
#define UART_MAX_TRIAL_COUNT          0x0FFF


#define UART_RX_TRIG_LEVEL_1          (UART_FCR_RXFIFTL_CHAR1                  \
                                                << UART_FCR_RXFIFTL_SHIFT)

#define UART_RX_TRIG_LEVEL_4          (UART_FCR_RXFIFTL_CHAR4                  \
                                                << UART_FCR_RXFIFTL_SHIFT)

#define UART_RX_TRIG_LEVEL_8          (UART_FCR_RXFIFTL_CHAR8                  \
                                                << UART_FCR_RXFIFTL_SHIFT)

#define UART_RX_TRIG_LEVEL_14         (UART_FCR_RXFIFTL_CHAR14                 \
                                                << UART_FCR_RXFIFTL_SHIFT)

#define UART_RX_TRIG_LEVEL            UART_FCR_RXFIFTL


#define UART_DMAMODE                  UART_FCR_DMAMODE1

#define UART_TX_CLEAR                 UART_FCR_TXCLR

#define UART_RX_CLEAR                 UART_FCR_RXCLR

#define UART_FIFO_MODE                UART_FCR_FIFOEN

#define UART_DMA_MODE                 (UART_FCR_DMAMODE1                  \
                                                 << UART_FCR_DMAMODE1_SHIFT)

#define UART_PARITY_ODD               UART_LCR_PEN

#define UART_PARITY_EVEN              (UART_LCR_PEN | UART_LCR_EPS)

#define UART_PARITY_STICK_ODD         (UART_PARITY_ODD | UART_LCR_SP)

#define UART_PARITY_STICK_EVEN        (UART_PARITY_EVEN | UART_LCR_SP)



#define UART_DLAB                      UART_LCR_DLAB

#define UART_BREAK_CTRL                UART_LCR_BC

#define UART_STICK_PARITY              UART_LCR_SP

#define UART_SET_PARITY_TYPE           UART_LCR_EPS

#define UART_PARITY                    UART_LCR_PEN

#define UART_STOP_BIT                  UART_LCR_STB

#define UART_WORDL                     UART_LCR_WLS

#define UART_WORDL_5BITS               UART_LCR_WLS_5BITS

#define UART_WORDL_6BITS               UART_LCR_WLS_6BITS

#define UART_WORDL_7BITS               UART_LCR_WLS_7BITS

#define UART_WORDL_8BITS               UART_LCR_WLS_8BITS



#define UART_RXFIFO_ERROR               UART_LSR_RXFIFOE

#define UART_THR_TSR_EMPTY              UART_LSR_TEMT

#define UART_THR_EMPTY                  UART_LSR_THRE

#define UART_BREAK_IND                  UART_LSR_BI

#define UART_FRAME_ERROR                UART_LSR_FE


#define UART_DATA_READY                 UART_LSR_DR


#define UART_INT_TX_EMPTY              UART_IER_ETBEI

#define UART_INT_RXDATA_CTI            UART_IER_ERBI


#define UART_FIFOEN_STAT              UART_IIR_FIFOEN

#define UART_INTID                    UART_IIR_INTID

#define UART_INTID_TX_EMPTY           UART_IIR_INTID_THRE

#define UART_INTID_RX_DATA            UART_IIR_INTID_RDA

#define UART_INTID_RX_LINE_STAT       UART_IIR_INTID_RLS

#define UART_INTID_CTI                UART_IIR_INTID_CTI

#define UART_INTID_IPEND              UART_IIR_IPEND


#define UART_AUTOFLOW                 UART_MCR_AFE

#define UART_LOOPBACK                 UART_MCR_LOOP

#define UART_OUT2_CTRL                UART_MCR_OUT2

#define UART_OUT1_CTRL                UART_MCR_OUT1
/* Request To Send(RTS) */
#define UART_RTS                      UART_MCR_RTS

#define UART_CD                       UART_MSR_CD
#define UART_RI                       UART_MSR_RI
#define UART_DSR                      UART_MSR_DSR
#define UART_CTS                      UART_MSR_CTS

#define UART_DCD                      UART_MSR_DCD
#define UART_TERI                     UART_MSR_TERI
#define UART_DDSR                     UART_MSR_DDSR
#define UART_DCTS                     UART_MSR_DCTS


#define UART_TX_RST_ENABLE            UART_PWREMU_MGMT_UTRST

#define UART_RX_RST_ENABLE            UART_PWREMU_MGMT_URRST

#define UART_FREE_MODE                UART_PWREMU_MGMT_FREE


#define UART_OVER_SAMP_RATE             UART_MDR_OSM_SEL

#define UART_OVER_SAMP_RATE_16          UART_MDR_OSM_SEL_SHIFT

#define UART_OVER_SAMP_RATE_13          UART_MDR_OSM_SEL



#define UART_INT_MODEM_STAT                 (0x00000008u)
#define UART_INT_LINE_STAT                  (0x00000004u)
#define UART_INT_THR                        (0x00000002u)
#define UART_INT_RHR_CTI                    (0x00000001u)


#define UART0_BASE_ADDR     (0x2540000)



#define LCR_WLS_SHIFT       0
#define LCR_WLS_MASK        0x3

#define LCR_STB_SHIFT       2
#define LCR_STB_MASK        0x4

#define LCR_PEN_SHIFT       3
#define LCR_PEN_MASK        0x8

#define LCR_EPS_SHIFT       4
#define LCR_EPS_MASK        0x10

#define LCR_SP_SHIFT        5
#define LCR_SP_MASK         0x20

#define LCR_BC_SHIFT        6
#define LCR_BC_MASK         0x40

#define LCR_DLAB_SHIFT      7
#define LCR_DLAB_MASK       0x80

#define MGMT_UTRST_SHIFT    14
#define MGMT_UTRST_MASH     0x4000

#define MGMT_URRST_SHIFT    13
#define MGMT_URRST_MASK     0x2000

#define MGMT_FREE_SHIFT     0

//LSR
#define LSR_DR_SHIFT        0
#define LSR_OE_SHIFT        1
#define LSR_PE_SHIFT        2
#define LSR_FE_SHIFT        3
#define LSR_BI_SHIFT        4
#define LSR_THRE_SHIFT      5   //发送保持寄存器为空
#define LSR_TEMT_SHIFT      6   //发送为空
#define LSR_RXFIFOE         7   //接收ERROR

#define LSR_THRE_EMPTY      (1 <<LSR_THRE_SHIFT)
#define LSR_TEMT_EMPTY      (1 << LSR_TEMT_SHIFT)
#define LSR_DR_READY        1



/*
* UART 寄存器定义
*/
typedef struct
{
	volatile Uint32 RBR;
	volatile Uint32 IER;
	volatile Uint32 IIR;
	volatile Uint32 LCR;
	volatile Uint32 MCR;
	volatile Uint32 LSR;
	volatile Uint32 MSR;
	volatile Uint32 SCR;
	volatile Uint32 DLL;
	volatile Uint32 DLH;
	volatile Uint32 REVID1;
	volatile Uint32 REVID2;
	volatile Uint32 PWREMU_MGMT;
	volatile Uint32 MDR;
}UART_REG_ST, *PUART_REG_ST;


/*
* UART 配置定义
*/
typedef struct
{
	unsigned char nUartMode;    //中断、轮训
	unsigned char nDataBit;     //数据位
	unsigned char nStopBit;     //停止位
	unsigned char nParityBit;   //校验位
	unsigned int nBaudRate;     //停止位
	unsigned int nOverSampRate; //分频因子

	PUART_REG_ST pUartReg;      //寄存器

}UART_CFG;











UART_CFG UartCfg;

void UART_Init(UART_CFG *pUartCfg)
{
#ifndef USE_PLATFORM_LIB
    unsigned int divisor = 0;
    unsigned int cfg = 0;
    unsigned int sysClk = 1000000000;
    unsigned int uartClk = sysClk/6;

    switch(pUartCfg->nOverSampRate)
    {
        case OSM_SEL_16X:
        {
            divisor = uartClk/(pUartCfg->nBaudRate * 16);
            pUartCfg->pUartReg->MDR = OSM_SEL_16X;
            break;
        }
        case OSM_SEL_13X:
        {
            divisor = uartClk/(pUartCfg->nBaudRate * 13);
            pUartCfg->pUartReg->MDR = OSM_SEL_13X;
           break;
        }
        default:
        {
            divisor = uartClk/(pUartCfg->nBaudRate * 16);
            pUartCfg->pUartReg->MDR = OSM_SEL_16X;
            break;
        }
    }

    //波特率配置
    pUartCfg->pUartReg->DLL =(divisor & 0xFF);
    pUartCfg->pUartReg->DLH = ((divisor>>8) & 0xFF);

    //数据位、停止位、校验位配置
#if 1
    cfg |= ((pUartCfg->nDataBit<<LCR_WLS_SHIFT) & LCR_WLS_MASK);
    cfg |= ((pUartCfg->nStopBit<<LCR_STB_SHIFT) & LCR_STB_MASK);
    if((pUartCfg->nParityBit != PARITY_NO ))
    {
        cfg |= ((1 << LCR_PEN_SHIFT) | (pUartCfg->nParityBit<<LCR_EPS_SHIFT));
    }
#else
    cfg = 3;
    cfg &= ( UART_STOP_BIT | UART_WORDL |          \
            UART_PARITY | UART_SET_PARITY_TYPE |  \
            UART_STICK_PARITY | UART_BREAK_CTRL | \
            UART_DLAB);
#endif
    pUartCfg->pUartReg->LCR = cfg;

    //SP = 0, BC = 0 DLAB = 0

    //使能发送 接收
    cfg = 0;
    cfg = ((1 <<MGMT_UTRST_SHIFT ) | (1 << MGMT_URRST_SHIFT) | (1 << MGMT_FREE_SHIFT));
    pUartCfg->pUartReg->PWREMU_MGMT = cfg;

    //接收中断
    //pUartCfg->pUartReg->IIR=(UART_FIFO_MODE | UART_DMAMODE | UART_RX_CLEAR | UART_TX_CLEAR);
    //pUartCfg->pUartReg->IIR=((UART_RX_TRIG_LEVEL_8 & (UART_RX_TRIG_LEVEL)) | UART_FIFO_MODE);

    unsigned int intFlags = 0;
    	intFlags |= (UART_INT_LINE_STAT  |  \
    				 UART_INT_TX_EMPTY |    \
    				 UART_INT_RXDATA_CTI);
    pUartCfg->pUartReg->IER= (intFlags & (UART_INT_MODEM_STAT | UART_INT_LINE_STAT |
            UART_INT_TX_EMPTY | UART_INT_RXDATA_CTI));

#else
    UartInit();
    UartSetBaudRate(115200);

#endif
    return;
}

unsigned int UART_PollGetChar(UART_CFG *pUartCfg)
{
   while ((pUartCfg->pUartReg->LSR & LSR_DR_READY) == 0);

   return (unsigned int)(pUartCfg->pUartReg->RBR);
}

void UART_PollPutChar(UART_CFG *pUartCfg, char nData)
{
    unsigned int nTxEmpty = (LSR_THRE_EMPTY | LSR_TEMT_EMPTY);
    volatile unsigned int nRead = 0;

    nRead = pUartCfg->pUartReg->LSR;

    while (nTxEmpty!= (pUartCfg->pUartReg->LSR & nTxEmpty))
    {
        nRead = pUartCfg->pUartReg->LSR;
    }

    pUartCfg->pUartReg->THR = nData;

}

unsigned int UART_PollGetLine(UART_CFG *pUartCfg, char *pRxBuffer,int nBytes)
{
    int nCount = 0;
    char *pBuffer = pRxBuffer;

    if(nBytes <= 0)
    {
        return 0;
    }

    do
    {
#ifndef USE_PLATFORM_LIB
        *pBuffer = (unsigned char)UART_PollGetChar(pUartCfg);
#else
        *pBuffer = (unsigned char)UartReadData();
#endif
        /*
         * 0xD  - ASCII
         * 0x1B - ASCII ESC
         */
        if(0xD == *pBuffer || 0x1B == *pBuffer)
        {
            *pBuffer = '\n';
            nCount++;

            pBuffer++;
            *pBuffer = '\0';
            break;
        }

        pBuffer++;
        nCount++;

        if((nCount == nBytes))
        {
            break;
        }

    }while(1);

    return nCount;
}

unsigned int UART_PollPutLine(UART_CFG *pUartCfg, char *pTxBuffer,int nBytes)
{
    int nSendCount = 0;
    char *pBuffer = pTxBuffer;

    if(nBytes <= 0)
    {
        return 0;
    }

    while('\0' != *pBuffer)
    {
#ifndef USE_PLATFORM_LIB
        if('\n' == *pBuffer)
        {
            UART_PollPutChar(pUartCfg,'\r');
            UART_PollPutChar(pUartCfg,'\n');
        }
        else
        {
            UART_PollPutChar(pUartCfg,*pBuffer);
        }
#else
       if('\n' == *pBuffer)
       {
           UartWriteData('\r');
           UartWriteData('\n');
       }
       else
       {
           UartWriteData(*pBuffer);
       }

#endif
        pBuffer++;
        nSendCount++;

        if((nSendCount == nBytes))
        {
           break;
        }

    }
    return nSendCount;
}


unsigned int UART_GetLine(UART_CFG *pUartCfg, char *pRxBuffer, int nBytes)
{
	int nCount = 0;
	char *pBuffer = pRxBuffer;

	if (nBytes <= 0)
	{
		return 0;
	}

	do
	{
		int i=0xFFFF;
		while((pUartCfg->pUartReg->LSR & LSR_DR_READY) == 0)
		{
			i--;
			if(i == 0)
			{
				return nCount;
			}
		}

		*pBuffer = (pUartCfg->pUartReg->RBR);
#if 0
		/*
		* 0xD  - ASCII
		* 0x1B - ASCII ESC
		*/
		if (0xD == *pBuffer || 0x1B == *pBuffer)
		{
			*pBuffer = '\n';
			nCount++;

			pBuffer++;
			*pBuffer = '\0';
			break;
		}
#endif
		pBuffer++;
		nCount++;

		if ((nCount == nBytes))
		{
			break;
		}

	} while (1);

	return nCount;
}




void InitUart(UartCfg_st uartcfg)
{
    UartCfg.nBaudRate = uartcfg.nBaudRate;
    UartCfg.nDataBit = uartcfg.nDataBit;
    UartCfg.nParityBit = uartcfg.nParityBit;
    UartCfg.nStopBit = uartcfg.nStopBit;
    UartCfg.nUartMode = uartcfg.nUartMode;
    UartCfg.nOverSampRate = uartcfg.nOverSampRate;
    UartCfg.pUartReg = (PUART_REG_ST)UART0_BASE_ADDR;
    UART_Init(&UartCfg);
    return;
}

int Uart_Printf(char *pBuf, int nLen)
{  
    return  UART_PollPutLine(&UartCfg, pBuf, nLen);
}

int Uart_RecvInfo(unsigned char * pBuf, int nLen)
{
	//return UART_PollGetLine(&UartCfg,(char*)pBuf,nLen);
	return UART_GetLine(&UartCfg, (char*)pBuf, nLen);
}


int Uart_GetIntStatus()
{
	return UartCfg.pUartReg->IIR>>1;
}
