#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HrtElf.h"



void osElfHdrRead(char * dest,char* src,int size)
{
	int usSize = size ;// 2;
	int index = 0;
	for(index = 0 ; index < usSize ; index++)
	{
		*dest = *src;
		dest ++;
		src ++;
	}
}


void Hrt_bzero(char * pAddr,int size)
{
	int index = 0;
	for(index = 0 ; index < size ; index++,pAddr++)
	{
		*pAddr = 0;
	}
}

int Hrt_CheckElfHead(char *pBuf, int size)
{
    if(size < EI_NIDENT)
    {
        return 0;
    }

    if((pBuf[0] != 0x7f) || (pBuf[1] != 'E')|| (pBuf[2] != 'L')|| (pBuf[3] != 'F'))
    {
        return 0;
    }

    return 1;

}
int Hrt_ReadEntryOfElfObj(unsigned int pFlashOffsetAddr, HRT_TIOBJ_ENTRY *entry)
{
	struct Elf32_Ehdr ehdr;		/* Elf header structure pointer     */
	struct Elf32_Shdr shdr;		/* Section header structure pointer */
	int index;
	unsigned int Addr;


	osElfHdrRead((char*)&ehdr,(char*)pFlashOffsetAddr,sizeof(struct Elf32_Ehdr));

	for (index = 0; index < ehdr.e_shnum; ++index)
	{
		Addr = pFlashOffsetAddr + ehdr.e_shoff + (index * sizeof (struct Elf32_Shdr));
		osElfHdrRead((char*)&shdr,(char*)Addr,sizeof(struct Elf32_Shdr));
		if (!(shdr.sh_flags & SHF_ALLOC) || (shdr.sh_addr == 0) || (shdr.sh_size == 0)) {
			continue;
		}	
		if (shdr.sh_type == SHT_NOBITS) {
			Hrt_bzero ((void *)shdr.sh_addr, shdr.sh_size);
		}else{
			osElfHdrRead((char*)shdr.sh_addr,(char *) pFlashOffsetAddr + shdr.sh_offset,shdr.sh_size);
		}
	}
	if(entry)
	{
		*entry = (HRT_TIOBJ_ENTRY)(ehdr.e_entry);
	}

	return 0;
}


/*
 * 加载其他核的处理
 * 由于Core0运行在DDR中，其他核不建议运行在DDR中。
 * 并且该函数的处理中也不支持其他core运行在DDR中。
*/

void Hrt_TryBootCore(int core, HRT_TIOBJ_ENTRY entry)
{
    unsigned int * pCoreBootAddr = NULL;
    unsigned int * pIPCGR = NULL;

	if( (core >= 8) || (NULL == entry))
	{
	    return;
	}


    switch(core)
    {
        case 1:
            pIPCGR = (unsigned int *)0x2620244;
            pCoreBootAddr = (unsigned int*)0x1187FFFC;
        break;
        case 2:
            pIPCGR = (unsigned int *)0x2620248;
            pCoreBootAddr = (unsigned int*)0x1287FFFC;
        break;
        case 3:
            pIPCGR = (unsigned int *)0x262024c;
            pCoreBootAddr = (unsigned int*)0x1387FFFC;
        break;
        case 4:
            pIPCGR = (unsigned int *)0x2620250;
            pCoreBootAddr = (unsigned int*)0x1487FFFC;
        break;
        case 5:
            pIPCGR = (unsigned int *)0x2620254;
            pCoreBootAddr = (unsigned int*)0x1587FFFC;
        break;
        case 6:
            pIPCGR = (unsigned int *)0x2620258;
            pCoreBootAddr = (unsigned int*)0x1687FFFC;
        break;
        case 7:
            pIPCGR = (unsigned int *)0x262025c;
            pCoreBootAddr = (unsigned int*)0x1787FFFC;
        break;
        default:

            return;
    }
    *pCoreBootAddr = (unsigned int)entry;
    *pIPCGR = 1;

    return;

}

