/**
 *  \file   nandlib.c
 *
 *  \brief  This file contains the NAND prtocol abstraction layer 
 *          macro definitions and function definitions.
 *
 */

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
*/
/*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#include "../drv_Nand.h"
#include "hw_types.h"
#include "hw_soc_c6678.h"
/*******************************************************************************
*                       INTERNAL MACRO DEFINITIONS
*******************************************************************************/

/*****************************************************************************/
/*
** Macros which defines the NAND commands.
*/
#define NAND_CMD_READ                           (0x00u)
#define NAND_CMD_READ_SPARE                     (0x50u)

#define NAND_CMD_LO_PAGE                    (0)     /* Read the lo page CMD */
#define NAND_CND_HI_PAGE                    (1)     /* Read the hi page CMD */

#define NAND_CMD_READ_CYC2                      (0x30u)
#define NAND_CMD_READID                         (0x90u)
#define NAND_CMD_RESET                          (0xFFu)
#define NAND_CMD_PROGRAM                        (0x80u)
#define NAND_CMD_PROGRAM_CYC2                   (0x10u)
#define NAND_CMD_ERASE                          (0x60u)
#define NAND_CMD_ERASE_CYC2                     (0xD0u)
#define NAND_CMD_READ_STATUS                    (0x70u)
#define NAND_CMD_READ_RANDOM                    (0x05u)
#define NAND_CMD_READ_RANDOM_CYC2               (0xE0u)




NandInfo_t EMIFNand;
/*****************************************************************************/
/*
** NAND RB wait timeout.
*/
#define NAND_RBWAIT_TIMEOUT                     (0xFFFFu)

/*****************************************************************************/
/*
** NAND command status mask  defintions.
*/
#define NAND_CMD_STATUS_PASSFAIL_MASK          (0x01)
#define NAND_CMD_STATUS_DEVREADY_MASK          (0x20)
#define NAND_CMD_STATUS_WRPROTECT_MASK         (0x80)

/******************************************************************************
**                      INTERNAL FUNCTION DEFINITIONS
*******************************************************************************/

/**
* \brief  Delay function for NAND device interface. \n
*
*         This function provides a delay functionality for the NAND device
*         interface module. The parameter specified as delay is not related
*         to any time. It is just a count to execute a dummy loop.\n
*
* \param  delay         : Number of cycles in the delay loop.\n
*
* \return none.\n
*/
static void NANDDelay(unsigned int delay)
{
	unsigned int Delay=delay;
    while (Delay)
    {
    	Delay--;
    }
}

/**
* \brief  Function to write command to the NAND command register.\n
*
* \param  cmdRegAddr    : Command register address.\n
*
* \param  cmd           : Command to write.\n
*
* \return none.\n
*/
static void NANDCommandWrite(unsigned int cmdRegAddr, unsigned int cmd)
{
    (*(volatile unsigned char*)(cmdRegAddr)) = cmd;
}

/**
* \brief  Function to write the address to the NAND address register.\n
*
* \param  addrRegAddr   : Address register address.\n
*
*         addr          : Address to write.\n
*
* \return none.\n
*/
static void NANDAddressWrite(unsigned int addrRegAddr, unsigned int addr)
{
    (*(volatile unsigned char*)(addrRegAddr)) = (unsigned char)addr;
}



/**
* \brief  Function to wait till NAND is Ready. \n
*
*         This function Waits untill NAND device enters the  ready stateIt
*         reads the status of the NAND's ready/busy status by reading the
*         wait  status from the NANDFSR register. It also implements a
*         timeout functionality to avoid indefinte lockup.\n
*
* \param  pNandInfo      :  Pointer to structure which conatins controller and
*                          device information.\n
*
* \return NAND device status.\n
*
*         NAND_STATUS_PASSED      : If device is ready.\n
*         NAND_STATUS_WAITTIMEOUT : If device is not ready.\n
*/
NandStatus_t NANDWaitUntilReady()
{
    NandStatus_t retVal;
    unsigned int waitPinStatus;
    volatile int timeout;

    retVal  = NAND_STATUS_PASSED;
    timeout = NAND_RBWAIT_TIMEOUT;

    /* This function is called immediatly after issuing commands    *
     * to the NAND flash. Since the NAND flash takes sometime to    *
     * pull the R/B line low,it would be safe to introduce a delay  *
     * before checking the ready/busy status.                       */
    NANDDelay(0xFFF);
    /* Keep checking the status of ready/busy line.Also maintain a  *
     * timeout count. If the NAND is not ready during the timeout   *
     * period, stop checking the ready/busy status                  */
    while (timeout > 0u)
    {
        /* Check the Ready/Busy status                              */
        waitPinStatus = ((EMIFNand.hNandCtrlInfo.EMIF_WaitPinStatusGet)(EMIFNand.hNandCtrlInfo.waitPin));
        if (waitPinStatus != 0)
        /* Note: Check "should" be (waitPinStatus != 1); Need to investigate why
         * wait pin (~R/B pin) is not toggled on AM335x platform. */
        {
            /* NAND flash is ready. Break from the loop. */
            break;
        }
        timeout = timeout - 1u;
    }
    /* Determine if the wait for ready status ended due to timeout. */
    if (0 == timeout)
    {
        retVal = NAND_STATUS_WAITTIMEOUT;
    }
    return (retVal);
}




static NandStatus_t EMIFAHammingCodeECCInit()
{
	unsigned int cs = EMIFNand.hNandCtrlInfo.currChipSelect;

	EMIF_NANDCSSet(cs);

	EMIFNand.hNandEccInfo.eccOffSet = NAND_ECC_1BIT_HAMMINGCODE_OOB_OFFSET + EMIFNand.pageSize;
	EMIFNand.hNandEccInfo.eccByteCnt = NAND_ECC_1BIT_HAMMINGCODE_BYTECNT;

	return NAND_STATUS_PASSED;
}




/**
*\brief This function does the Reed-Solomon ECC related initializes to the NAND
*       controller.\n
*
* \param  pNandInfo      : Pointer to structure containing controller and
*                         device information.\n
*
* \return
*        NAND_STATUS_PASSED          : On success.\n
*        NAND_STATUS_FAILED          : On failure.\n
*        NAND_STATUS_ECC_UNSUPPORTED : If unsupported ECC is used.\n
*
*/
static NandStatus_t EMIFAReedSolomonECCInit()
{
	unsigned int cs = EMIFNand.hNandCtrlInfo.currChipSelect;

	EMIF_NANDCSSet(cs);
	EMIF_NAND4BitECCSelect(cs);

	EMIFNand.hNandEccInfo.eccOffSet = 6;    //¿ÕÏÐÇøÆ«ÒÆ6×Ö½Ú
	EMIFNand.hNandEccInfo.eccByteCnt = NAND_ECC_RS_4BIT_UNUSED_BYTECNT + NAND_ECC_RS_4BIT_BYTECNT;

	return NAND_STATUS_PASSED;
}


/**
*\brief This function reads/calculates the 1-bit hamming code ECC values.\n
*
* \param  pNandInfo      : Pointer to structure containing controller and
*                         device information.\n
*
* \param  eccResReg     : ECC Result register value.\n
*
* \param  eccRead       : Pointer where read ECC data has to store.\n
*
* \return none.\n
*
*/
static void EMIFAHammingCodeECCCalculate(unsigned char *ptrEccData)
{
	unsigned int cs = EMIFNand.hNandCtrlInfo.currChipSelect;

	unsigned int eccVal;

	eccVal = EMIF_NANDEccValGet(EMIFA_NAND_1BIT_ECC, cs);
	/* Squeeze 4 bytes ECC into 3 bytes by removing RESERVED bits
	and shifting. RESERVED bits are 31 to 28 and 15 to 12. */
	eccVal = (eccVal & 0x00000fff) | ((eccVal & 0x0fff0000) >> 4);

	/* Invert so that erased block ECC is correct */
	eccVal = ~eccVal;

	*ptrEccData++ = (unsigned char)(eccVal >> 0);
	*ptrEccData++ = (unsigned char)(eccVal >> 8);
	*ptrEccData++ = (unsigned char)(eccVal >> 16);
}


/**
*\brief This function reads/calculates the Reed-Solomon 4-bit andc 8-bit ECC values.\n
*
* \param  pNandInfo      : Pointer to structure containing controller and
*                         device information.\n
*
* \param  ptrEccData    : Pointer where read ECC data has to store.\n
*
* \return none.\n
*
*/
static void EMIFAReedSolomonECCCalculate(unsigned char *ptrEccData)
{
	int i = 0;
	unsigned int eccParity[8];

	for (i = 0; i < 8; i++)
	{
		eccParity[i] = EMIF_NANDEccValGet(EMIFA_NAND_4BIT_ECC, i + 1);
	}

	/* Convert 8 10-bit values into 10 8-bit values */
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 0] = eccParity[0] & (0xFFu);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 1] = ((eccParity[1] & (0x3Fu)) << 2) |
		((eccParity[0] & (0x300u)) >> 8);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 2] = ((eccParity[2] & (0x0Fu)) << 4) |
		((eccParity[1] & (0x3C0u)) >> 6);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 3] = ((eccParity[3] & (0x03u)) << 6) |
		((eccParity[2] & (0x3F0u)) >> 4);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 4] = ((eccParity[3] & (0x3FCu)) >> 2);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 5] = eccParity[4] & (0xFFu);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 6] = ((eccParity[5] & (0x3Fu)) << 2) |
		((eccParity[4] & (0x300u)) >> 8);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 7] = ((eccParity[6] & (0x0Fu)) << 4) |
		((eccParity[5] & (0x3C0u)) >> 6);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 8] = ((eccParity[7] & (0x03u)) << 6) |
		((eccParity[6] & (0x3F0u)) >> 4);
	ptrEccData[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 9] = ((eccParity[7] & (0x3FCu)) >> 2);

}

/**
* \brief This function checks for ECC errors using 1-bit hamming code algorithm
*        and correct if any ECC errors.\n
*
* \param  pNandInfo      : Pointer to structure containing controller and
*                         device information.\n
*
* \param   eccRead      : Pointer to the ECC data which is read from the spare
*                         area.\n
*
* \param   data         : Pointer to the data, where if an ecc error need to
*                         correct.\n
*
* \return ECC correction Status.\n
*    NAND_STATUS_PASSED                        : If no ecc errors.\n
*    NAND_STATUS_READ_ECC_ERROR_CORRECTED      : If error are corrected.\n
*    NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR  : If errors are uncorrectable.\n
*
*/
static NandStatus_t EMIFAHammingCodeECCCheckAndCorrect(unsigned char *eccRead,
	unsigned char *data)
{
	NandStatus_t retVal;
	unsigned char eccCalc[4];
	unsigned int readEccVal;
	unsigned int calcEccVal;
	unsigned int eccDiffVal;
	unsigned int bitPos;
	unsigned int bytePos;

	retVal = NAND_STATUS_PASSED;

	EMIFAHammingCodeECCCalculate(&eccCalc[0]);
	readEccVal = eccRead[0] | (eccRead[1] << 8) | (eccRead[2] << 16);
	calcEccVal = eccCalc[0] | (eccCalc[1] << 8) | (eccCalc[2] << 16);
	eccDiffVal = readEccVal ^ calcEccVal;

	if (eccDiffVal)
	{
		/*
		* No error              : The ecc diff value (eccDiffVal) is 0.
		* Correctable error     : For 512-byte inputs, ecc diff value has
		*                         12 bits at 1. For 256 byte ecc diff value has
		*                         11 bits at 1.
		* ECC error             : The ecc diff value has only 1 bit at 1.
		* Non-correctable error : The ecc diff value provides all other results
		*/

		/*
		* Beow condition checks for number of 1's in eccDiffValu.
		* Since Total ecc has 3bytes = 24 bits. Make 2 halfs and XOR.
		* If eccDiffVal has  12 1's, it produces the result 0xFFF.
		*/
		if ((((eccDiffVal >> 12) ^ eccDiffVal) & 0xfff) == 0xfff)
		{
			/* Correctable error */
			/* Check bytePos is within NAND_BYTES_PER_TRNFS i.e 512 */
			if ((eccDiffVal >> (12 + 3)) < NAND_BYTES_PER_TRNFS)
			{
				bitPos = 1 << ((eccDiffVal >> 12) & 7);
				bytePos = eccDiffVal >> (12 + 3);
				data[bytePos] ^= bitPos;
				retVal = NAND_STATUS_READ_ECC_ERROR_CORRECTED;
			}
			else
			{
				retVal = NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR;
			}
		}
		else if (!(eccDiffVal & (eccDiffVal - 1)))
		{
			/* Single bit ECC error in the ECC itself,nothing to fix */
			retVal = NAND_STATUS_READ_ECC_ERROR_CORRECTED;
		}
		else
		{
			retVal = NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR;
		}
	}

	return (retVal);
}


/**
* \brief This function checks for ECC errors using Reed-Solomon algorithm and corrects
*        if any ECC errors. \n
*
* \param  pNandInfo      : Pointer to structure containing controller and
*                         device information.\n
*
* \param   eccRead      : Pointer to the ECC data which is read from the spare
*                         area.\n
*
* \param   data         : Pointer to the data, where if an ecc error need to
*                         correct.\n
*
* \return ECC correction Status.\n
*    NAND_STATUS_PASSED                        : If no ecc errors.\n
*    NAND_STATUS_READ_ECC_ERROR_CORRECTED      : If error are corrected.\n
*    NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR  : If errors are uncorrectable.\n
*
*/
static NandStatus_t EMIFAReedSolomonECCCheckAndCorrect(unsigned char *eccRead,
	unsigned char *data)
{
	int i = 0;

	NandStatus_t retVal = NAND_STATUS_PASSED;
	unsigned int eccVal;
	unsigned int eccParity1[8];


	/* Dummy read to clear the 4BITECCSTART bit */
	for (i = 1; i <= 8; i++)
	{
		EMIF_NANDEccValGet(EMIFA_NAND_4BIT_ECC, i);
	}
	retVal = NAND_STATUS_PASSED;

	/* Convert 10 8-bit values into 8 10-bit values */
	eccParity1[0] = ((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 1] & (0x3u)) << 8) |
		(eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 0]);
	eccParity1[1] = ((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 2] & (0xFu)) << 6) |
		((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 1] & (0xFCu)) >> 2);
	eccParity1[2] = ((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 3] & (0x3Fu)) << 4) |
		((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 2] & (0xF0u)) >> 4);
	eccParity1[3] = (unsigned int)((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 4] << 2) |
		((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 3] & (0xC0u)) >> 6));
	eccParity1[4] = ((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 6] & (0x3u)) << 8) |
		(eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 5]);
	eccParity1[5] = ((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 7] & (0xFu)) << 6) |
		((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 6] & (0xFCu)) >> 2);
	eccParity1[6] = ((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 8] & (0x3Fu)) << 4) |
		((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 7] & (0xF0u)) >> 4);
	eccParity1[7] = (unsigned int)((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 9] << 2) |
		((eccRead[NAND_ECC_RS_4BIT_UNUSED_BYTECNT + 8] & (0xC0u)) >> 6));

	/*
	* Write the parity values in the NAND Flash 4-bit ECC Load
	* register. The parity data is written in reverse order to
	* ECC Load reg.Write each parity value one at a time starting
	* from 4bit_ecc_val8 to 4bit_ecc_val1.                         */
	for (i = 7; i >= 0; i--)
	{
		EMIF_NAND4BitECCLoad(eccParity1[i]);
	}
	/*
	* Perform a dummy read to the EMIF Revision Code and Status
	* register. This is required to ensure time for syndrome
	* calculation after writing the ECC values in previous step    */
	EMIF_ModuleIdRead();

	/*
	* Read the syndrome from the NAND Flash 4-Bit ECC 1-4
	* registers. A syndrome value of 0 means no bit errors. If
	* the syndrome is non-zero then go further otherwise return.   */
	for (i = 1; i <= 8; i++)
	{
		eccVal = EMIF_NANDEccValGet(EMIFA_NAND_4BIT_ECC, i);
		if (eccVal != 0)
		{
			retVal = NAND_STATUS_READ_ECC_ERROR_CORRECTED;
			break;
		}
	}

	/* One or more errors appears to exist, try to correct */
	if (retVal != NAND_STATUS_PASSED)
	{
		unsigned int eccState = 0;

		/*
		* Clear any previous address calculation by doing a dummy read
		* of an error address register.
		*/
		EMIF_NAND4BitEccErrAddrGet(EMIFA_4BITECC_ERRADDR_INDEX_1);
		EMIF_NAND4BitEccErrAddrGet(EMIFA_4BITECC_ERRADDR_INDEX_2);
		EMIF_NAND4BitEccErrAddrGet(EMIFA_4BITECC_ERRADDR_INDEX_3);
		EMIF_NAND4BitEccErrAddrGet(EMIFA_4BITECC_ERRADDR_INDEX_4);

		/* Start the Error Address Calculation */
		EMIF_NAND4BitECCAddrCalcStart();

		/* Wait for the 4-bit ECC state to be 1,2 or 3 */
		do {
			eccState = EMIF_NAND4BitECCStateGet();
		} while (eccState >= 0x04);

		if (eccState == EMIFA_4BITECC_CORRECTION_ECCSTATE_0)
		{
			retVal = NAND_STATUS_PASSED;
		}
		else if (eccState == EMIFA_4BITECC_CORRECTION_ECCSTATE_2 ||
			eccState == EMIFA_4BITECC_CORRECTION_ECCSTATE_3)
		{
			unsigned int numErrors, errAddr, errVal;
			/* Read the number of errors */
			numErrors = EMIF_NAND4BitECCNumOfErrsGet();
			for (i = 0; i <= numErrors; i++)
			{
				errAddr = EMIF_NAND4BitEccErrAddrGet((i + 1));
				errVal = EMIF_NAND4BitEccErrValGet((i + 1));

				/* Address of the errored byte */
				errAddr = ((NAND_BYTES_PER_TRNFS + 7) - errAddr);
				/* Correct the error */
				data[errAddr] ^= (unsigned char)(errVal);
			}
			retVal = NAND_STATUS_READ_ECC_ERROR_CORRECTED;
		}
		else
		{
			retVal = NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR;
		}
	}

	return (retVal);
}


/**
* \brief  Function to read the data from NAND data register.\n
*
* \param  dataRegAddr   : data register address.\n
*
* \return Data from nand data register.\n
*/
static unsigned char NANDDataReadByte(unsigned int dataRegAddr)
{
    unsigned char dataByte;

    dataByte = (unsigned char)(*(volatile unsigned char*)(dataRegAddr));

    return(dataByte);
}

/**
* \brief  Function to read the NAND previous command status.\n
*
*         This function retrives the status of previous command.\n
*
* \param  pNandInfo   :  Pointer to structure which contains  
*                       device information.\n
*
* \return NAND status containing below info.\n
*
*         NAND_STATUS_PASSED         : If the previous command is passed.\n
*         NAND_STATUS_FAILED         : If the previous command is failed.\n
*         NAND_STATUS_DEVBUSY        : If the device is busy.\n
*         NAND_STATUS_DEVWRPROTECT   : If the device is write protect.\n
*         NAND_STATUS_WAITTIMEOUT    : If the RB pin inidcating device 
*                                      busy status.\n
*
*/
static NandStatus_t NANDDeviceStatusGet()
{
    unsigned char nandStatus;
    NandStatus_t retVal = NAND_STATUS_PASSED;

    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_READ_STATUS);
    NANDDelay(10);

    nandStatus = NANDDataReadByte(EMIFNand.dataRegAddr);
    if(nandStatus & NAND_CMD_STATUS_PASSFAIL_MASK)
    {
        retVal |= NAND_STATUS_FAILED;
    }
    if( !(nandStatus & NAND_CMD_STATUS_DEVREADY_MASK) )
    {
        retVal |= NAND_STATUS_DEVBUSY;
    }
    if( !(nandStatus & NAND_CMD_STATUS_WRPROTECT_MASK) )
    {
        retVal |= NAND_STATUS_DEVWRPROTECT;
    }

    return retVal;
}

/**
* \brief  Function to write data to the NAND for specified number of bytes.\n
*
* \param  pNandInfo      :  Pointer to structure which conatins controller and 
*                         device information.\n
*
* \param  txData        :  Pointer to the array containing the data to write.\n
*
* \param  size          :  Transfer count.\n
*
* \return none.\n
*/
static void NANDDataWrite(volatile unsigned char *txData,
                   unsigned int size)
{
    unsigned short data;
    
    if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
    {
        unsigned short *ptrData = (unsigned short *)txData;
        while(size)
        {
            data = *ptrData;
           (*(volatile unsigned short*)(EMIFNand.dataRegAddr)) =
                                        data;
            ptrData++;
            size   -= 2;
        }
    }
    else
    {
        while(size)
        {
            (*(volatile unsigned char*)(EMIFNand.dataRegAddr)) =
                                       *txData;
            txData++;
            size--;
        }
    }
}

/**
* \brief  Function to read the data from NAND for specified number of bytes.\n
*
* \param  pNandInfo      :  Pointer to structure which conatins controller and 
*                          device information.\n
*
* \param  rxData        :  Pointer to the array where read data has to place.\n
*
* \param  size          :  Transfer count.\n
*
* \return none.\n
*/
static void NANDDataRead(volatile unsigned char *rxData,
                  unsigned int size)
{
    unsigned short data;
    unsigned short *ptrData;

    if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
    {
        ptrData = (unsigned short *)rxData;
        while(size)
        {
            data = (*(volatile unsigned short*)
                     (EMIFNand.dataRegAddr));
            *ptrData   = data;
            size   -= 2;
            ptrData++;
        }
    }
    else
    {
        while(size)
        {
            *rxData = (*(volatile unsigned char*)
                        (EMIFNand.dataRegAddr));
            rxData++;
            size--;
        }
    }
}

/**
* \brief  Function to start the page write command sequence.\n
*
*       This function forms the address, based on  the block, page and column.
*       Then the command for Page write is sent along with the address.\n
*
* \param  pNandInfo      : Pointer to structure which conatins controller and 
*                         device information.\n
*
* \param  blkNum        : Blk Number.\n
*
* \param  pageNum       : Page Number.\n
*
* \param  columnAddr    : Column Addr.\n
*
* \return none.\n
**/
static void NANDPageWriteCmdStart(unsigned int blkNum,
                           unsigned int pageNum, unsigned int columnAddr)
{
    unsigned int addr;
    NandStatus_t retVal;


    addr = PACK_ADDR(columnAddr,pageNum,blkNum);

    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_LO_PAGE);

    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_PROGRAM);

  //  NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_PROGRAM_CYC2);

    NANDDelay(2000);


    /* Write 4 bytes of column addr */
	NANDAddressWrite(EMIFNand.addrRegAddr,
		(unsigned char)((addr >> 0u) & 0xFF));// A0-A7  1st Cycle;  column addr
	NANDAddressWrite(EMIFNand.addrRegAddr,
		(unsigned char)((addr >> 9u) & 0xFF));// A9-A16 2nd Cycle;  page   addr & blk
	NANDAddressWrite(EMIFNand.addrRegAddr,
		(unsigned char)((addr >> 17u) & 0xFF)); // A17-A24 3rd Cycle; Block addr
	NANDAddressWrite(EMIFNand.addrRegAddr,
		(unsigned char)((addr >> 25u) & 0x1));// A25-A26  4th Cycle;  Plane addr

    retVal = NANDWaitUntilReady();
   if( (retVal & NAND_STATUS_WAITTIMEOUT) )
   {
       retVal = NANDDeviceStatusGet();
   }


    return;
}



/**
*\brief  Function to end the page write sequence.\n
*
*       This function forms the address, based on  the block, page and column.
*       Then the command for Page write is sent along with the address.\n
*
* \param  pNandInfo      : Pointer to structure which conatins controller and 
*                         device information.\n
*
* \return Write command status.\n
*
*         NAND_STATUS_PASSED        : If the write command is passed.\n
*         NAND_STATUS_FAILED        : If the write command is failed.\n
*         NAND_STATUS_DEVBUSY       : If the device is busy.\n
*         NAND_STATUS_DEVWRPROTECT  : If the device is write protect.\n
*         NAND_STATUS_WAITTIMEOUT   : If the RB pin inidcating device 
*                                     busy status.\n
*
*/
static NandStatus_t NANDPageWriteCmdEnd()
{
    NandStatus_t retVal;

    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_PROGRAM_CYC2);
    retVal = NANDWaitUntilReady();
    if( (retVal & NAND_STATUS_WAITTIMEOUT) )
    {
        retVal = NANDDeviceStatusGet();
    }

    return (retVal);
}

/**
*\brief  Function to start the page read command sequence.\n
*
*       This function forms the address, based on  the block, page and column.
*       Then the command for Page read is sent along with the address.\n
*
* \param blkNum         : Blk Number.\n
*
* \param pageNum        : Page Number.\n
*
* \param columnAddr     : Column Addr.\n
*
* \return NAND device status.\n
*
*         NAND_STATUS_PASSED      : If device is ready.\n
*         NAND_STATUS_WAITTIMEOUT : If device is not ready.\n
*
**/
static NandStatus_t NANDPageReadCmdSend(unsigned int blkNum,
                                 unsigned int pageNum, unsigned int columnAddr)
{
    NandStatus_t retVal;

    unsigned int addr;


   addr = PACK_ADDR(columnAddr,pageNum,blkNum);


   NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_READ);
   NANDDelay(1000);

   NANDAddressWrite(EMIFNand.addrRegAddr,
                    (unsigned char)((addr >>  0u) & 0xFF));// A0-A7  1st Cycle;  column addr
   NANDAddressWrite(EMIFNand.addrRegAddr,
                    (unsigned char)((addr >>  9u) & 0xFF));// A9-A16 2nd Cycle;  page   addr & blk
   NANDAddressWrite(EMIFNand.addrRegAddr,
                      (unsigned char)((addr >> 17u) & 0xFF)); // A17-A24 3rd Cycle; Block addr
   NANDAddressWrite(EMIFNand.addrRegAddr,
                      (unsigned char)((addr >> 25u) & 0x1));// A25-A26  4th Cycle;  Plane addr

   retVal = NANDWaitUntilReady();
  if( (retVal & NAND_STATUS_WAITTIMEOUT) )
  {
      retVal = NANDDeviceStatusGet();
  }


    return retVal;
}

static NandStatus_t NANDReadSpareCmdSend(unsigned int blkNum,
                                 unsigned int pageNum, unsigned int columnAddr)
{
    NandStatus_t retVal;

    unsigned int addr;


     addr = PACK_ADDR(columnAddr,pageNum,blkNum);

     NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_READ_SPARE);

     NANDDelay(2000);

	 NANDAddressWrite(EMIFNand.addrRegAddr,
		 (unsigned char)((addr >> 0u) & 0xFF));// A0-A7  1st Cycle;  column addr
	 NANDAddressWrite(EMIFNand.addrRegAddr,
		 (unsigned char)((addr >> 9u) & 0xFF));// A9-A16 2nd Cycle;  page   addr & blk
	 NANDAddressWrite(EMIFNand.addrRegAddr,
		 (unsigned char)((addr >> 17u) & 0xFF)); // A17-A24 3rd Cycle; Block addr
	 NANDAddressWrite(EMIFNand.addrRegAddr,
		 (unsigned char)((addr >> 25u) & 0x1));// A25-A26  4th Cycle;  Plane addr

     NANDDelay(2000);
     retVal = NANDWaitUntilReady();
    if( (retVal & NAND_STATUS_WAITTIMEOUT) )
    {
        retVal = NANDDeviceStatusGet();
    }


    return retVal;
}
static NandStatus_t NANDWriteSpareCmdSend(unsigned int blkNum,
                                 unsigned int pageNum, unsigned int columnAddr)
{
    NandStatus_t retVal;

    unsigned int addr;


     addr = PACK_ADDR(columnAddr,pageNum,blkNum);

     NANDCommandWrite(EMIFNand.cmdRegAddr, 0x50);
     NANDDelay(20);

     NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_PROGRAM);

     NANDDelay(20);

     NANDAddressWrite(EMIFNand.addrRegAddr,
                      (unsigned char)((addr >>  0u) & 0xFF));// A0-A7  1st Cycle;  column addr
     NANDAddressWrite(EMIFNand.addrRegAddr,
                      (unsigned char)((addr >>  9u) & 0xFF));// A9-A16 2nd Cycle;  page   addr & blk
     NANDAddressWrite(EMIFNand.addrRegAddr,
                        (unsigned char)((addr >> 17u) & 0xFF)); // A17-A24 3rd Cycle; Block addr
     NANDAddressWrite(EMIFNand.addrRegAddr,
                        (unsigned char)((addr >> 25u) & 0x1));// A25-A26  4th Cycle;  Plane addr

     NANDDelay(20);
     retVal = NANDWaitUntilReady();
    if( (retVal & NAND_STATUS_WAITTIMEOUT) )
    {
        retVal = NANDDeviceStatusGet();
    }


    return retVal;
}

 NandStatus_t NANDWriteSpare(unsigned int blkNum,
                                 unsigned int pageNum, unsigned int columnAddr,volatile unsigned char *txData,int size)
{
    NandStatus_t retVal;

    NANDWriteSpareCmdSend(blkNum,pageNum, columnAddr);

    NANDDataWrite(txData,size);

    retVal = NANDPageWriteCmdEnd();

    return retVal;

}

 NandStatus_t NANDReadSpare(unsigned int blkNum,
                                  unsigned int pageNum, unsigned int columnAddr,volatile unsigned char *txData,int size)
 {
     NandStatus_t retVal;

     retVal = NANDReadSpareCmdSend(blkNum, pageNum, columnAddr);

     NANDDataRead(txData,size);

     return retVal;
 }


 NandStatus_t EMIFANANDECCInit()
 {
	 NandStatus_t retVal;
	 retVal = NAND_STATUS_PASSED;

	 if (EMIFNand.eccType == NAND_ECC_ALGO_HAMMING_1BIT)
	 {
		 retVal = EMIFAHammingCodeECCInit();
	 }
	 else if (EMIFNand.eccType == NAND_ECC_ALGO_RS_4BIT)
	 {
		 retVal = EMIFAReedSolomonECCInit();
	 }

	 return(retVal);
 }

 /**
 *\brief This function enable the ECC.\n
 *
 * \param  nandCtrlInfo  : Pointer to structure containing controller info.\n
 *
 * \return none.\n
 *
 */
 void EMIFANANDECCEnable()
 {
	 unsigned int cs = EMIFNand.hNandCtrlInfo.currChipSelect;

	 if (EMIFNand.eccType == NAND_ECC_ALGO_HAMMING_1BIT)
	 {
		 EMIF_NANDECCStart(EMIFA_NAND_1BIT_ECC, cs);
	 }
	 else if (EMIFNand.eccType == NAND_ECC_ALGO_RS_4BIT)
	 {
		 EMIF_NAND4BitECCSelect(cs);
		 EMIF_NANDECCStart(EMIFA_NAND_4BIT_ECC, cs);
	 }
 }

 /**
 *\brief This function disable the ECC.\n
 *
 * \param  nandCtrlInfo  : Pointer to structure containing controller info.\n
 *
 * \return none.\n
 *
 */
 void EMIFANANDECCDisable()
 {
	 /* Nothing to do here */
 }

 /**
 *\brief  This Function does the ECC setting for write.\n
 *
 * \param  pNandInfo      : Pointer to structure containing controller and
 *                         device information.\n
 *
 * \return none.\n
 *
 */
 void EMIFANANDECCWriteSet()
 {
	 /* Nothing to do here */
 }

 /**
 *\brief  This Function does the ECC setting for read.\n
 *
 * \param  pNandInfo      : Pointer to structure containing controller and
 *                         device information.\n
 *
 * \return none.\n
 *
 */
 void EMIFANANDECCReadSet()
 {
	 /* Nothing to do here */
 }

 /**
 *\brief This function read the ecc data.\n
 *
 * \param  pNandInfo      : Pointer to structure containing controller and
 *                         device information.\n
 *
 * \param   eccRead      : Pointer where read ECC data has to store.\n
 *
 *
 * \return none.\n
 *
 */
 void EMIFANANDECCCalculate(unsigned char* eccWrite)
 {
	 if (EMIFNand.eccType == NAND_ECC_ALGO_HAMMING_1BIT)
	 {
		 EMIFAHammingCodeECCCalculate(eccWrite);
	 }
	 else if (EMIFNand.eccType == NAND_ECC_ALGO_RS_4BIT)
	 {
		 EMIFAReedSolomonECCCalculate(eccWrite);
	 }
 }

 int NandDataVerify(unsigned int blKNum, unsigned int pageNum, volatile unsigned char * txData)
 {
	 unsigned char RxBuf[NAND_PAGESIZE_512BYTES] = { 0 };
	 if (NAND_STATUS_PASSED != NANDPageRead(blKNum, pageNum, RxBuf))
	 {
		 return -1;
	 }
	 int i = 0;
	 for (i = 0; i < NAND_PAGESIZE_512BYTES; i++)
	 {
		 if (RxBuf[i] != txData[i])
		 {
			 return -1;
		 }
	 }
	 return 0;
 }

 /**
 *\brief This function checks and corrects ECC errors.\n
 *
 * \param  pNandInfo      : Pointer to structure containing controller and
 *                         device information.\n
 *
 * \param   eccRead      : Pointer to the ECC data which is read from the spare
 *                         area.\n
 *
 * \param   data         : Pointer to the data, where if an ecc error need to
 *                         correct.\n
 *
 * \return ECC correction Status.\n
 *    NAND_STATUS_PASSED                        : If no ecc errors.\n
 *    NAND_STATUS_READ_ECC_ERROR_CORRECTED      : If error are corrected.\n
 *    NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR  : If errors are uncorrectable.\n
 *
 */
 NandStatus_t EMIFANANDECCCheckAndCorrect(unsigned char* ReadData, unsigned char *eccRead)
 {
	 NandStatus_t retVal = NAND_STATUS_PASSED;

	 if (EMIFNand.eccType == NAND_ECC_ALGO_HAMMING_1BIT)
	 {
		 retVal = EMIFAHammingCodeECCCheckAndCorrect(eccRead, ReadData);
	 }
	 else if (EMIFNand.eccType == NAND_ECC_ALGO_RS_4BIT)
	 {
		 retVal = EMIFAReedSolomonECCCheckAndCorrect(eccRead, ReadData);
	 }

	 return(retVal);
 }

/******************************************************************************
**                       GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/

/**
* \brief  Function to Initialize the NAND and associated memory controller.\n
*
*       This function calls the registered controller init function.\n
*
* \param  pNandInfo      : Pointer to structure which conatins controller and 
*                         device information.\n
*
* \return
*        NAND_STATUS_PASSED          : On success.\n
*        NAND_STATUS_FAILED          : On failure.\n
*        NAND_STATUS_NOT_FOUND       : On NAND not found in the system.\n
*        NAND_STATUS_WAITTIMEOUT     : On NAND response timed out.\n
*
**/
NandStatus_t NANDOpen(NandInfo_t NandInfo)
{
	memcpy(&EMIFNand,&NandInfo,sizeof(NandInfo_t));
	switch (EMIFNand.hNandCtrlInfo.currChipSelect)
	{
		case EMIFA_CHIP_SELECT_0:
			EMIFNand.dataRegAddr = (SOC_EMIFA_CS0_ADDR + 0x00);
			EMIFNand.addrRegAddr = (SOC_EMIFA_CS0_ADDR + 0x2000);
			EMIFNand.cmdRegAddr = (SOC_EMIFA_CS0_ADDR + 0x4000);
			break;
		case EMIFA_CHIP_SELECT_1:
			EMIFNand.dataRegAddr = (SOC_EMIFA_CS1_ADDR + 0x00);
			EMIFNand.addrRegAddr = (SOC_EMIFA_CS1_ADDR + 0x2000);
			EMIFNand.cmdRegAddr = (SOC_EMIFA_CS1_ADDR + 0x4000);
			break;
		case EMIFA_CHIP_SELECT_2:
			EMIFNand.dataRegAddr = (SOC_EMIFA_CS2_ADDR + 0x00);
			EMIFNand.addrRegAddr = (SOC_EMIFA_CS2_ADDR + 0x2000);
			EMIFNand.cmdRegAddr = (SOC_EMIFA_CS2_ADDR + 0x4000);
			break;
		case EMIFA_CHIP_SELECT_3:
			EMIFNand.dataRegAddr = (SOC_EMIFA_CS3_ADDR + 0x00);
			EMIFNand.addrRegAddr = (SOC_EMIFA_CS3_ADDR + 0x2000);
			EMIFNand.cmdRegAddr = (SOC_EMIFA_CS3_ADDR + 0x4000);
			break;
		default:
			break;
	}
	
	EMIFNand.hNandEccInfo.eccOffSet = 6;
	EMIFNand.hNandEccInfo.eccByteCnt = 10;
	EMIFNand.pageSize = NAND_PAGESIZE_512BYTES;
	EMIFNand.blkSize = NAND_BLOCKSIZE_16KB;
	EMIFNand.hNandEccInfo.ECCInit = EMIFAReedSolomonECCInit;
	EMIFNand.hNandEccInfo.ECCEnable = EMIFANANDECCEnable;
	EMIFNand.hNandEccInfo.ECCCalculate = EMIFANANDECCCalculate;
	EMIFNand.hNandEccInfo.ECCCheckAndCorrect = EMIFAReedSolomonECCCheckAndCorrect;
    NandStatus_t retVal;

	/* Initialize the controller */
	(EMIFNand.hNandCtrlInfo.EMIF_Init)((int)EMIFNand.hNandCtrlInfo.currChipSelect,(unsigned char)EMIFNand.busWidth,EMIFNand.hNandCtrlInfo.nTiming);


    /* Init the ECC hardware/structures */
    if (NAND_ECC_ALGO_NONE != EMIFNand.eccType)
    {
        retVal = EMIFNand.hNandEccInfo.ECCInit();
        if (retVal & NAND_STATUS_FAILED)
        {
            return retVal;
        }
    }


    /* Reset the device first */
    retVal = NANDReset();
    if(retVal & NAND_STATUS_WAITTIMEOUT)
    {
        return retVal;
    }

    return retVal;    
}



/**
* \brief  Function to reset the NAND.\n
*
*         This function Resets the NAND device by issuing the RESET command
*         (0xFF) to the NAND device. It then waits until the NAND device is.\n
*
* \param  pNandInfo      :  Pointer to structure which conatins controller and 
*                          device information.\n
*
* \return NAND device status.\n
*
*         NAND_STATUS_PASSED      : If device is ready.\n
*         NAND_STATUS_WAITTIMEOUT : If device is not ready.\n
*
*/
NandStatus_t NANDReset()
{
    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_RESET);


    return NANDWaitUntilReady();
}

/**
* \brief  Function to read the NAND Device ID.\n
*
*         This function reads the NAND device ID . It issues the
*         NAND read ID command and reads a 4-byte NAND device ID.\n
*
* \param  pNandInfo      :  Pointer to structure which conatins controller and 
*                          device information.\n
*
* \return Updates the device id info in the nandDevInfo structure in pNandInfo.\n
*/
 NandStatus_t NANDReadId(unsigned char *pManID, unsigned char *pDevID)
{
    NandStatus_t retVal = NAND_STATUS_PASSED;
    /* Send the read ID command */
    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_READID);

    /* Write one cycle address with address as zero */
    NANDAddressWrite(EMIFNand.addrRegAddr, 0x00);
    NANDDelay(10);

    NANDWaitUntilReady();

    /* Read the 4-byte device ID */
	*pManID = NANDDataReadByte(EMIFNand.dataRegAddr);
    *pDevID = NANDDataReadByte(EMIFNand.dataRegAddr);
    
	

    if( (*pDevID ==0x00) || (*pDevID ==0xFF) )
    {
      retVal = NAND_STATUS_NOT_FOUND;
    }

   // NANDDataReadByte(EMIFNand.dataRegAddr);
   // devId = NANDDataReadByte (EMIFNand.dataRegAddr);

    /* Only try to detect device info from 4th ID byte if no valid  */
    /* values were given at initialization                          */
    if ( (EMIFNand.pageSize == NAND_PAGESIZE_INVALID) ||
         (EMIFNand.blkSize == NAND_BLOCKSIZE_INVALID) )
    {
        EMIFNand.pageSize      =  NAND_PAGESIZE_512BYTES;
        EMIFNand.blkSize       = NAND_BLOCKSIZE_16KB;
    }
    /* Only try to detect bus width info from 4th ID byte if no valid  */
    /* values were given at initialization                             */    
    if (EMIFNand.busWidth == NAND_BUSWIDTH_INVALID)
    {
        EMIFNand.busWidth = NAND_BUSWIDTH_8BIT;
    }
    
    /* Calculate the pagesPerBlock */
    EMIFNand.pagesPerBlk   = (EMIFNand.blkSize/EMIFNand.pageSize);
    
    return retVal;
}

/**
* \brief  Function to Erases The block.\n
*
*         This function erases the block specified as argument.\n
*
* \param  pNandInfo      :  Pointer to structure which conatins controller and 
*                          device information.\n
*
* \param  blkNum        :  Blk Number.\n
*
* \return Erase command status.\n
*
*         NAND_STATUS_PASSED         : If the erase command is passed.\n
*         NAND_STATUS_FAILED         : If the erase command is failed.\n
*         NAND_STATUS_DEVBUSY        : If the device is busy.\n
*         NAND_STATUS_DEVWRPROTECT   : If the device is write protect.\n
*         NAND_STATUS_WAITTIMEOUT    : If the RB pin inidcating device
*                                      busy status.\n
*
**/
NandStatus_t NANDBlockErase(unsigned int blkNum)
{
    NandStatus_t retVal;
    unsigned int addr;


    addr = PACK_ADDR(0x0, 0x0, blkNum);
    EMIF_NANDCSSet(EMIFNand.hNandCtrlInfo.currChipSelect);
    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_ERASE);

    /* Write 4 bytes of column addr */
    NANDAddressWrite(EMIFNand.addrRegAddr,
                     (unsigned char)((addr >>  0u) & 0xFF));// A0-A7  1st Cycle;  column addr
    NANDAddressWrite(EMIFNand.addrRegAddr,
                     (unsigned char)((addr >>  9u) & 0xFF));// A9-A16 2nd Cycle;  page   addr & blk
    NANDAddressWrite(EMIFNand.addrRegAddr,
                       (unsigned char)((addr >> 17u) & 0xFF)); // A17-A24 3rd Cycle; Block addr
    NANDAddressWrite(EMIFNand.addrRegAddr,
                       (unsigned char)((addr >> 25u) & 0x1));// A25-A26  4th Cycle;  Plane addr

    NANDDelay(1000);

    NANDCommandWrite(EMIFNand.cmdRegAddr, NAND_CMD_ERASE_CYC2);

    retVal = NANDWaitUntilReady();
    if(retVal != NAND_STATUS_WAITTIMEOUT)
    {
        retVal = NANDDeviceStatusGet();
    }

    return (retVal);
}

/**
* \brief  Function to write a page to NAND.\n
*
* \param  pNandInfo        Pointer to structure which conatins controller and 
*                         device information.\n
*
* \param  blkNum        :  Blk Number.\n
*
* \param  pageNum       :  Page Number.\n
*
* \param  data          :  Data to write.\n
*
* \return Write command status.\n
*
*         NAND_STATUS_PASSED              : If the write command is passed.\n
*         NAND_STATUS_FAILED              : If the write command is failed.\n
*         NAND_STATUS_DEVBUSY             : If the device is busy.\n
*         NAND_STATUS_DEVWRPROTECT        : If the device is write protect.\n
*         NAND_STATUS_WAITTIMEOUT         : If the RB pin inidcating device
*                                           busy status.\n
*         NAND_STATUS_READWRITE_DMA_FAIL  : If DMA transfer is failed.\n
*
**/
NandStatus_t NANDPageWrite(unsigned int blkNum,
                           unsigned int pageNum, volatile unsigned char *txData)
{
    NandStatus_t retVal;
    unsigned int trnsCnt;
    unsigned int columnAddr;
    unsigned char *eccDataTmp;
	unsigned char Ecc[16] = { 0 };
	unsigned char* pEccData = Ecc;
    
    columnAddr = 0;
    eccDataTmp = pEccData;
    EMIF_NANDCSSet(EMIFNand.hNandCtrlInfo.currChipSelect);
    NANDPageWriteCmdStart(blkNum, pageNum, columnAddr);
    retVal = NAND_STATUS_PASSED;
    
    for(trnsCnt = 0; trnsCnt < ((EMIFNand.pageSize) / NAND_BYTES_PER_TRNFS);
        trnsCnt++)
    {
        if (NAND_ECC_ALGO_NONE != EMIFNand.eccType)
        {    
            (EMIFNand.hNandEccInfo.ECCEnable)();
        }
        NANDDataWrite(txData, NAND_BYTES_PER_TRNFS);
        if (NAND_ECC_ALGO_NONE != EMIFNand.eccType)
        {    
            (EMIFNand.hNandEccInfo.ECCCalculate)(pEccData);
            pEccData += EMIFNand.hNandEccInfo.eccByteCnt;        
        }
        txData += NAND_BYTES_PER_TRNFS;
    }
    if(retVal == NAND_STATUS_PASSED)
    {
        retVal = NANDPageWriteCmdEnd();
        
        if (NAND_ECC_ALGO_NONE != EMIFNand.eccType)
        {    
            pEccData = eccDataTmp;
            if(retVal == NAND_STATUS_PASSED)
            {
                columnAddr = EMIFNand.hNandEccInfo.eccOffSet;
                if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
                {
                    columnAddr = columnAddr/2;
                }

                NANDWriteSpare(blkNum, pageNum, columnAddr, pEccData, (EMIFNand.hNandEccInfo.eccByteCnt)*((EMIFNand.pageSize)/NAND_BYTES_PER_TRNFS));
            }
        }
    }

#if 0
    unsigned char buf[128]={0};
    unsigned char* pData=buf;
    NANDReadSpare(blkNum, pageNum, columnAddr, pData, (EMIFNand.hNandEccInfo.eccByteCnt)*((EMIFNand.pageSize)/NAND_BYTES_PER_TRNFS));
#endif
    NANDWaitUntilReady();
    retVal = NANDDeviceStatusGet();
    return (retVal);
}

/**
* \brief  Function to read a page from NAND.\n
*
* \param  pNandInfo      : Pointer to structure which conatins controller and 
*                         device information.\n
*
* \param  blkNum        : Blk Number.\n
*
* \param  pageNum       : Blk Number.\n
*
* \param  data          : Pointer to Data where read data has to place.\n
*
* \return Read command status.\n
*
*    NAND_STATUS_PASSED                       : Page read is succssfull without any 
*                                               ECC error.\n
*    NAND_STATUS_READ_ECC_ERROR_CORRECTED     : Page read is sucssfull with ECC errors
*                                               and are corrected.\n
*    NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR : Uncurrectable ECC errors occured during
*                                               page read.\n
*    NAND_STATUS_WAITTIMEOUT                  : Device is busy.\n
*
*    NAND_STATUS_READWRITE_DMA_FAIL           : If DMA transfer is failed.\n
*
**/
NandStatus_t NANDPageRead(unsigned int blkNum,
                          unsigned int pageNum, volatile unsigned char *rxData)
{
    NandStatus_t retVal;
    unsigned int trnsCnt;
    unsigned int eccCorFlag;
    unsigned int columnAddr;
	unsigned char Ecc[16] = { 0 };
	unsigned char* pEccData = Ecc;
    NandEccInfo_t *nandEccInfo = &EMIFNand.hNandEccInfo;

    EMIF_NANDCSSet(EMIFNand.hNandCtrlInfo.currChipSelect);
    retVal = NAND_STATUS_PASSED;
    eccCorFlag = 0;

    columnAddr = nandEccInfo->eccOffSet;
    if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
    {
        columnAddr = columnAddr/2;
    }
#if 1
    NANDReadSpare(blkNum,pageNum,columnAddr,pEccData, (nandEccInfo->eccByteCnt)*((EMIFNand.pageSize)/NAND_BYTES_PER_TRNFS));
#endif
    /* Set the column addr to start of the page */
    columnAddr = 0;

    retVal = NANDPageReadCmdSend(blkNum, pageNum, columnAddr);
    if(retVal == NAND_STATUS_WAITTIMEOUT)
    {
        return (retVal);
    }

    for(trnsCnt = 0; trnsCnt < ((EMIFNand.pageSize)/ NAND_BYTES_PER_TRNFS) ; trnsCnt++)
    {
        if (NAND_ECC_ALGO_NONE != EMIFNand.eccType)
        {
            (EMIFNand.hNandEccInfo.ECCEnable)();
        }
        /* Read the sub-page from the data area. */

        NANDDataRead(rxData, NAND_BYTES_PER_TRNFS);
                
        if (NAND_ECC_ALGO_NONE != EMIFNand.eccType)
        {        
            if( EMIFNand.eccType == NAND_ECC_ALGO_BCH_8BIT )
            {
                /* Read the ECC data, as BCH algo reguires this for syndrome 
                 *  calculation.
                 */
                columnAddr = (nandEccInfo->eccOffSet) + (trnsCnt * nandEccInfo->eccByteCnt);
                if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
                {
                    columnAddr = columnAddr/2;
                }
                retVal = NANDPageReadCmdSend(blkNum, pageNum, columnAddr);
                if(retVal == NAND_STATUS_WAITTIMEOUT)
                {
                    return (retVal);
                }
                /* Read the ECC Data from spare area */
                NANDDataRead(pEccData, (nandEccInfo->eccByteCnt - 1));
            }        
           		
            /* Check for ECC errors and correct if any errors */
            retVal = (EMIFNand.hNandEccInfo.ECCCheckAndCorrect)((unsigned char*)rxData,pEccData);
            if( (retVal & NAND_STATUS_READ_ECC_ERROR_CORRECTED) )
            {
                eccCorFlag = 1;
            }
            else if( (retVal & NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR) )
            {
                break;
            }
            if( EMIFNand.eccType == NAND_ECC_ALGO_BCH_8BIT )
            {
                /* Reset the colomn pointer to appropariate position.  */

                columnAddr = ((trnsCnt + 1) * NAND_BYTES_PER_TRNFS);
                if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
                {
                    columnAddr = columnAddr/2;
                }
                NANDCommandWrite(EMIFNand.cmdRegAddr,
                                 NAND_CMD_READ_RANDOM);
                /* Write 2 bytes of column addr */
                NANDAddressWrite(EMIFNand.addrRegAddr,
                                (unsigned char)(columnAddr & 0xFF));
                NANDAddressWrite(EMIFNand.addrRegAddr,
                                 (unsigned char)((columnAddr >> 8) & 0xFF));
                NANDCommandWrite(EMIFNand.cmdRegAddr,
                                 NAND_CMD_READ_RANDOM_CYC2);
                retVal = NANDWaitUntilReady();
                if(retVal == NAND_STATUS_WAITTIMEOUT)
                {
                    return (retVal);
                }
            }
            pEccData += nandEccInfo->eccByteCnt;
        }
        //rxData += NAND_BYTES_PER_TRNFS;
    }
    if( eccCorFlag && (!(retVal & NAND_STATUS_READ_ECC_UNCORRECTABLE_ERROR)) )
    {
        retVal = NAND_STATUS_READ_ECC_ERROR_CORRECTED;
    }
    return (retVal);
}

/**
* \brief  This function Check whether Block is bad or Not.\n
*
* \param  pNandInfo      : Pointer to structure which conatins controller and 
*                         device information.\n
*
* \param  blkNum        : Blk Number.\n
*
* \return Bad block status.\n
*
*       NAND_BLOCK_BAD                    - If Block is bad.\n
*       NAND_BLOCK_GOOD                   - If Block is Good.\n
*       NAND_BLOCK_SPARE_AREA_READ_FAILED - Error while reading the spare area.\n
*
**/
NandBlockStatus_t NANDBadBlockCheck(unsigned int blkNum)
{
    NandBlockStatus_t retVal = NAND_BLOCK_GOOD;

    volatile unsigned char badBlkMark[16];
    
    EMIF_NANDCSSet(EMIFNand.hNandCtrlInfo.currChipSelect);
    NANDReadSpareCmdSend(blkNum,0,EMIFNand.pageSize);

    NANDDataRead(badBlkMark, 16);

    if(badBlkMark[5] != NAND_BLK_GOOD_MARK)
   {
       retVal = NAND_BLOCK_BAD;
   }

    return (retVal);
}

/**
* \brief  This function Marks the block as BAD by writing non 0xFF value in to
*         the spare area.\n
*
* \param  pNandInfo      : Pointer to structure which conatins controller and 
*                         device information.\n
*
* \param  blkNum        : Blk Number.\n
*
*
* \return
*
*         NAND_STATUS_PASSED        : If the write command is passed.\n
*         NAND_STATUS_FAILED        : If the write command is failed.\n
*         NAND_STATUS_DEVBUSY       : If the device is busy.\n
*         NAND_STATUS_DEVWRPROTECT  : If the device is write protect.\n
*         NAND_STATUS_WAITTIMEOUT   : If the RB pin inidcating device 
*                                     busy status.\n
*
**/
NandStatus_t NANDMarkBlockAsBad(unsigned int blkNum)
{
    unsigned int pageNum;
    unsigned int columnAddr;
    NandStatus_t retVal = NAND_STATUS_PASSED;

    columnAddr  = EMIFNand.pageSize;

    EMIF_NANDCSSet(EMIFNand.hNandCtrlInfo.currChipSelect);
    /* Adjust column address for 16-bit devices */    
    if(EMIFNand.busWidth == NAND_BUSWIDTH_16BIT)
    {
        columnAddr = columnAddr >> 1;
    }

    /* First erase the block before marking it as bad. The result of *
     * marking this as bad can be ignored.                           */
    NANDBlockErase(blkNum);

    /* Read the spare area of 1st, 2nd and last page of the block */
    for (pageNum = 0; pageNum < 3u; pageNum++)
    {
        unsigned char badBlkMark[2];
    
        /* Last page number of the block */
        if (pageNum == 2)
        {
            pageNum = (EMIFNand.pagesPerBlk - 1u);
        }
        badBlkMark[0] = NAND_BLK_BAD_MARK;
        NANDPageWriteCmdStart( blkNum, pageNum, columnAddr);
        NANDDataWrite(badBlkMark, 2);
        retVal = NANDPageWriteCmdEnd();
        if (retVal != NAND_STATUS_PASSED)
        {
            break;
        }
    }
    return (retVal);
}



/******************************************************************************
**                              END OF FILE
*******************************************************************************/

