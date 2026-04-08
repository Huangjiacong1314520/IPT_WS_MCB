
#include "../drv_spiflash.h"
#include <stdio.h>
#include <string.h>
#include "hw_types.h"
#include "hw_spi.h"

#define SPI_REG_READ(reg) (*(volatile unsigned int *)(SPI_REG_BASE + reg))
#define SPI_REG_WRITE(reg,data) (*((volatile unsigned int *)(SPI_REG_BASE + reg)) = (data))




#define SPI_CMD_SIZE   0x5

 unsigned int  spiData1 = 0;
 unsigned char  cmd[FLASH_PAGE_SIZE + SPI_CMD_SIZE];


#ifndef min
#define min(x, y)   (((x) < (y)) ? (x) : (y))
#endif


#define FALSE 0
#define TRUE  1
/*******************************************************************************
*
* spiInit - initialize OMAPL138 SPI module.
*
* This routine initializes OMAPL138 SPI module.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

void spiInit (unsigned char nChip)
    {
    /* reset the SPI module */
    int i = 500000;

    SPI_REG_WRITE (SPIGCR0, SPIGCR0_RESET);

    while(i--)
    {

    }

    SPI_REG_WRITE (SPIGCR0, 1);

    /* setup various parameters */

    SPI_REG_WRITE (SPIGCR1, SPIGCR1_CLKMOD | SPIGCR1_MASTER);

    if( 0 == nChip)
    {
        SPI_REG_WRITE (SPIPC0, SPIPC0_SOMIFUN | SPIPC0_SIMOFUN | SPIPC0_CLKFUN | \
                       SPIPC0_SCS0FUN); //Ƭѡ0
    }
    else
    {
        SPI_REG_WRITE (SPIPC0, SPIPC0_SOMIFUN | SPIPC0_SIMOFUN | SPIPC0_CLKFUN | \
                       SPIPC0_SCS1FUN); //Ƭѡ1
    }

    SPI_REG_WRITE (SPIFMT0, SPIFMT_PHASE | SPIFMT_PRESCALE | SPIFMT_CHARLEN);

    spiData1 = SPIDAT1_CSHOLD | SPIDAT1_CSNR;

    SPI_REG_WRITE (SPIDAT1, SPIDAT1_CSHOLD | SPIDAT1_CSNR);

    SPI_REG_WRITE (SPIDELAY, SPIDDELAY_C2TDELAY | SPIDDELAY_T2CDELAY);

    if(nChip == 0)
    {
        SPI_REG_WRITE (SPIDEF, SPIDEF_CSDEF0);  //Ƭѡ0
    }
    else
    {
        SPI_REG_WRITE (SPIDEF, SPIDEF_CSDEF1);  //Ƭѡ1
    }


    SPI_REG_WRITE (SPIINT0, 0);

    SPI_REG_WRITE (SPILVL, 0);

    /* enable SPI module */

    SPI_REG_WRITE (SPIGCR1, SPI_REG_READ(SPIGCR1) | SPIGCR1_ENABLE);

    }

/*******************************************************************************
*
* spiReadWrite - read and write through OMAPL138 SPI module.
*
* This routine uses polling mode to read from or write to data to SPI module.
*
* RETURNS: 1.
*
* ERRNO: N/A.
*/

int spiReadWrite
    (
    unsigned char * buf,
    unsigned int   len,
    unsigned int   flag
    )
    {
    int i;
    unsigned int timeout;
    volatile unsigned int status;

    SPI_REG_READ(SPIBUF);

    for (i = 0; i < len; i++)
        {
        spiData1 &= 0xFFFF0000;
        spiData1 |= buf[i];

        /* if last byte, de-assert cs_hold */

        //spiData1 = 0x9e;
        if ((flag & SPI_STOP) && ((i + 1) == len))
            SPI_REG_WRITE (SPIDAT1, spiData1 & (~SPIDAT1_CSHOLD));
        else
            SPI_REG_WRITE (SPIDAT1, spiData1);

        /* wait for read/write data */

        timeout = 0;
        while (TRUE)
            {
            if (timeout++ > SPI_TIMEOUT)
                {
                SPI_REG_WRITE (SPIDAT1, spiData1 & (~SPIDAT1_CSHOLD));
                break;
                }
            status = SPI_REG_READ (SPIFLG);
            if (status & SPI_RXINTFLG)
                {
                if (flag & SPI_WRITE)
                    SPI_REG_READ (SPIBUF);
                else
                    buf[i] = SPI_REG_READ (SPIBUF);
                break;
                }
            if (status & SPI_OVRNINTFLG)
                {
                if (flag & SPI_WRITE)
                    SPI_REG_READ (SPIBUF);
                else
                    buf[i] = SPI_REG_READ (SPIBUF);
                status &= SPI_OVRNINTFLG;
                SPI_REG_WRITE (SPIFLG, status);
                continue;
                }
            if (status & SPI_BITERRFLG)
                {
                if (flag & SPI_WRITE)
                    SPI_REG_READ (SPIBUF);
                else
                    buf[i] = 0x0;
                break;
                }
            }
        }
    return(1);
    }

/*******************************************************************************
*
* flashWriteEnable - enable SPI flash write
*
* This routine must be called before any write transaction.
*
* RETURNS: always 1.
*
* ERRNO: N/A.
*/

int flashWriteEnable (void)
    {
    cmd[0] = FLASH_CMD_WRITE_ENABLE;

    spiReadWrite (cmd, 1, SPI_STOP);

    return (1);
    }

/*******************************************************************************
*
* flashStatus - get flash status
*
* This routine gets the status of SPI flash.
*
* RETURNS: flash status.
*
* ERRNO: N/A
*/

int flashStatus (void)
    {
    cmd[0] = FLASH_CMD_READ_STATUS;
    cmd[1] = 0;

    spiReadWrite (cmd, 2, SPI_STOP);

    return cmd[1];

    }

/*******************************************************************************
*
* flashRead - read a flash page from the specified address
*
* This routine reads a flash page from the specified address
*
* RETURNS: 1 or 0 if parameter check fail
*
* ERRNO: N/A.
*/

int flashReadPage
    (
    unsigned int   addr,
    unsigned char * buf,
    unsigned int   len
    )
    {
    unsigned char * str = cmd + SPI_CMD_SIZE;

    if ((len == 0) || (len > FLASH_PAGE_SIZE))
        {
        return 0;
        }

    /* setup flash read command */

    cmd[0] = FLASH_CMD_FAST_READ;
    cmd[1] = addr >> 16;
    cmd[2] = addr >> 8;
    cmd[3] = addr >> 0;
    cmd[4] = 0;

    /* write command to flash */

    spiReadWrite (cmd, 5, 0);

    /* read data */

    spiReadWrite (str, FLASH_PAGE_SIZE, SPI_STOP);

    memcpy (buf, str, len);

    return 1;
    }


/*******************************************************************************
*
* flashRead - read flash
*
* This routine reads data from SPI flash.
*
* RETURNS: 1 or 0 if parameter check fail
*
* ERRNO: N/A.
*/

int flashRead
    (
    unsigned int   addr,
    unsigned char * buf,
    unsigned int   len
    )
    {
    if ((len == 0)  || (len > FLASH_SIZE))
        {
        return 0;
        }

    while (len >= FLASH_PAGE_SIZE)
        {
        if (flashReadPage (addr, buf, FLASH_PAGE_SIZE) == 1)
            {
            addr += FLASH_PAGE_SIZE;
            len  -= FLASH_PAGE_SIZE;
            buf  += FLASH_PAGE_SIZE;
            }
        else
            {
            return(0);
            }
        }

    if (len > 0)
        {
        return(flashReadPage (addr, buf, len));
        }

    return 1;
    }

/*******************************************************************************
*
* flashBlockErase - block erase flash
*
* This routine erases a block of SPI flash
*
* RETURNS: 1 or 0 if erase fail
*
* ERRNO: N/A.
*/

int flashBlockErase
    (
    unsigned int   blockAddr
    )
    {
    unsigned int  timeout;
    unsigned int  status;

    /* write enable */

    flashWriteEnable ();

    cmd[0] = FLASH_CMD_BLOCK_ERASE;
    cmd[1] = blockAddr >> 16;
    cmd[2] = blockAddr >> 8;
    cmd[3] = blockAddr >> 0;

    /* send erase command */

    spiReadWrite (cmd, 4, SPI_STOP);

    /* wait erase finish */

    timeout = 0;
    status = flashStatus ();
    while (status & 0x1)
        {
        if (timeout++ > SPI_TIMEOUT)
            {
            return 0;
            }
        status = flashStatus ();
        }
    return 1;
    }

/*******************************************************************************
*
* flashSectorErase - Sector erase flash
*
* This routine erases a sector of SPI flash
*
* RETURNS: 1 or 0 if erase fail
*
* ERRNO: N/A.
*/

int flashSectorErase
    (
    unsigned int   sectorAddr
    )
    {
    unsigned int  timeout;
    unsigned int  status;

    /* write enable */

    flashWriteEnable ();

    cmd[0] = FLASH_CMD_SECTOR_ERASE;
    cmd[1] = sectorAddr >> 16;
    cmd[2] = sectorAddr >> 8;
    cmd[3] = sectorAddr >> 0;

    spiReadWrite (cmd, 4, SPI_STOP);

    /* wait erase finish */

    timeout = 0;
    status = flashStatus ();
    while (status & 0x1)
        {
        if (timeout++ > SPI_TIMEOUT)
            {
            return 0;
            }

        status = flashStatus ();
        }
    return 1;
    }


/*******************************************************************************
*
* flashErase - erase flash
*
* This routine erases SPI flash
*
* RETURNS: 1 or 0 if parameter check fail
*
* ERRNO: N/A.
*/

int flashErase
    (
    unsigned int   addr,
    unsigned int   len,
	unsigned char  nType
    )
    {
    if ((len == 0)  || (len > FLASH_SIZE))
        return 0;

	if (nType)
	{
		while (len > FLASH_BLOCK_SIZE)
		{
			flashBlockErase(addr);
			len -= FLASH_BLOCK_SIZE;
			addr += FLASH_BLOCK_SIZE;
		}

		flashBlockErase(addr);
	}
	else
	{
		while (len > FLASH_SECTOR_SIZE)
		{
			flashSectorErase(addr);
			len -= FLASH_SECTOR_SIZE;
			addr += FLASH_SECTOR_SIZE;
		}

		flashSectorErase(addr);
	}

    return 1;

    }

/*******************************************************************************
*
* flashPageProgram -  flash page write
*
* This routine programms data to SPI flash
*
* RETURNS: 1 or 0 if program fail
*
* ERRNO: N/A.
*/

int flashPageProgram
    (
    unsigned int   pageAddr,
    unsigned char*  buf,
    unsigned int   len
    )
    {
    unsigned int  timeout;
    unsigned int  status;

    /* write enable */

    flashWriteEnable ();

    cmd[0] = FLASH_CMD_PAGE_PROGRAM;
    cmd[1] = pageAddr >> 16;
    cmd[2] = pageAddr >> 8;
    cmd[3] = pageAddr >> 0;

    /* send programm command */

    memcpy (cmd + 4, buf, len);
    spiReadWrite (cmd, 4 + FLASH_PAGE_SIZE, SPI_WRITE | SPI_STOP);

    /* wait erase finish */

    timeout = 0;
    status = flashStatus ();
    while (status & 0x1)
        {
        if (timeout++ > SPI_TIMEOUT)
            {
            return 0;
            }
        status = flashStatus ();
        }
    return 1;
    }


 int flashWrite
    (
    unsigned int         startAddr,
    unsigned char  *       buf,
    unsigned int         len
    )
    {
    unsigned int  timeout;
    unsigned int  status;
    unsigned int  dataLen = 0;
    unsigned int  offset = 0;
    int rc = 0;
    

 
    if ((len == 0)  || (startAddr+len > FLASH_SIZE))
    {
   
    	 return 0;
    }
        
    do
        {
        /* Data must be written in "page Size" chunks */

        while (offset < len)
            {
            /* Compute the number of data ready to write for one write cycle */

            dataLen = min (len - offset, FLASH_PAGE_SIZE -
                           ((startAddr + offset) % FLASH_PAGE_SIZE));

            /* Set page program command */
      
            flashWriteEnable ();

              cmd[0] = FLASH_CMD_PAGE_PROGRAM;
                cmd[1] = (startAddr+offset) >> 16;
                cmd[2] = (startAddr+offset) >> 8;
                cmd[3] = (startAddr+offset) >> 0;

                /* send programm command */

                memcpy (cmd + 4, (void*)(buf + offset), dataLen);
                spiReadWrite (cmd, 4 + dataLen, SPI_WRITE | SPI_STOP);

                /* wait erase finish */

                timeout = 0;
                status = flashStatus ();
                while (status & 0x1)
                    {
                    if (timeout++ > SPI_TIMEOUT)
                        {
                   
                        return 0;
                        
                        }
                    status = flashStatus ();
                    }

            offset += dataLen;
            }

        rc = 1;

        }while(FALSE);

    /* Release resources */
    
    return rc;
 }

int spiFlashWrite(unsigned int	address ,unsigned char * buf,int length)
{

	return flashWrite(address,(unsigned char*)buf,length);
}
int spiFlashDataVerify(unsigned int address, unsigned char * buf, int length)
{
	unsigned char ReadBuf[1024];
	int i = 0, noffset = 0;
	int nCount = length / 1024;
	int nremain = length % 1024;
	
	while (nCount)
	{
		memset(ReadBuf, 0, 1024);
		spiFlashRead(address + noffset, ReadBuf, 1024);
		for (i = 0; i < 1024; i++)
		{
			if (*(buf + noffset + i) != ReadBuf[i])
			{
				return 0;
			}
		}
		noffset += 1024;
		nCount--;
	}

	memset(ReadBuf, 0, 1024);
	for (i = 0; i < nremain; i++)
	{
		if (*(buf + noffset + i) != ReadBuf[i])
		{
			return 0;
		}
	}
	return 1;
}
int spiFlashRead(unsigned int	address ,unsigned char * buf,int length)
{
	return flashRead(address,(unsigned char*)buf,length);

}
int spiFlashErase(unsigned int address, int length, char Type)
{
	return flashErase(address, length, Type);

}

int spiFlashReadID(char *pBuf,int nLen)
{
    if(!pBuf || !nLen)
    {
        return 0;
    }

    cmd[0] = FLASH_CMD_READ_ID;

    spiReadWrite (cmd, 1, 0);

    spiReadWrite ((unsigned char*)pBuf, nLen, SPI_STOP);

    return 1;
}
