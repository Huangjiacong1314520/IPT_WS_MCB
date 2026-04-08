/*
 * EDMA3.h
 *
 *  Created on: 2020-8-31
 *      Author: linxi
 */

#ifndef EDMA3_H_
#define EDMA3_H_

#define EDMA3_TRIGMODE_MANUAL                (0u)
#define EDMA3_TRIGMODE_QDMA                  (1u)
#define EDMA3_TRIGMODE_EVENT                 (2u)

#define EDMA3_CHANNELTYPE_DMA                (0u)
#define EDMA3_CHANNELTYPE_QDMA               (1u)

#define EDMA3_SYNCTYPE_A                     (0u)
#define EDMA3_SYNCTYPE_AB                    (1u)

#define EDMA3_CC0                             (0)
#define EDMA3_CC1                             (1)
#define EDMA3_CC2                             (2)



typedef struct _EDMA3_PARAM_OPT_
{
	unsigned int  sam : 1;
	unsigned int  dam : 1;
	unsigned int  syncDim : 1;
	unsigned int  stat : 1;
	unsigned int  Reserved2 : 4;
	unsigned int  fwid : 3;
	unsigned int  tccMode : 1;
	unsigned int  tcc : 6;
	unsigned int  Reserved1 : 2;
	unsigned int  tcintEn : 1;
	unsigned int  itcintEn : 1;
	unsigned int  tcchEn : 1;
	unsigned int  itcchEn : 1;
	unsigned int  Reserved0 : 8;
}EDMA3Opt_st, *pEDMA3Opt_st;


typedef struct _EDMA3_CCPARAM_ENTRY_
{
        /** OPT field of PaRAM Set */
		EDMA3Opt_st opt;
        /**
         * \brief Starting byte address of Source
         * For FIFO mode, srcAddr must be a 256-bit aligned address.
         */
        unsigned int srcAddr;

        /**
         * \brief Number of bytes in each Array (ACNT)
         */
        unsigned short aCnt;

        /**
         * \brief Number of Arrays in each Frame (BCNT)
         */
        unsigned short bCnt;

        /**
         * \brief Starting byte address of destination
         * For FIFO mode, destAddr must be a 256-bit aligned address.
         * i.e. 5 LSBs should be 0.
         */
        unsigned int destAddr;

        /**
         * \brief Index between consec. arrays of a Source Frame (SRCBIDX)
         */
        short  srcBIdx;

        /**
         * \brief Index between consec. arrays of a Destination Frame (DSTBIDX)
         */
        short  destBIdx;

        /**
         * \brief Address for linking (AutoReloading of a PaRAM Set)
         * This must point to a valid aligned 32-byte PaRAM set
         * A value of 0xFFFF means no linking
         */
        unsigned short linkAddr;

        /**
         * \brief Reload value of the numArrInFrame (BCNT)
         * Relevant only for A-sync transfers
         */
        unsigned short bCntReload;

        /**
         * \brief Index between consecutive frames of a Source Block (SRCCIDX)
         */
        short  srcCIdx;

        /**
         * \brief Index between consecutive frames of a Dest Block (DSTCIDX)
         */
        short  destCIdx;

        /**
         * \brief Number of Frames in a block (CCNT)
         */
        unsigned short cCnt;

        /**
         * \brief  This field is Reserved. Write zero to this field.
         */
        unsigned short rsvd;

}EDMA3CCParam_st,*pEDMA3CCParam_st;

#define EDMA3_REGIONID    0

#ifdef __cplusplus
extern "C" {
#endif

int 		 EDMA3_Init(unsigned char nCC, unsigned int nCh, unsigned int nParamId, unsigned int intcNum, unsigned int evtQNum, EDMA3CCParam_st Param);
int 		 EDMA3_StartChTrans(unsigned char nCh);
unsigned int EDMA3_GetIntrStatus();
unsigned int EDMA3_GetHighIntrStatus();
int 		 EDMA3_WaitForTransComplete(unsigned int intcNum);
void 		 EDMA3_ClrIntr(unsigned int intcNum);
int 		 EDMA3_UnInit(unsigned char nCh, unsigned int intcNum, unsigned int evtQNum);


#ifdef __cplusplus
}
#endif

#endif /* EDMA3_H_ */
