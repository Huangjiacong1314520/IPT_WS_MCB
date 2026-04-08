/**
 * \file  hw_spi.h
 *
 * \brief SPI register definitions
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



#ifndef _HW_SPI_H_
#define _HW_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif
/* spi register define */

#define  SPI_REG_BASE              (0x20BF0000)     //»ùµØÖ·

#define  SPIGCR0                   0x00
#define  SPIGCR1                   0x04
#define  SPIINT0                   0x08
#define  SPILVL                    0x0C
#define  SPIFLG                    0x10
#define  SPIPC0                    0x14
#define  SPIPC1                    0x18
#define  SPIPC2                    0x1C
#define  SPIPC3                    0x20
#define  SPIPC4                    0x24
#define  SPIPC5                    0x28
#define  SPIDAT0                   0x38
#define  SPIDAT1                   0x3C
#define  SPIBUF                    0x40
#define  SPIEMU                    0x44
#define  SPIDELAY                  0x48
#define  SPIDEF                    0x4C
#define  SPIFMT0                   0x50
#define  SPIFMT1                   0x54
#define  SPIFMT2                   0x58
#define  SPIFMT3                   0x5C
#define  INTVEC0                   0x60
#define  INTVEC1                   0x64

#define  SPIGCR0_RESET             0x00

#define  SPIGCR1_ENABLE            (1 << 24)
#define  SPIGCR1_CLKMOD            (1 << 1)
#define  SPIGCR1_MASTER            (1 << 0)

#define  SPIPC0_SOMIFUN            (1 << 11)
#define  SPIPC0_SIMOFUN            (1 << 10)
#define  SPIPC0_CLKFUN             (1 << 9)
#define  SPIPC0_ENAFUN             (1 << 8)
#define  SPIPC0_SCS0FUN            (1 << 0)
#define  SPIPC0_SCS1FUN            (1 << 1)

#define  SPIDAT1_CSHOLD            (1 << 28)
#define  SPIDAT1_WDEL              (1 << 26)
#define  SPIDAT1_CSNR              (0 << 16)

#define  SPIDDELAY_C2TDELAY        (25 << 24)
#define  SPIDDELAY_T2CDELAY        (25 << 16)

//#define  SPIDDELAY_C2TDELAY        (50 << 24)
//#define  SPIDDELAY_T2CDELAY        (50 << 16)


#define  SPIDDELAY_T2EDELAY        (0 << 8)
#define  SPIDDELAY_C2EDELAY        (0 << 0)

#define  SPIDEF_CSDEF0             (1 << 0)
#define  SPIDEF_CSDEF1             (1 << 1)

#define  SPIBUF_RXEMPTY            (1 << 31)
#define  SPIBUF_RXOVR              (1 << 30)
#define  SPIBUF_TXFULL             (1 << 29)
#define  SPIBUF_BITERR             (1 << 28)
#define  SPIBUF_DESYNC             (1 << 27)
#define  SPIBUF_PARERR             (1 << 26)
#define  SPIBUF_TIMEOUT            (1 << 25)
#define  SPIBUF_DLENERR            (1 << 24)

#define  SPIFMT_PARPOL             (1 << 23)
#define  SPIFMT_PARENA             (1 << 22)
#define  SPIFMT_WAITENA            (1 << 21)
#define  SPIFMT_SHIFTDIR           (1 << 20)
#define  SPIFMT_DISCSTIMERS        (1 << 18)
#define  SPIFMT_POLARITY           (1 << 17)
#define  SPIFMT_PHASE              (1 << 16)
//#define  SPIFMT_PRESCALE           (40 << 8)
#define  SPIFMT_PRESCALE           (7 << 8)
#define  SPIFMT_CHARLEN            (8 << 0)

#ifdef __cplusplus
}
#endif

#endif
