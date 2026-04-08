#ifndef SPI_FLASH_H
#define SPI_FLASH_H



#define  FLASH_PAGE_SIZE         0x100       /* a page is 256 byte in size */
#define  FLASH_SECTOR_SIZE       0x1000    	 //4K
#define  FLASH_BLOCK_SIZE        0x10000
#define  FLASH_SIZE              0x2000000	//32MB
#define  SPI_TIMEOUT             0xFFFFFFFF

#define FLASH_CMD_WRITE_STATUS   0x01
#define FLASH_CMD_READ_STATUS    0x05
#define FLASH_CMD_WRITE_ENABLE   0x06
#define FLASH_CMD_WRITE_DISABLE  0x04
#define FLASH_CMD_READ           0x03
#define FLASH_CMD_PAGE_PROGRAM   0x02
#define FLASH_CMD_FAST_READ      0x0B
#define FLASH_CMD_SECTOR_ERASE   0x20
#define FLASH_CMD_BLOCK_ERASE    0xD8
#define FLASH_CMD_CHIP_ERASE     0xC7
#define FLASH_CMD_READ_ID        0x9E

#define SPI_CMD_SIZE   0x5

#define SPI_STOP                 0x1
#define SPI_READ                 0x2
#define SPI_WRITE                0x4

#define SPI_RXINTFLG             0x100
#define SPI_OVRNINTFLG           0x040
#define SPI_BITERRFLG            0x010
#define SPI_DESELECTFLG          0x004



#ifdef __cplusplus
extern "C" {
#endif

void spiInit (unsigned char nChip);

int	 spiFlashErase(unsigned int address, int length, char Type);

int  spiFlashWrite(unsigned int address ,unsigned char * buf,int length);

int  spiFlashDataVerify(unsigned int address, unsigned char * buf, int length);

int  spiFlashRead(unsigned int address ,unsigned char * buf,int length);

int  spiFlashReadID(char *pBuf,int nLen);

#ifdef __cplusplus
}
#endif


#endif
