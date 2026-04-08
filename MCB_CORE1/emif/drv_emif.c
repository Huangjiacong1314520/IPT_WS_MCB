/**
 *  \file   emifa.c
 *
 *  \brief  EMIFA  APIs.
 *
 *   This file contains the device abstraction layer APIs for EMIFA.
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

#include "hw_emifa2.h"
#include "hw_types.h"
#include "../drv_emif.h"

volatile unsigned int baseAddr = (0x20C00000);

/*******************************************************************************
*                       INTERNAL API DEFINITIONS
*******************************************************************************/
/**
* \brief   This function reads the module ID of EMIFA.\n
*
* \param   baseAddr  Memory address of EMIFA.\n
* 
* \return  Module Id of EMIFA.\n
*/

unsigned int  EMIF_ModuleIdRead()
{
    unsigned int modId;
    
    modId = HWREG(baseAddr + EMIFA_MIDR);
    
    return (modId);
}

/**
* \brief  This function sets the chip select for NAND.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  csNum      Chip select for which nand is interfaced.
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  Using nand on CS2.
*                    EMIFA_CHIP_SELECT_3  -  Using nand on CS3.
*                    EMIFA_CHIP_SELECT_4  -  Using nand on CS4.
*                    EMIFA_CHIP_SELECT_5  -  Using nand on CS5.\n
*
* \return none.
*/

void EMIF_NANDCSSet(unsigned int CSNum)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
             HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS2NAND;
        break;
        case EMIFA_CHIP_SELECT_1:
             HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS3NAND;
        break;
        case EMIFA_CHIP_SELECT_2:
             HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS4NAND;
        break;
        case EMIFA_CHIP_SELECT_3:
             HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS5NAND;
        break;
        default:
        break;
    }
}

/**
* \brief  This function start the NAND 4-bit ECC address 
*         and error value calculation.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \return none.
*/

void EMIF_NAND4BitECCAddrCalcStart()
{
    HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_4BITECC_ADD_CALC_START;
}

/**
* \brief  This function  start the 1-Bit and 4-Bit ECC calculation.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  eccType    ECC Type.
*                    This can take one of the following values:
*                    EMIFA_NAND_1BIT_ECC - To start 1 bit ECC.
*                    EMIFA_NAND_4BIT_ECC - To start 4 bit ecc
*
* \param  csNum      This parameter is only valid when ecc value is 
*                    EMIFA_NAND_1BIT_ECC. This specifies the chip 
*                    select for which nand is interfaced.
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  Using nand on CS2.
*                    EMIFA_CHIP_SELECT_3  -  Using nand on CS3.
*                    EMIFA_CHIP_SELECT_4  -  Using nand on CS4.
*                    EMIFA_CHIP_SELECT_5  -  Using nand on CS5.\n

* \return none.
*/

void EMIF_NANDECCStart(unsigned int eccType,
                       unsigned int CSNum)
{
    if(eccType == EMIFA_NAND_1BIT_ECC)
    {
	    switch(CSNum)
	    {
	        case EMIFA_CHIP_SELECT_0:
            	 HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS2ECC;
	        break;
	        case EMIFA_CHIP_SELECT_1:
   		         HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS3ECC;
	        break;
	        case EMIFA_CHIP_SELECT_2:
	  	         HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS4ECC;
	        break;
	        case EMIFA_CHIP_SELECT_3:
	             HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_CS5ECC;
	        break;
	        default:
	        break;
	    }
    }
    else if (eccType == EMIFA_NAND_4BIT_ECC)
    {
        HWREG(baseAddr + EMIFA_NANDFCR) |= EMIFA_NANDFCR_4BITECC_START;
    }
} 

/**
* \brief  This function selects the NAND for 4-bit ECC.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  csNum      Chip select for which nand is interfaced.
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  Using nand on CS2.
*                    EMIFA_CHIP_SELECT_3  -  Using nand on CS3.
*                    EMIFA_CHIP_SELECT_4  -  Using nand on CS4.
*                    EMIFA_CHIP_SELECT_5  -  Using nand on CS5.\n
*
* \return none.
*/

void EMIF_NAND4BitECCSelect(unsigned int CSNum)
{
    HWREG(baseAddr + EMIFA_NANDFCR) &= ~EMIFA_NANDFCR_4BITECCSEL;   
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
 	         HWREG(baseAddr + EMIFA_NANDFCR) |= (EMIFA_NANDFCR_4BITECCSEL_CS2ECC
 											 << EMIFA_NANDFCR_4BITECCSEL_SHIFT);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_NANDFCR) |= (EMIFA_NANDFCR_4BITECCSEL_CS3ECC
                                             << EMIFA_NANDFCR_4BITECCSEL_SHIFT);
        break;
        case EMIFA_CHIP_SELECT_2:
 	         HWREG(baseAddr + EMIFA_NANDFCR) |= (EMIFA_NANDFCR_4BITECCSEL_CS4ECC
 										     << EMIFA_NANDFCR_4BITECCSEL_SHIFT);
        break;
        case EMIFA_CHIP_SELECT_3:
    	     HWREG(baseAddr + EMIFA_NANDFCR) |= (EMIFA_NANDFCR_4BITECCSEL_CS5ECC
											 << EMIFA_NANDFCR_4BITECCSEL_SHIFT);
        break;
        default:
        break;
    }
}

/**
* \brief  This function sets the maximum extended waitcycles.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  waitVal    Maximum extended wait cycles.The EMIFA will wait for 
*                    a maximum of (waitVal + 1)*16  clock cycles before it 
*                    stops inserting asynchronous wait cycles and proceeds 
*                    to the hold period of the access.\n
*
* \return none.
*/

void EMIFAMaxExtWaitCycleSet(unsigned int waitVal)
{
    HWREG(baseAddr + EMIFA_AWCC) &= ~EMIFA_AWCC_MAX_EXT_WAIT;   
    HWREG(baseAddr + EMIFA_AWCC) |=(waitVal & EMIFA_AWCC_MAX_EXT_WAIT); 
}

/**
* \brief  This function selects the waitpin for specified chip select.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  csNum      Chip Select for which wait pin has to select.
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  Wait Pin Selection For CS2.
*                    EMIFA_CHIP_SELECT_3  -  Wait Pin Selection For CS3.
*                    EMIFA_CHIP_SELECT_4  -  Wait Pin Selection For CS4.
*                    EMIFA_CHIP_SELECT_5  -  Wait Pin Selection For CS5.\n
*
* \param  pin        Pin number to be used to control external wait states.
*                    This can take one of the following value.
*                    EMIFA_EMA_WAIT_PIN0  -  To select EMA_WAIT[0] pin.\n
*                    EMIFA_EMA_WAIT_PIN1  -  To select EMA_WAIT[1] pin.\n
* \return none.
*/
void EMIFACSWaitPinSelect(unsigned int CSNum, 
                          unsigned int pin)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
 	         HWREG(baseAddr + EMIFA_AWCC) &= ~EMIFA_AWCC_CS2_WAIT;
     	     HWREG(baseAddr + EMIFA_AWCC) |= ((pin << EMIFA_AWCC_CS2_WAIT_SHIFT)
	                                           &EMIFA_AWCC_CS2_WAIT);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_AWCC) &= ~EMIFA_AWCC_CS3_WAIT;
	         HWREG(baseAddr + EMIFA_AWCC) |= ((pin << EMIFA_AWCC_CS3_WAIT_SHIFT)
     		                                   &EMIFA_AWCC_CS3_WAIT);      
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_AWCC) &= ~EMIFA_AWCC_CS4_WAIT;
	         HWREG(baseAddr + EMIFA_AWCC) |= ((pin << EMIFA_AWCC_CS4_WAIT_SHIFT)
	                                           &EMIFA_AWCC_CS4_WAIT);      
        break;
        case EMIFA_CHIP_SELECT_3:
      	     HWREG(baseAddr + EMIFA_AWCC) &= ~EMIFA_AWCC_CS5_WAIT;
	         HWREG(baseAddr + EMIFA_AWCC) |= ((pin << EMIFA_AWCC_CS5_WAIT_SHIFT)
	                                           &EMIFA_AWCC_CS5_WAIT);      
        break;
        default:
        break;
    }
}

/**
* \brief  This function set the polarity of EMA_WAIT[n] pin.\n
*
* \param  baseAddr    Memory address of EMIFA.\n
*
* \param  pin         Pin number to be used to control external wait states.
*                     This can take one of the following values :
*                     EMIFA_EMA_WAIT_PIN0  -  For EMA_WAIT[0] pin.
*                     EMIFA_EMA_WAIT_PIN1  -  For EMA_WAIT[1] pin.\n
*
* \param  pinPolarity Polarity of the pin.\n
*                     This can take one of the following values :
*                     EMIFA_EMA_WAIT_PIN_POLARITY_LOW  -   To select polarity low.
*                     EMIFA_EMA_WAIT_PIN_POLARITY_HIGH  -  To select polarity high.\n
*
* \return none.
*/
void EMIFAWaitPinPolaritySet(unsigned int pin, 
                             unsigned int pinPolarity)
{
    if(pin == EMIFA_EMA_WAIT_PIN0)
    {   
    	if(pinPolarity == 0)
    	{
    		HWREG(baseAddr + EMIFA_AWCC) &= ~(1 << EMIFA_AWCC_WP0_SHIFT);
    	}
    	else
    	{
    		HWREG(baseAddr + EMIFA_AWCC) |= (1 << EMIFA_AWCC_WP0_SHIFT);
    	}
    }
    else if(pin == EMIFA_EMA_WAIT_PIN1)
    {
    	if(pinPolarity == 0)
    	{
    		HWREG(baseAddr + EMIFA_AWCC) &= ~(1 << EMIFA_AWCC_WP1_SHIFT);
    	}
    	else
    	{
    		HWREG(baseAddr + EMIFA_AWCC) |= (1 << EMIFA_AWCC_WP1_SHIFT);
    	}
    }       
}

/**
* \brief  This function will set the buswidth of async device connected.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  CSNum      Chip Select where device has interfaced.
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  Device Interfaced on CS2.
*                    EMIFA_CHIP_SELECT_3  -  Device Interfaced on CS3.
*                    EMIFA_CHIP_SELECT_4  -  Device Interfaced on CS4.
*                    EMIFA_CHIP_SELECT_5  -  Device Interfaced on CS5.\n
*
* \param  width      Bus width of CSNum.\n
*                    This can take one of the following values :
*                    EMIFA_DATA_BUSWITTH_8BIT    -  8-bit data bus.
*                    EMIFA_DATA_BUSWITTH_16BBIT  -  16-bit data bus.\n
*
* \return none.
*/
void EMIFAAsyncDevDataBusWidthSelect(unsigned int CSNum,
                                     unsigned int width)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
        	 HWREG(baseAddr + EMIFA_CE2CFG) &= 0xFFFFFFFC;
        	 HWREG(baseAddr + EMIFA_CE2CFG) |= (width & EMIFA_CE2CFG_ASIZE);
        break;
        case EMIFA_CHIP_SELECT_1:
        	 HWREG(baseAddr + EMIFA_CE3CFG) &= 0xFFFFFFFC;
	         HWREG(baseAddr + EMIFA_CE3CFG) |= (width & EMIFA_CE3CFG_ASIZE);
        break;
        case EMIFA_CHIP_SELECT_2:
         	 HWREG(baseAddr + EMIFA_CE4CFG) &= 0xFFFFFFFC;
	         HWREG(baseAddr + EMIFA_CE4CFG) |= (width & EMIFA_CE4CFG_ASIZE);
        break;
        case EMIFA_CHIP_SELECT_3:
        	 HWREG(baseAddr + EMIFA_CE5CFG) &= 0xFFFFFFFC;
	         HWREG(baseAddr + EMIFA_CE5CFG) |= (width & EMIFA_CE5CFG_ASIZE);
        break;
        default:
        break;
    }
}

/**
* \brief   This function selects the aync interface opmode.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param   CSNum      Chip Select where device has interfaced.
*                     This can take one of the following values :
*                     EMIFA_CHIP_SELECT_2  -  Device Interfaced on CS2.
*                     EMIFA_CHIP_SELECT_3  -  Device Interfaced on CS3.
*                     EMIFA_CHIP_SELECT_4  -  Device Interfaced on CS4.
*                     EMIFA_CHIP_SELECT_5  -  Device Interfaced on CS5.\n
*
* \param  mode        Opmode of CSNum.\n
*                     This can take one of the following values :
*                     EMIFA_ASYNC_INTERFACE_NORMAL_MODE    -  Normal Mode.
*                     EMIFA_ASYNC_INTERFACE_STROBE_MODE    -  Strobe Mode.\n
* \return none.
*/
void EMIFAAsyncDevOpModeSelect(unsigned int CSNum,
                               unsigned int mode)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
        	 HWREG(baseAddr + EMIFA_CE2CFG) |= ((mode << EMIFA_CE2CFG_SS_SHIFT)
	                                            &EMIFA_CE2CFG_SS);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_CE3CFG) |= ((mode << EMIFA_CE3CFG_SS_SHIFT)
     		                                     &EMIFA_CE3CFG_SS);     
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_CE4CFG) |= ((mode << EMIFA_CE4CFG_SS_SHIFT)
     	                                        &EMIFA_CE4CFG_SS);     
        break;
        case EMIFA_CHIP_SELECT_3:
	         HWREG(baseAddr + EMIFA_CE5CFG) |= ((mode << EMIFA_CE5CFG_SS_SHIFT)
     	                                        &EMIFA_CE5CFG_SS);     
        break;
        default:
        break;
    }
}

/**
* \brief  This function configures the extended wait cycles to the device.\n
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param   CSNum      Chip Select where device has interfaced.
*                     This can take one of the following values :
*                     EMIFA_CHIP_SELECT_2  -  Device Interfaced on CS2.
*                     EMIFA_CHIP_SELECT_3  -  Device Interfaced on CS3.
*                     EMIFA_CHIP_SELECT_4  -  Device Interfaced on CS4.
*                     EMIFA_CHIP_SELECT_5  -  Device Interfaced on CS5.\n
*
*
* \param  flag        Flag indicates whether to enable or not.
*                     This can take one of the following values :
*                     EMIFA_EXTENDED_WAIT_ENABLE    -  Extended Wait enable.
*                     EMIFA_EXTENDED_WAIT_DISABLE   -  Extended Wait disable.\n
*
* \return none.
*/
void EMIFAExtendedWaitConfig(unsigned int CSNum,
                             unsigned int flag)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
	         HWREG(baseAddr + EMIFA_CE2CFG) |= ((flag << EMIFA_CE2CFG_EW_SHIFT)
     	                                        &EMIFA_CE2CFG_EW);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_CE3CFG) |= ((flag << EMIFA_CE3CFG_EW_SHIFT)
     		                                    &EMIFA_CE3CFG_EW);      
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_CE4CFG) |= ((flag << EMIFA_CE4CFG_EW_SHIFT)
     		                                    &EMIFA_CE4CFG_EW);      
        break;
        case EMIFA_CHIP_SELECT_3:
	         HWREG(baseAddr + EMIFA_CE5CFG) |= ((flag << EMIFA_CE5CFG_EW_SHIFT)
     	 	                                    &EMIFA_CE5CFG_EW);      
        break;
        default:
        break;
    }
}

/**
* \brief  This function returns the status of EMA_WAIT[n] pins.\n 
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  pin        EMA_WAIT[n] pin number.
*                    This can take one of the following values :
*                    EMIFA_EMA_WAIT_PIN0  -  For EMA_WAIT[0] pin.
*                    EMIFA_EMA_WAIT_PIN1  -  For EMA_WAIT[1] pin.\n
*
* \return Pin Status 0 : If pin status is low.
*                    1 : If pin status is high.
*/
unsigned int EMIF_WaitPinStatusGet(unsigned int pinNum)
{
    unsigned int status = 0;

    if( pinNum  == EMIFA_EMA_WAIT_PIN0)
    {   
        status = (HWREG(baseAddr + EMIFA_NANDFSR) & 0x1);
        
    }
    else if(pinNum == EMIFA_EMA_WAIT_PIN1)
    {
        status = ((HWREG(baseAddr + EMIFA_NANDFSR) >> 1) & 0x1);
    }       
    return status;   
}

/**
* \brief  This function returns ECC correction state while performing 4-bit ECC
*         Address and Error Value Calculation.\n 
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \return eccState   ECC correction state (ECC_STATE).
*/
unsigned int EMIF_NAND4BitECCStateGet()
{
    unsigned int eccState;
    
    eccState = ((HWREG(baseAddr + EMIFA_NANDFSR) & EMIFA_NANDFSR_ECC_STATE) >> 
                EMIFA_NANDFSR_ECC_STATE_SHIFT); 
                
    return eccState;                
}

/**
* \brief  This function returns the Number of Errors found after the 4-Bit ECC 
*         Error Address and Error Value Calculation.
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \return eccErrNum  Num of erros during 4-bit ECC add and err val 
*                    calculation(ECC_ERRNUM).
*/
unsigned int EMIF_NAND4BitECCNumOfErrsGet()
{
    unsigned int eccErrNum;
    
    eccErrNum = ((HWREG(baseAddr + EMIFA_NANDFSR) & EMIFA_NANDFSR_ECC_ERRNUM) >> 
                EMIFA_NANDFSR_ECC_ERRNUM_SHIFT);    
                
    return eccErrNum;   
    
}

/**
* \brief  This function configures the pagemode of NOR.
*
* \param  baseAddr   Memory address of EMIFA.\n
*
*         CSNum      Chip Select for which nor is interfaced .
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  NOR interfaced on CS2.
*                    EMIFA_CHIP_SELECT_3  -  NOR interfaced on CS3.
*                    EMIFA_CHIP_SELECT_4  -  NOR interfaced on CS4.
*                    EMIFA_CHIP_SELECT_5  -  NOR interfaced on CS5.\n
*
*         flag       Page mode to enable or disable.
*                    This can take one of the following values :        
*                    EMIFA_NOR_PAGEMODE_ENABLE  - Nor page mode enable
*                    EMIFA_NOR_PAGEMODE_DISABLE - Nor page mode disable.\n
* 
* \return none.
*/
void EMIFANORPageModeConfig( unsigned int CSNum, 
                                   unsigned int flag)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((flag << EMIFA_PMCR_CS2_PG_MD_EN_SHIFT)
     		                                  & EMIFA_PMCR_CS2_PG_MD_EN);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((flag << EMIFA_PMCR_CS3_PG_MD_EN_SHIFT)
     		                                  & EMIFA_PMCR_CS3_PG_MD_EN);
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((flag << EMIFA_PMCR_CS4_PG_MD_EN_SHIFT)
     	                                      & EMIFA_PMCR_CS4_PG_MD_EN);
        break;
        case EMIFA_CHIP_SELECT_3:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((flag << EMIFA_PMCR_CS5_PG_MD_EN_SHIFT)
     	                                      & EMIFA_PMCR_CS5_PG_MD_EN);
        break;
        default:
        break;
    }
}

/**
* \brief  This function sets the page size of NOR.
*
* \param  baseAddr   Memory address of EMIFA.\n
*
*         CSNum      Chip Select for which nand is interfaced .
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  NOR interfaced on CS2.
*                    EMIFA_CHIP_SELECT_3  -  NOR interfaced on CS3.
*                    EMIFA_CHIP_SELECT_4  -  NOR interfaced on CS4.
*                    EMIFA_CHIP_SELECT_5  -  NOR interfaced on CS5.
*
*        pageSize    Page size of NOR.
*                    This can take one of the following values :
*                    EMIFA_NOR_PAGE_SIZE_4WORDS  - Nor page size is 4 words
*                    EMIFA_NOR_PAGE_SIZE_8WORDS  - Nor page size is 8 words
* 
* \return none.
*/
void EMIFANORPageSizeSet(unsigned int CSNum,
                                unsigned int pageSize)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
        	 HWREG(baseAddr + EMIFA_PMCR) |= ((pageSize << 
											  EMIFA_PMCR_CS2_PG_SIZE_SHIFT)
                                              & EMIFA_PMCR_CS2_PG_SIZE);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((pageSize << 
							  	     		  EMIFA_PMCR_CS3_PG_SIZE_SHIFT)
                                              & EMIFA_PMCR_CS3_PG_SIZE);
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((pageSize << 
											  EMIFA_PMCR_CS4_PG_SIZE_SHIFT)
		                                      & EMIFA_PMCR_CS4_PG_SIZE);
        break;
        case EMIFA_CHIP_SELECT_3:
		     HWREG(baseAddr + EMIFA_PMCR) |= ((pageSize << 
								  			  EMIFA_PMCR_CS5_PG_SIZE_SHIFT)
	                                          & EMIFA_PMCR_CS5_PG_SIZE);
        break;
        default:
        break;
    }
}

/**
* \brief  This function sets the page access delay for NOR.
*
* \param  baseAddr   Memory address of EMIFA.\n
*
*         CSNum      Chip Select for which nor interfaced .\n
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  NOR interfaced on CS2.
*                    EMIFA_CHIP_SELECT_3  -  NOR interfaced on CS3.
*                    EMIFA_CHIP_SELECT_4  -  NOR interfaced on CS4.
*                    EMIFA_CHIP_SELECT_5  -  NOR interfaced on CS5.\n
*
*        delay      Page access delay for NOR Flash in EMA_CLK cycles.\n
*        
* 
* \return none.\n
*/
void EMIFANORPageAccessDelaySet( unsigned int CSNum,
                                unsigned int delay)
{
    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
        	 HWREG(baseAddr + EMIFA_PMCR) |= ((delay << 
                                              EMIFA_PMCR_CS2_PG_DEL_SHIFT) 
             			                      & EMIFA_PMCR_CS2_PG_DEL);
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((delay << 
											  EMIFA_PMCR_CS3_PG_DEL_SHIFT) 
		                                      & EMIFA_PMCR_CS3_PG_DEL);
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((delay <<
											   EMIFA_PMCR_CS4_PG_DEL_SHIFT) 
			                                   & EMIFA_PMCR_CS4_PG_DEL);
        break;
        case EMIFA_CHIP_SELECT_3:
	         HWREG(baseAddr + EMIFA_PMCR) |= ((delay << 
					  						  EMIFA_PMCR_CS5_PG_DEL_SHIFT) 
			                                  & EMIFA_PMCR_CS5_PG_DEL);
        break;
        default:
        break;
    }
}

/**
* \brief  This function loads the 4-BIT ECC value.
*
* \param  baseAddr   Memory address of EMIFA.\n
*
*         eccLdVal   ECC load value .\n  
* 
* \return none.\n
*/
void EMIF_NAND4BitECCLoad(unsigned int eccLdVal)
{

    HWREG(baseAddr + EMIFA_NAND4BITECCLOAD) =( eccLdVal & 
                                             EMIFA_NAND4BITECCLOAD_4BITECCLOAD);
}

/**
* \brief  This function retrives the ECC value.
*
* \param  baseAddr         Memory address of EMIFA.\n
*
* \param  eccType          ECC type.
*                          This can take one of the following values:
*                          EMIFA_NAND_1BIT_ECC - To start 1 bit ECC.
*                          EMIFA_NAND_4BIT_ECC - To start 4 bit ecc
*
*         eccValIndexOrCS  When ecc is EMIFA_NAND_1BIT_ECC, this argument 
*                          acts as CS(chip select), where it can take one of 
*                          the following values : 
*                          EMIFA_CHIP_SELECT_2  -  Using nand on CS2.
*                          EMIFA_CHIP_SELECT_3  -  Using nand on CS3.
*                          EMIFA_CHIP_SELECT_4  -  Using nand on CS4.
*                          EMIFA_CHIP_SELECT_5  -  Using nand on CS5.\n
*
*                          When ecc is EMIFA_NAND_4BIT_ECC, this argument 
*                          acts asa ECC value index, where it can take one of     
*                          following values :
*                          EMIFA_NAND_4BITECCVAL1 -- To retrive 4BITECCVAL1
*                          EMIFA_NAND_4BITECCVAL2 -- To retrive 4BITECCVAL2
*                          EMIFA_NAND_4BITECCVAL3 -- To retrive 4BITECCVAL3
*                          EMIFA_NAND_4BITECCVAL4 -- To retrive 4BITECCVAL4
*                          EMIFA_NAND_4BITECCVAL5 -- To retrive 4BITECCVAL5
*                          EMIFA_NAND_4BITECCVAL6 -- To retrive 4BITECCVAL6
*                          EMIFA_NAND_4BITECCVAL7 -- To retrive 4BITECCVAL7
*                          EMIFA_NAND_4BITECCVAL8 -- To retrive 4BITECCVAL8
*   
* \return eccVal           ECC value.
*/
unsigned int EMIF_NANDEccValGet(unsigned int eccType,
                                unsigned int eccValIndexOrCS)
{
    unsigned int eccVal = 0;

    if(eccType == EMIFA_NAND_1BIT_ECC)
    {
        switch(eccValIndexOrCS)
        {
            case EMIFA_CHIP_SELECT_0:
                 eccVal = (HWREG(baseAddr + EMIFA_NANDF1ECC));
            break;
            case EMIFA_CHIP_SELECT_1:
                  eccVal = (HWREG(baseAddr + EMIFA_NANDF2ECC));
            break;
            case EMIFA_CHIP_SELECT_2:
                 eccVal = (HWREG(baseAddr + EMIFA_NANDF3ECC));
            break;
            case EMIFA_CHIP_SELECT_3:
                 eccVal = (HWREG(baseAddr + EMIFA_NANDF4ECC));
            break;
           default:
           break;
        }
    }
    else if(eccType == EMIFA_NAND_4BIT_ECC)
    {
        switch(eccValIndexOrCS)
        {
            case EMIFA_NAND_4BITECCVAL1:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC1) &
                            EMIFA_NAND4BITECC1_4BITECCVAL1) >> 
                            EMIFA_NAND4BITECC1_4BITECCVAL1_SHIFT);
            break;
            case EMIFA_NAND_4BITECCVAL2:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC1) &
                            EMIFA_NAND4BITECC1_4BITECCVAL2) >> 
                            EMIFA_NAND4BITECC1_4BITECCVAL2_SHIFT);                
            break;
            case EMIFA_NAND_4BITECCVAL3:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC2) &
                            EMIFA_NAND4BITECC2_4BITECCVAL3) >> 
                            EMIFA_NAND4BITECC2_4BITECCVAL3_SHIFT);
            break;
            case EMIFA_NAND_4BITECCVAL4:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC2) &
                            EMIFA_NAND4BITECC2_4BITECCVAL4) >> 
                            EMIFA_NAND4BITECC2_4BITECCVAL4_SHIFT);        
            break;
            case EMIFA_NAND_4BITECCVAL5:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC3) &
                            EMIFA_NAND4BITECC3_4BITECCVAL5) >> 
                            EMIFA_NAND4BITECC3_4BITECCVAL5_SHIFT);        
            break;
            case EMIFA_NAND_4BITECCVAL6:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC3) &
                            EMIFA_NAND4BITECC3_4BITECCVAL6) >> 
                            EMIFA_NAND4BITECC3_4BITECCVAL6_SHIFT);        
            break;
            case EMIFA_NAND_4BITECCVAL7:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC4) &
                            EMIFA_NAND4BITECC4_4BITECCVAL7) >> 
                            EMIFA_NAND4BITECC4_4BITECCVAL7_SHIFT);        
            break;
            case EMIFA_NAND_4BITECCVAL8:
                 eccVal = ((HWREG(baseAddr + EMIFA_NAND4BITECC4) &
                            EMIFA_NAND4BITECC4_4BITECCVAL8) >> 
                            EMIFA_NAND4BITECC4_4BITECCVAL8_SHIFT);
            break;
            default:
            break;
        }
    }
    return eccVal;
}               

/**
* \brief  This function retrives the Address of 4-bit ECC error.
*
* \param  baseAddr         Memory address of EMIFA.\n
*
*         eccErrAddrIndex  ECC address error index.\n    
*                          This can take one of the following values : 
*                          EMIFA_4BITECC_ERRADDR_INDEX_1 -- To retrice 
*                                                           4BITECCERRADD1
*                          EMIFA_4BITECC_ERRADDR_INDEX_2 -- To retrice 
*                                                           4BITECCERRADD2
*                          EMIFA_4BITECC_ERRADDR_INDEX_3 -- To retrice 
*                                                           4BITECCERRADD3
*                          EMIFA_4BITECC_ERRADDR_INDEX_4 -- To retrice 
*                                                           4BITECCERRADD4
*   
* \return eccAddrVal   Address of 4-bit ECC error.
**/
unsigned int EMIF_NAND4BitEccErrAddrGet(
                                        unsigned int eccErrAddrIndex)
{
    unsigned int eccAddrVal = 0;   

    switch(eccErrAddrIndex)
    {
        case EMIFA_NAND_4BITECCVAL1:
             eccAddrVal = ((HWREG(baseAddr + EMIFA_NANDERRADD1) &
                            EMIFA_NANDERRADD1_4BITECCERRADD1) >>
                            EMIFA_NANDERRADD1_4BITECCERRADD1_SHIFT);
        break;
        case EMIFA_4BITECC_ERRADDR_INDEX_2:
             eccAddrVal = ((HWREG(baseAddr + EMIFA_NANDERRADD1) &
                           EMIFA_NANDERRADD1_4BITECCERRADD2) >>
                           EMIFA_NANDERRADD1_4BITECCERRADD2_SHIFT);  
        break;
        case EMIFA_4BITECC_ERRADDR_INDEX_3:
             eccAddrVal = ((HWREG(baseAddr + EMIFA_NANDERRADD2) &
                          EMIFA_NANDERRADD2_4BITECCERRADD3) >>
                          EMIFA_NANDERRADD2_4BITECCERRADD3_SHIFT);  
        break;
        case EMIFA_4BITECC_ERRADDR_INDEX_4:
             eccAddrVal = ((HWREG(baseAddr + EMIFA_NANDERRADD2) &
                          EMIFA_NANDERRADD2_4BITECCERRADD4) >>
                          EMIFA_NANDERRADD2_4BITECCERRADD4_SHIFT);  
        break;
        default:
        break;
    }
    return (eccAddrVal);
}

/**
* \brief  This function retrives the Value of 4-bit ECC error.
*
* \param  baseAddr        Memory address of EMIFA.\n
*
*         eccErrValIndex  ECC error value index.
*                    This can take one of the following values :
*                    EMIFA_4BITECC_ERRVAL_INDEX_1 -- To retrice 4BITECCERRVAL1
*                    EMIFA_4BITECC_ERRVAL_INDEX_2 -- To retrice 4BITECCERRVAL2
*                    EMIFA_4BITECC_ERRVAL_INDEX_3 -- To retrice 4BITECCERRVAL3
*                    EMIFA_4BITECC_ERRVAL_INDEX_4 -- To retrice 4BITECCERRVAL4
*   
* \return eccErrVal  Value of 4-bit ECC error.
*/
unsigned int EMIF_NAND4BitEccErrValGet(
                                        unsigned int eccErrValIndex)
{
    unsigned int eccErrVal = 0;    

    switch(eccErrValIndex)
    {
        case EMIFA_4BITECC_ERRVAL_INDEX_1:
	         eccErrVal = ((HWREG(baseAddr + EMIFA_NANDERRVAL1) &
      	                   EMIFA_NANDERRVAL1_4BITECCERRVAL1) >>
	                       EMIFA_NANDERRVAL1_4BITECCERRVAL1_SHIFT);
        break;
        case EMIFA_4BITECC_ERRVAL_INDEX_2:
	         eccErrVal = ((HWREG(baseAddr + EMIFA_NANDERRVAL1) &
     	                   EMIFA_NANDERRVAL1_4BITECCERRVAL2) >>
          		           EMIFA_NANDERRVAL1_4BITECCERRVAL2_SHIFT);  
        break;
        case EMIFA_4BITECC_ERRVAL_INDEX_3:
	         eccErrVal = ((HWREG(baseAddr + EMIFA_NANDERRVAL2) &
     	                   EMIFA_NANDERRVAL2_4BITECCERRVAL3) >>
	                       EMIFA_NANDERRVAL2_4BITECCERRVAL3_SHIFT);  
        break;
        case EMIFA_4BITECC_ERRVAL_INDEX_4:
	         eccErrVal = ((HWREG(baseAddr + EMIFA_NANDERRVAL2) &
     	                   EMIFA_NANDERRVAL2_4BITECCERRVAL4) >>
	                       EMIFA_NANDERRVAL2_4BITECCERRVAL4_SHIFT);  
        break;
        default:
        break;
    }    
 
    return (eccErrVal);
}

/**
* \brief  This function configures the wait timing for the device interfaced on 
*         csNum.
*
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  csNum      Chip Select for which wait timing has to configure.
*                    This can take one of the following values :
*                    EMIFA_CHIP_SELECT_2  -  For CS2.
*                    EMIFA_CHIP_SELECT_3  -  For CS3.
*                    EMIFA_CHIP_SELECT_4  -  For CS4.
*                    EMIFA_CHIP_SELECT_5  -  For CS5.\n
*
* \param  conf       Configuration value for the device connected to csNum.
*                    Use the EMIFA_ASYNC_WAITTIME_CONFIG macro to get the value 
*                    of conf. \n
*
* \return none.
**/                    
void EMIFAWaitTimingConfig(unsigned int CSNum,
                           unsigned int conf)
{

    switch(CSNum)
    {
        case EMIFA_CHIP_SELECT_0:
     	     HWREG(baseAddr + EMIFA_CE2CFG) |= conf;     
        break;
        case EMIFA_CHIP_SELECT_1:
	         HWREG(baseAddr + EMIFA_CE3CFG) |= conf;     
        break;
        case EMIFA_CHIP_SELECT_2:
	         HWREG(baseAddr + EMIFA_CE4CFG) |= conf;     
        break;
        case EMIFA_CHIP_SELECT_3:
	         HWREG(baseAddr + EMIFA_CE5CFG) |= conf;     
        break;
        default:
        break;
    }
}                          

/**
* \brief  This function monitors/reads the EMIFA's hardware-generated interrupts
*           
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  intFlag    Interrupt Flag for which interrupt status has to read.\n
*                    This can take one of the following values :
*                    EMIFA_ASYNC_TIMOUT_INT  -  For Asynchronous Timeout 
*                                               interrupt.
*                    EMIFA_LINE_TRAP_INT     -  For Line Trap interrupt.
*                    EMIFA_WAIT_RISE_INT     -  For Wait Rise interrupt.
*
* \return Interrupt status.
*/
unsigned int EMIFARawIntStatusRead(unsigned int intFlag)
{
    unsigned int intStatus = 0;
    if(intFlag == EMIFA_ASYNC_TIMOUT_INT)
    {
        intStatus = ((HWREG(baseAddr + EMIFA_INTRAW) & EMIFA_INTRAW_AT) >>
                    EMIFA_INTRAW_AT_SHIFT);
    }
    else if (intFlag == EMIFA_LINE_TRAP_INT)
    {
        intStatus = ((HWREG(baseAddr + EMIFA_INTRAW) & EMIFA_INTRAW_LT) >>
                    EMIFA_INTRAW_LT_SHIFT);     
    }
    else if (intFlag == EMIFA_WAIT_RISE_INT)
    {
        intStatus = ((HWREG(baseAddr + EMIFA_INTRAW) & EMIFA_INTRAW_WR) >>
                    EMIFA_INTRAW_WR_SHIFT);             
    }       
    return intStatus;
}

/**
* \brief  This function Clears the EMIFA hardware-generated interrupts
*           
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  intFlag    Interrupt Flag for which interrupt has to clear.\n
*                    This can take one of the following values :
*                    EMIFA_ASYNC_TIMOUT_INT  -  For Asynchronous Timeout
*                                                interrupt.
*                    EMIFA_LINE_TRAP_INT     -  For Line Trap interrupt.\n
*                    EMIFA_WAIT_RISE_INT     -  For Wait Rise interrupt.\n
*
* \return none.
*/
void EMIFARawIntClear(unsigned int intFlag)
{
    if(intFlag == EMIFA_ASYNC_TIMOUT_INT)
    {
        HWREG(baseAddr + EMIFA_INTRAW) |= EMIFA_INTRAW_AT;
    }
    else if (intFlag == EMIFA_LINE_TRAP_INT)
    {
        HWREG(baseAddr + EMIFA_INTRAW) |= EMIFA_INTRAW_LT;       
    }
    else if (intFlag == EMIFA_WAIT_RISE_INT)
    {
        HWREG(baseAddr + EMIFA_INTRAW) |= EMIFA_INTRAW_WR;               
    }       
}

/**
* \brief  This function monitors/reads the EMIFA's hardware-generated interrupts
*           
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  intFlag    Interrupt Flag for which interrupt status has to read.\n
*                    This can take one of the following values :
*                    EMIFA_ASYNC_TIMOUT_INT  -  For Asynchronous Timeout
*                                               interrupt.
*                    EMIFA_LINE_TRAP_INT     -  For Line Trap interrupt.
*                    EMIFA_WAIT_RISE_INT     -  For Wait Rise interrupt.\n
*
* \return Interrupt status.
*
*  NOTE : Main diffrence between EMIFAMskedIntStatusRead and 
*         EMIFARawIntStatusRead is that when any int flag in 
*         EMIFAMskedIntStatusRead is set an active-high pulse will be sent 
*         to the CPU interrupt controller.
*
*/
unsigned int EMIF_MskedIntStatusRead(unsigned int intFlag)
{
    unsigned int intStatus = 0;

    if(intFlag == EMIFA_ASYNC_TIMOUT_INT)
    {
       intStatus = ((HWREG(baseAddr + EMIFA_INTMSK) & EMIFA_INTMSK_ATED_MASK) >>
                     EMIFA_INTMSK_ATED_SHIFT);
    }
    else if (intFlag == EMIFA_LINE_TRAP_INT)
    {
       intStatus = ((HWREG(baseAddr + EMIFA_INTMSK) & EMIFA_INTMSK_LTED_MASK) >>
                     EMIFA_INTMSK_LTED_SHIFT);
    }
    else if (intFlag == EMIFA_WAIT_RISE_INT)
    {
       intStatus = ((HWREG(baseAddr + EMIFA_INTMSK) & EMIFA_INTMSK_WRED_MASK) >>
                     EMIFA_INTMSK_WRED_SHIFT);
    }       
    return intStatus;   
}

/**
* \brief  This function Clears the EMIFA抯 hardware-generated interrupts
*           
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  intFlag    Interrupt Flag for which interrupt has to clear.\n
*                    This can take one of the following values :
*                    EMIFA_ASYNC_TIMOUT_INT  -  For Asynchronous Timeout
*                                                interrupt.\n
*                    EMIFA_LINE_TRAP_INT     -  For Line Trap interrupt.\n
*                    EMIFA_WAIT_RISE_INT     -  For Wait Rise interrupt.\n
*
*  NOTE : Main diffrence between EMIFAMskedIntStatusRead and 
*         EMIFARawIntStatusRead is that when any int flag in 
*         EMIFAMskedIntStatusRead is set an active-high pulse will be sent 
*         to the CPU interrupt controller.
*
* \return none.
**/
void EMIF_MskedIntClear(unsigned int intFlag)
{
    if(intFlag == EMIFA_ASYNC_TIMOUT_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSK) |= EMIFA_INTMSK_ATED_MASK;
    }
    else if (intFlag == EMIFA_LINE_TRAP_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSK) &= (~EMIFA_INTMSK_LTED_MASK);        
    }
    else if (intFlag == EMIFA_WAIT_RISE_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSK) &= (~EMIFA_INTMSK_WRED_MASK);                
    }       
}

/**
* \brief  This function Enables the interrupts.
*           
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  intFlag    Interrupt Flag for which interrupt has to enable.\n
*                    This can take one of the following values :
*                    EMIFA_ASYNC_TIMOUT_INT  -  For Asynchronous Timeout
*                                                interrupt.\n
*                    EMIFA_LINE_TRAP_INT     -  For Line Trap interrupt.\n
*                    EMIFA_WAIT_RISE_INT     -  For Wait Rise interrupt.\n
*
* \return none.
*/                 
void EMIFAMskedIntEnable(unsigned int intFlag)
{
    if(intFlag == EMIFA_ASYNC_TIMOUT_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSKSET) |= EMIFA_INTMSKSET_AT_SET_MASK;
    }
    else if (intFlag == EMIFA_LINE_TRAP_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSKSET) |= EMIFA_INTMSKSET_LT_SET_MASK;       
    }
    else if (intFlag == EMIFA_WAIT_RISE_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSKSET) |= EMIFA_INTMSKSET_WR_SET_MASK;       
    }           
}

/**
* \brief  This function Disables the interrupts.
*           
* \param  baseAddr   Memory address of EMIFA.\n
*
* \param  intFlag    Interrupt Flag for which interrupt has to disable.\n
*                    This can take one of the following values :
*                    EMIFA_ASYNC_TIMOUT_INT  -  For Asynchronous Timeout 
*                                               interrupt.\n
*                    EMIFA_LINE_TRAP_INT     -  For Line Trap interrupt.\n
*                    EMIFA_WAIT_RISE_INT     -  For Wait Rise interrupt.\n
*
* \return none.
*/                 
void EMIFAMskedIntDisable(unsigned int intFlag)
{
    if(intFlag == EMIFA_ASYNC_TIMOUT_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSKCLR) |= EMIFA_INTMSKCLR_AT_CLR_MASK;
    }
    else if (intFlag == EMIFA_LINE_TRAP_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSKCLR) |= EMIFA_INTMSKCLR_LT_CLR_MASK;       
    }
    else if (intFlag == EMIFA_WAIT_RISE_INT)
    {
        HWREG(baseAddr + EMIFA_INTMSKCLR) |= EMIFA_INTMSKCLR_WR_CLR_MASK;       
    }           
}
/***************************** End Of File ***********************************/




void EMIF_Init(unsigned char CSNum, unsigned char DataBitWidth, unsigned int WatTime)
{
	//设置片选
	EMIF_NANDCSSet(CSNum);

	/* Enable the Extended wait cycles */
	/* Set the Max extended wait cycles */
	EMIFAMaxExtWaitCycleSet(0xFF);
	/*  Select pin0 to control external wait states. */
	EMIFACSWaitPinSelect(CSNum, EMIFA_EMA_WAIT_PIN0);
	/* set the polarity if EMA_WAIT[0] pin. */
	EMIFAWaitPinPolaritySet(EMIFA_EMA_WAIT_PIN0,
		EMIFA_EMA_WAIT_PIN_POLARITY_HIGH);


	//配置正常模式
	EMIFAAsyncDevOpModeSelect(CSNum, EMIFA_ASYNC_INTERFACE_NORMAL_MODE);

	//EMIF_MASKIntcCfg(0, EMIFA_ASYNC_TIMOUT_INT);
	//EMIF_MASKIntcCfg(0, EMIFA_LINE_TRAP_INT);
	//EMIF_MASKIntcCfg(0, EMIFA_WAIT_RISE_INT);

	//配置异步等待时序
	EMIFAWaitTimingConfig(CSNum, WatTime);

	//配置数据总线
	EMIFAAsyncDevDataBusWidthSelect(CSNum, DataBitWidth);
}

void EMIF_MASKIntcCfg(unsigned char blEnable, int IntcFlag)
{
	if (blEnable)
	{
		EMIFAMskedIntEnable(IntcFlag);
	}
	else
	{
		EMIFAMskedIntDisable(IntcFlag);
	}
}
