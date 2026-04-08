/*
 * psc.h
 *
 *  Created on: 2020Äê9ÔÂ3ÈÕ
 *      Author: zhuyb
 */

#ifndef PSC_PSC_H_
#define PSC_PSC_H_


/**************Clock Domain ID**********************/
typedef enum
{
    HW_PSC_SharedLPSC      = 0,  // Always on
    HW_PSC_SmartReflex     = 1,  // Always on
    HW_PSC_DDR3EMIF        = 2,  // Always on
    HW_PSC_EMIF16AndSPI    = 3,
    HW_PSC_TSPI            = 4,
    HW_PSC_DebugSS_Tracers = 5,
    HW_PSC_TETB            = 6,
    HW_PSC_PACKET          = 7,
    HW_PSC_SGMII           = 8,
    HW_PSC_PCIe            = 10,
    HW_PSC_SRIO            = 11,
    HW_PSC_HyperLink       = 12,
    HW_PSC_Reserved        = 13,
    HW_PSC_MSMCRAM         = 14,
    HW_PSC_CorePac0_Timer0 = 15,
    HW_PSC_CorePac1_Timer1 = 16,
    HW_PSC_CorePac2_Timer2 = 17,
    HW_PSC_CorePac3_Timer3 = 18,
    HW_PSC_CorePac4_Timer4 = 19,
    HW_PSC_CorePac5_Timer5 = 20,
    HW_PSC_CorePac6_Timer6 = 21,
    HW_PSC_CorePac7_Timer7 = 22
} Psc0Peripheral;


/*************Power Domain ID**********************/
#define PSC_POWERDOMAIN_ALWAYS_ON    0
#define PSC_POWERDOMAIN_TETB         1
#define PSC_POWERDOMAIN_PACKET       2
#define PSC_POWERDOMAIN_PCIe         3
#define PSC_POWERDOMAIN_SRIO         4
#define PSC_POWERDOMAIN_HyperLink    5
#define PSC_POWERDOMAIN_MSMCRAM      7
#define PSC_POWERDOMAIN_COREPAC0     8
#define PSC_POWERDOMAIN_COREPAC1     9
#define PSC_POWERDOMAIN_COREPAC2     10
#define PSC_POWERDOMAIN_COREPAC3     11
#define PSC_POWERDOMAIN_COREPAC4     12
#define PSC_POWERDOMAIN_COREPAC5     13
#define PSC_POWERDOMAIN_COREPAC6     14
#define PSC_POWERDOMAIN_COREPAC7     15


/*************Power Domain ×´Ì¬**********************/
/* NEXT */
#define PSC_PDCTL_NEXT_OFF (0x00000000u)
#define PSC_PDCTL_NEXT_ON  (0x00000001u)


/*************Clock Domain ×´Ì¬**********************/
/* NEXT */
#define PSC_MDCTL_NEXT_SWRSTDISABLE (0x00000000u)
#define PSC_MDCTL_NEXT_ENABLE       (0x00000003u)


int PSCModuleControl(Psc0Peripheral clockDomain, unsigned int mdflags, unsigned int powerDomain, unsigned int pdflags);

#endif /* PSC_PSC_H_ */
