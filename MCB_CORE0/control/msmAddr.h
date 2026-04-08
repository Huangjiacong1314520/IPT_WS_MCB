/*
 * msmAddr.h
 *
 *  Created on: 2021Äê8ÔÂ22ÈÕ
 *      Author: lkx
 */

#ifndef CONTROL_MSMADDR_H_
#define CONTROL_MSMADDR_H_

#include <ti/csl/csl_xmcAux.h>


#define MSMTOTALSIZE 0x400000u
#define COREMEMSIZE 0x40000u
#define VXRAWREGIONSIZE   0x100000u
#define INTERACTIONMEMSIZE 0x200000u
#define MSM_MAP_BASEADDR 0x40000000u
#define MSM_PHY_BASEADDR 0x00C000000u
#define MSM_SHAREDATA_BASEADDR (MSM_MAP_BASEADDR + INTERACTIONMEMSIZE)
#define MSM_SHAREDATA_PHY_BASEADDR (MSM_PHY_BASEADDR + INTERACTIONMEMSIZE)
#define MSM_VXCMDLATCH_BASEADDR (MSM_MAP_BASEADDR + VXRAWREGIONSIZE)


#define PADDR(x)     (((unsigned int)x >= MSM_MAP_BASEADDR && (unsigned int)x < (MSM_MAP_BASEADDR + MSMTOTALSIZE))? \
                            ((((unsigned int)x)&0x00FFFFFF)|0x0C000000):(unsigned int)x)
#define XMC_BADDR(x) ((unsigned int)x>>12)
#define XMC_RADDR(x) ((unsigned int)((unsigned long long )x>>12))
#define MAR_INDEX(x) ((unsigned char)((unsigned int)x>>24))

inline void MSMRemap()
{
    CSL_XMC_XMPAXH mpaxh;
    unsigned int index = 0x5;
    mpaxh.bAddr = XMC_BADDR(MSM_MAP_BASEADDR);
    mpaxh.segSize = 21;
    CSL_XMC_setXMPAXH(index, &mpaxh);
    CSL_XMC_XMPAXL mpaxl;
    mpaxl.rAddr = XMC_RADDR(MSM_PHY_BASEADDR);
    mpaxl.sr = 1;
    mpaxl.sw = 1;
    mpaxl.sx = 1;
    mpaxl.ur = 1;
    mpaxl.uw = 1;
    mpaxl.ux = 1;
    CSL_XMC_setXMPAXL(index, &mpaxl);
    CSL_XMC_getXMPAXL(index, &mpaxl);

    CACHE_disableCaching(MAR_INDEX(MSM_MAP_BASEADDR));
}
#endif /* CONTROL_MSMADDR_H_ */
