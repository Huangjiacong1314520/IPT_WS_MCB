/**
 *  \file   emifa.h
 *
 *  \brief  Definitions used for EMIFA
 *
 *   This file contains the driver API prototypes and macro definitions.
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



#ifndef _EMIFA_H_
#define _EMIFA_H__


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                           MACRO DEFINITIONS
*******************************************************************************/
/*****************************************************************************/

/*CE2CFG */

#define EMIFA_CE2CFG_SS         (0x80000000u)
#define EMIFA_CE2CFG_SS_SHIFT        (0x0000001Fu)

#define EMIFA_CE2CFG_EW         (0x40000000u)
#define EMIFA_CE2CFG_EW_SHIFT        (0x0000001Eu)

#define EMIFA_CE2CFG_W_SETUP    (0x3C000000u)
#define EMIFA_CE2CFG_W_SETUP_SHIFT   (0x0000001Au)

#define EMIFA_CE2CFG_W_STROBE   (0x03F00000u)
#define EMIFA_CE2CFG_W_STROBE_SHIFT  (0x00000014u)

#define EMIFA_CE2CFG_W_HOLD     (0x000E0000u)
#define EMIFA_CE2CFG_W_HOLD_SHIFT    (0x00000011u)

#define EMIFA_CE2CFG_R_SETUP    (0x0001E000u)
#define EMIFA_CE2CFG_R_SETUP_SHIFT   (0x0000000Du)

#define EMIFA_CE2CFG_R_STROBE   (0x00001F80u)
#define EMIFA_CE2CFG_R_STROBE_SHIFT  (0x00000007u)

#define EMIFA_CE2CFG_R_HOLD     (0x00000070u)
#define EMIFA_CE2CFG_R_HOLD_SHIFT    (0x00000004u)

#define EMIFA_CE2CFG_TA         (0x0000000Cu)
#define EMIFA_CE2CFG_TA_SHIFT        (0x00000002u)

#define EMIFA_CE2CFG_ASIZE      (0x00000003u)
#define EMIFA_CE2CFG_ASIZE_SHIFT     (0x00000000u)
/*
** Values that can be passed to EMIFANandCSSet API as CSNum to select 
** Chip Select.
*/

  /* Chip Select 0 */
#define EMIFA_CHIP_SELECT_0                     0
 /* Chip Select 1 */
#define EMIFA_CHIP_SELECT_1                     1
 /* Chip Select 2 */
#define EMIFA_CHIP_SELECT_2                     2
 /* Chip Select 3 */
#define EMIFA_CHIP_SELECT_3                     3


/*****************************************************************************/
/*
** Values that can be used to initialize NANDDevInfo structure
** Chip Select.
*/

  /* Chip Select 2 */
#define EMIFA_CHIP_SELECT_0_SIZE                (0x04000000)
 /* Chip Select 3 */
#define EMIFA_CHIP_SELECT_1_SIZE                (0x04000000)
 /* Chip Select 4 */
#define EMIFA_CHIP_SELECT_2_SIZE                (0x04000000)
 /* Chip Select 5 */
#define EMIFA_CHIP_SELECT_3_SIZE                (0x04000000)


/*****************************************************************************/
/*
** Values that can be passed to EMIFACSWaitPinSelect API as pin to select 
** EMA_WAIT Pin.
*/

 /* EMA_WAIT[0] pin */
#define EMIFA_EMA_WAIT_PIN0                     0
 /* EMA_WAIT[1] pin */
#define EMIFA_EMA_WAIT_PIN1                     1

/*****************************************************************************/
/*
** Values that can be passed to EMIFAWaitPinPolaritySet API as pinPolarity to 
** select Pin polarity.
*/

 /* EMA_WAIT pin polarity low*/
#define EMIFA_EMA_WAIT_PIN_POLARITY_LOW         0
  /* EMA_WAIT pin polarity low*/
#define EMIFA_EMA_WAIT_PIN_POLARITY_HIGH        1


/*****************************************************************************/
/*
** Values that can be passed to EMIFADataBusWidthSelect API as width to set the 
** bus width.
*/

#define EMIFA_DATA_BUSWITTH_8BIT                0
#define EMIFA_DATA_BUSWITTH_16BIT               1

/*****************************************************************************/
/*
** Values that can be passed to EMIFAOpModeSelect API as mode to set the 
** Asynchronous interface opmode.
*/

#define EMIFA_ASYNC_INTERFACE_NORMAL_MODE       0
#define EMIFA_ASYNC_INTERFACE_STROBE_MODE       1

/*****************************************************************************/
/*
** Values that can be passed to EMIFAExtendedWaitEnable API as flag to select 
** or deselect the extended wait cycles.
*/

#define EMIFA_EXTENDED_WAIT_ENABLE              1
#define EMIFA_EXTENDED_WAIT_DISABLE             0

/*****************************************************************************/
/*
** Values that can be passed to EMIFANORPageSizeSet API as pagesize to set the  
** page size for NOR.
*/

#define EMIFA_NOR_PAGE_SIZE_4WORDS              0
#define EMIFA_NOR_PAGE_SIZE_8WORDS              1

/*****************************************************************************/
/*
** Values that can be passed to EMIFANANDEccValGet API as eccValIndex to   
** specify the ecc value to read.
*/

#define EMIFA_NAND_4BITECCVAL1                  1
#define EMIFA_NAND_4BITECCVAL2                  2
#define EMIFA_NAND_4BITECCVAL3                  3
#define EMIFA_NAND_4BITECCVAL4                  4
#define EMIFA_NAND_4BITECCVAL5                  5
#define EMIFA_NAND_4BITECCVAL6                  6
#define EMIFA_NAND_4BITECCVAL7                  7
#define EMIFA_NAND_4BITECCVAL8                  8

/*****************************************************************************/
/*
** Values that can be passed to EMIFANAND4BitEccErrAddrGet API as    
** eccErrAddrIndex to specify the ecc error address to read.
*/

#define EMIFA_4BITECC_ERRADDR_INDEX_1           1
#define EMIFA_4BITECC_ERRADDR_INDEX_2           2
#define EMIFA_4BITECC_ERRADDR_INDEX_3           3
#define EMIFA_4BITECC_ERRADDR_INDEX_4           4

/*****************************************************************************/
/*
** Values that define the state values in the ECC_STATE field of NANDFSR   
** as returned by EMIFANAND4BitECCStateGet.
*/
#define EMIFA_4BITECC_CORRECTION_ECCSTATE_0           (0u)
#define EMIFA_4BITECC_CORRECTION_ECCSTATE_1           (1u)
#define EMIFA_4BITECC_CORRECTION_ECCSTATE_2           (2u)
#define EMIFA_4BITECC_CORRECTION_ECCSTATE_3           (3u)


/*****************************************************************************/
/*
** Values that can be passed to EMIFANANDECCStart API as eccType to specify   
** specify the ecc type.
*/

#define EMIFA_NAND_1BIT_ECC                     0
#define EMIFA_NAND_4BIT_ECC                     1

/*****************************************************************************/
/*
** Values that can be passed to EMIFANAND4BitEccErrValGet API as eccErrValIndex 
** to specify the ecc error value to read.
*/

#define EMIFA_4BITECC_ERRVAL_INDEX_1            1
#define EMIFA_4BITECC_ERRVAL_INDEX_2            2
#define EMIFA_4BITECC_ERRVAL_INDEX_3            3
#define EMIFA_4BITECC_ERRVAL_INDEX_4            4

/*****************************************************************************/
/*
** Values that can be passed to EMIFASDRAMSelfRefModeConfig API as flag 
** to specify the selfrefresh mode to enter or exit.
*/

#define EMIFA_SDRAM_SELFREF_MODE_ENTER          1
#define EMIFA_SDRAM_SELFREF_MODE_EXIT           0

/*****************************************************************************/
/*
** Values that can be passed to EMIFASDRAMPowDownModeConfig API as flag 
** to specify the powerdown mode ot enter or exit.
*/

#define EMIFA_SDRAM_POWDOWN_MODE_ENTER          1
#define EMIFA_SDRAM_POWDOEN_MODE_EXIT           0

/*****************************************************************************/
/*
** Values that can be passed to EMIFA_SDRAM_CONF macro as psize 
** to specify the internal page size.
*/

#define EMIFA_SDRAM_8COLUMN_ADDR_BITS           0
#define EMIFA_SDRAM_9COLUMN_ADDR_BITS           1
#define EMIFA_SDRAM_10COLUMN_ADDR_BITS          2
#define EMIFA_SDRAM_11COLUMN_ADDR_BITS          3

/*****************************************************************************/
/*
** Values that can be passed to EMIFA_SDRAM_CONF macro as ibank 
** to specify the Internal SDRAM bank size.
*/

#define EMIFA_SDRAM_1BANK                       0
#define EMIFA_SDRAM_2BANK                       1
#define EMIFA_SDRAM_4BANK                       2

/*****************************************************************************/
/*
** Values that can be passed to EMIFA_SDRAM_CONF macro as bit11_9lock 
** to specify the CAS lat write lock flag.
*/

#define EMIFA_SDRAM_CAS_WRITE_LOCK              0
#define EMIFA_SDRAM_CAS_WRITE_UNLOCK            1

/*****************************************************************************/
/*
** Values that can be passed to EMIFA_SDRAM_CONF macro as nm 
** to specify the Narrow mode bit.
*/

#define EMIFA_SDRAM_32BIT                       0
#define EMIFA_SDRAM_16BIT                       1

/*****************************************************************************/
/*
** Values that can be passed to EMIFA_SDRAM_CONF macro as caslat 
** to specify the CAS latency.
*/

#define EMIFA_SDRAM_CAS_LAT_2CYCLES             EMIFA_SDCR_CL_CL2
#define EMIFA_SDRAM_CAS_LAT_3CYCLES             EMIFA_SDCR_CL_CL3

/*****************************************************************************/
/*
** Values that can be passed to EMIFANORPageModeConfig API as flag 
** to specify the page mode to enable or disable.
*/

#define EMIFA_NOR_PAGEMODE_ENABLE               1
#define EMIFA_NOR_PAGEMODE_DISABLE              0

/*****************************************************************************/
/*
** Values that can be passed to EMIFARawIntStatusRead,EMIFARawIntClear,
** EMIFAMskedIntStatusRead,EMIFAMskedIntClear,EMIFAMskedIntSet,
** EMIFAMskedIntClear API as intFlag to specify the interrupt name.
*/

#define EMIFA_ASYNC_TIMOUT_INT                  1
#define EMIFA_LINE_TRAP_INT                     2
#define EMIFA_WAIT_RISE_INT                     3





/* NAND Module clk frequency                                        */
#define NAND_MODULE_CLK                         ((100u)*(1000u)*(1000u))
#define NAND_MODULE_CLK_IN_MHZ                  (NAND_MODULE_CLK / 1000000)



/* Setup,strobe,hold times for read/write for the dev MT29F4G08AAA  */
#define NAND_WRITE_SETUP_TIME_IN_NS             (0u)
#define NAND_WRITE_STROBE_TIME_IN_NS            (30u)
#define NAND_WRITE_HOLD_TIME_IN_NS              (30u)
#define NAND_READ_SETUP_TIME_IN_NS              (20u)
#define NAND_READ_STROBE_TIME_IN_NS             (40u)
#define NAND_READ_HOLD_TIME_IN_NS               (0u)
#define NAND_TURN_ARND_TIME_IN_NS               (0u)


/* Setup,strobe,hold times reset values                             */
#define EMIFA_WRITE_SETUP_RESETVAL              (0x0F)
#define EMIFA_WRITE_STROBE_RESETVAL             (0x3F)
#define EMIFA_WRITE_HOLD_RESETVAL               (0x07)
#define EMIFA_READ_SETUP_RESETVAL               (0x0F)
#define EMIFA_READ_STROBE_RESETVAL              (0x3F)
#define EMIFA_READ_HOLD_RESETVAL                (0x07)
#define EMIFA_TA_RESETVAL                       (0x03)

/*****************************************************************************/
/*
* \brief  This macro used to make the conf value which is used to configure the
*           async wait time.\n 
*
* \param  wset      Write setup time or width in EMA_CLK cycles.\n
*
*         wstb      Write strobe time or width in EMA_CLK cycles.\n
*
*         whld      Write hold  time or width in EMA_CLK cycles.\n
*
*         rset      Read setup time or width in EMA_CLK cycles.\n
*
*         rstb      Read strobe time or width in EMA_CLK cycles.\n
*
*         rhld      Read hold  time or width in EMA_CLK cycles.\n
*
*         ta        Minimum Turn-Around time..\n
*
*/

#define EMIFA_ASYNC_WAITTIME_CONFIG(wset, wstb, whld, rset, rstb, rhld, ta )   ((unsigned int) \
                                                                                ((wset << EMIFA_CE2CFG_W_SETUP_SHIFT) & EMIFA_CE2CFG_W_SETUP) | \
                                                                                ((wstb << EMIFA_CE2CFG_W_STROBE_SHIFT) & EMIFA_CE2CFG_W_STROBE) | \
                                                                                ((whld << EMIFA_CE2CFG_W_HOLD_SHIFT) & EMIFA_CE2CFG_W_HOLD) | \
                                                                                ((rset << EMIFA_CE2CFG_R_SETUP_SHIFT) & EMIFA_CE2CFG_R_SETUP) | \
                                                                                ((rstb << EMIFA_CE2CFG_R_STROBE_SHIFT) & EMIFA_CE2CFG_R_STROBE) | \
                                                                                ((rhld << EMIFA_CE2CFG_R_HOLD_SHIFT) & EMIFA_CE2CFG_R_HOLD) | \
                                                                                ((ta << EMIFA_CE2CFG_TA_SHIFT) & EMIFA_CE2CFG_TA))

/*****************************************************************************/
/*
* \brief  This macro used to make the conf value which is used to configure the
*           SDRAM.\n 
*
* \param  psize       -- internal page size.It can take follwing values.
*                           EMIFA_SDRAM_8COLUMN_ADDR_BITS
*                           EMIFA_SDRAM_9COLUMN_ADDR_BITS
*                           EMIFA_SDRAM_10COLUMN_ADDR_BITS
*                           EMIFA_SDRAM_11COLUMN_ADDR_BITS
*         ibank       -- Internal SDRAM bank size. It can take following values.
*                           EMIFA_SDRAM_1BANK
*                           EMIFA_SDRAM_2BANK
*                           EMIFA_SDRAM_4BANK
*         bit11_9lock -- CAS lat write lock flag. It can take following values.
*                           EMIFA_SDRAM_CAS_WRITE_LOCK
*                           EMIFA_SDRAM_CAS_WRITE_UNLOCK
*         caslat      -- CAS latency. It can take following values.
*                           EMIFA_SDRAM_CAS_LAT_2CYCLES
*                           EMIFA_SDRAM_CAS_LAT_3CYCLES
*         nm          -- Narrow mode bit.This defines whether a 16- or 
*                        32-bit-wide SDRAM is connected to the EMIFA.It can
*                        take following values.
*                           EMIFA_SDRAM_32BIT
*                           EMIFA_SDRAM_16BIT
*
*/
                               
#define EMIFA_SDRAM_CONF(psize, ibank, bit11_9lock, caslat, nm )   ((unsigned int) \
                                                                    ((psize << EMIFA_SDCR_PAGESIZE_SHIFT) & EMIFA_SDCR_PAGESIZE) | \
                                                                    ((ibank << EMIFA_SDCR_IBANK_SHIFT) & EMIFA_SDCR_IBANK) | \
                                                                    ((bit11_9lock << EMIFA_SDCR_BIT11_9LOCK_SHIFT) & EMIFA_SDCR_BIT11_9LOCK) | \
                                                                    ((caslat << EMIFA_SDCR_CL_SHIFT) & EMIFA_SDCR_CL) | \
                                                                    ((nm << EMIFA_SDCR_NM_SHIFT) & EMIFA_SDCR_NM))
                                                                    
/*****************************************************************************/
/*
* \brief  This macro used to make the conf value which is used to configure the
*           SDRAM.\n 
*
* \param  t_rrd   -- internal page size.It can take follwing values.
*         t_rc    -- EMA_CLK clock cycles from Activate to Activate 
*         t_ras   -- EMA_CLK clock cycles from Activate(ACTV) to Precharge(PRE)
*         t_wr    -- EMA_CLK cycles from last Write (WRT) to Precharge (PRE)
*         t_rcd   -- EMA_CLK cycles from Active(ACTV) to Rd(READ) or Wr(WRT).
*         t_rp    -- EMA_CLK cycles from Precharge (PRE) to Activate (ACTV) 
*                       or Refresh (REFR) command,
*         t_rfc   -- EMA_CLK cycles from Refresh (REFR) to Refresh (REFR).
*
*/                                                                  
                                                                                                                            
#define EMIFA_SDRAM_TIMING_CONF(t_rrd, t_rc, t_ras, t_wr, t_rcd, t_rp, t_rfc )   ((unsigned int) \
                                                                                  ((t_rrd << EMIFA_SDTIMR_T_RRD_SHIFT) & EMIFA_SDTIMR_T_RRD) | \
                                                                                  ((t_rc << EMIFA_SDTIMR_T_RC_SHIFT) & EMIFA_SDTIMR_T_RC) | \
                                                                                  ((t_ras << EMIFA_SDTIMR_T_RAS_SHIFT) & EMIFA_SDTIMR_T_RAS) | \
                                                                                  ((t_wr << EMIFA_SDTIMR_T_WR_SHIFT) & EMIFA_SDTIMR_T_WR) | \
                                                                                  ((t_rcd << EMIFA_SDTIMR_T_RCD_SHIFT) & EMIFA_SDTIMR_T_RCD) | \
                                                                                  ((t_rp << EMIFA_SDTIMR_T_RP_SHIFT) & EMIFA_SDTIMR_T_RP) | \
                                                                                  ((t_rfc << EMIFA_SDTIMR_T_RFC_SHIFT) & EMIFA_SDTIMR_T_RFC))

/***************************************************************************/

/*
** Function Prototypes
*/

unsigned int  EMIF_ModuleIdRead();
void          EMIF_Init(unsigned char CSNum, unsigned char DataBitWidth, unsigned int WatTime);
void          EMIF_NANDCSSet(unsigned int CSNum);
void          EMIF_MASKIntcCfg(unsigned char blEnable, int IntcFlag);
unsigned int  EMIF_MskedIntStatusRead(unsigned int intFlag);
void          EMIF_MskedIntClear(unsigned int intFlag);
unsigned int  EMIF_WaitPinStatusGet(unsigned int pinNum);


void          EMIF_NANDECCStart(unsigned int eccType, unsigned int CSNum);
void          EMIF_NAND4BitECCSelect(unsigned int CSNum);
unsigned int  EMIF_NANDEccValGet(unsigned int eccType, unsigned int eccValIndexOrCS);
unsigned int  EMIF_NAND4BitEccErrAddrGet(unsigned int eccErrAddrIndex);
unsigned int  EMIF_NAND4BitECCNumOfErrsGet();
unsigned int  EMIF_NAND4BitECCStateGet();
void 		  EMIF_NAND4BitECCAddrCalcStart();
unsigned int  EMIF_NAND4BitEccErrValGet(unsigned int eccErrValIndex);
void 		  EMIF_NAND4BitECCLoad(unsigned int eccLdVal);
#ifdef __cplusplus
}
#endif
#endif
