
/* HW Macros */
#include "hw_types.h"

#include <stdio.h>
#include <stdlib.h>


/* DSP System Defines */
#include "../drv_psc.h"


#define     hPscRegs                            ((PSC_Regs *) (SOC_PSC_0_REGS))

/**************************************************************************
  API FUNCTION DEFINITIONS
***************************************************************************/

/**
 *
 *  \brief   This function sets the requested module in the required state
 *
 * \param     baseAdd         Memory address of the PSC instance used.
 * \param     moduleId        The module number of the module to be commanded.
 * \param     powerDomain     The power domain of the module to be commanded.
 * \param     flags           This contains the flags that is a logical OR of
 *                            the commands that can be given to a module.
 *
 * \return                    0 in case of successful transition, -1 otherwise.
 *            
 */

#define SOC_PSC_0_REGS                      (0x02350000)

#define ON          1
#define OFF         0

typedef unsigned int Uint32;
typedef unsigned char Uint8;
typedef unsigned char Bool;


typedef struct {
	volatile Uint32 PID;
	volatile Uint8 RSVD0[16];
	volatile Uint32 VCNTLID;
	volatile Uint8 RSVD1[264];
	volatile Uint32 PTCMD;
	volatile Uint8 RSVD2[4];
	volatile Uint32 PTSTAT;
	volatile Uint8 RSVD3[212];
	volatile Uint32 PDSTAT[32];
	volatile Uint8 RSVD4[128];
	volatile Uint32 PDCTL[32];
	volatile Uint8 RSVD5[1152];
#if defined(SOC_C6678)||defined(SOC_C6657)
	volatile Uint32 MDSTAT[32];
	volatile Uint8 RSVD6[384];
	volatile Uint32 MDCTL[32];
#else
	volatile Uint32 MDSTAT[64];
	volatile Uint8 RSVD6[256];
	volatile Uint32 MDCTL[64];
#endif
} PSC_Regs;




/** @brief
*
*  Possible PSC Power domain states
*/
typedef enum {
	/** Power domain is Off */
	PSC_PDSTATE_OFF = 0,

	/** Power domain is On */
	PSC_PDSTATE_ON = 1
} PSC_PDSTATE;

/** @brief
*
*  Possible PSC Module states
*/
typedef enum {
	/** Module is in Reset state. Clock is off. */
	PSC_MODSTATE_SWRSTDISABLE = 0,


	/** Module is in enable state. */
	PSC_MODSTATE_ENABLE = 3,


} PSC_MODSTATE;

/** @brief
*
*  Possible module local reset status
*/
typedef enum {
	/** Module local reset asserted */
	PSC_MDLRST_ASSERTED = 0,

	/** Module local reset deasserted */
	PSC_MDLRST_DEASSERTED = 1
} PSC_MDLRST;

/** @brief
*
*  Possible module reset status
*/
typedef enum {
	/** Module reset asserted */
	PSC_MDRST_ASSERTED = 0,

	/** Module reset deasserted */
	PSC_MDRST_DEASSERTED = 1
} PSC_MDRST;


/****************************************************************************/
/*                                                                          */
/*              Œª”Ú∂®“Â                                                    */
/*                                                                          */
/****************************************************************************/
/* REVID */
#define PSC_REVID_REV          (0xFFFFFFFFu)
#define PSC_REVID_REV_SHIFT         (0x00000000u)

/*VCNELID*/
#define PSC_VCNTL      (0x00003F00u)
#define PSC_VCNTL_SHIFT     (0x00000010u)
/* INTEVAL */
#define PSC_INTEVAL_ALLEV      (0x00000001u)
#define PSC_INTEVAL_ALLEV_SHIFT     (0x00000000u)

/* PDSTAT */
#define PSC_PDSTAT_STATE       (0x00000001u)
#define PSC_PDSTAT_STATE_SHIFT      (0x00000000u)

/* PDCTL */
#define PSC_PDCTL_NEXT         (0x00000001u)
#define PSC_PDCTL_NEXT_SHIFT        (0x00000000u)


#define PSC_REVID       (0x0)
#define PSC_VCNTLID     (0x14)
#define PSC_PTCMD       (0x120)
#define PSC_PTSTAT      (0x128)
#define PSC_PDSTAT(n)   (0x200 + (n * 4))
#define PSC_PDCTL(n)    (0x300 + (n * 4))
#define PSC_MDSTAT(n)   (0x800 + (n * 4))
#define PSC_MDCTL(n)    (0xA00 + (n * 4))

/* MDSTAT */
#define PSC_MDSTAT_MCKOUT      (0x00001000u)
#define PSC_MDSTAT_MCKOUT_SHIFT     (0x0000000Cu)
#define PSC_MDSTAT_MRSTDONE    (0x00000800u)
#define PSC_MDSTAT_MRSTDONE_SHIFT   (0x0000000Bu)
#define PSC_MDSTAT_MRST        (0x00000400u)
#define PSC_MDSTAT_MRST_SHIFT       (0x0000000Au)
#define PSC_MDSTAT_LRSTDONE    (0x00000200u)
#define PSC_MDSTAT_LRSTDONE_SHIFT   (0x00000009u)
#define PSC_MDSTAT_LRST        (0x00000100u)
#define PSC_MDSTAT_LRST_SHIFT       (0x00000008u)
#define PSC_MDSTAT_STATE       (0x0000003Fu)
#define PSC_MDSTAT_STATE_SHIFT      (0x00000000u)
/* STATE */
#define PSC_MDSTAT_STATE_SWRSTDISABLE (0x00000000u)
#define PSC_MDSTAT_STATE_ENABLE       (0x00000003u)

/* MDCTL */
#define PSC_MDCTL_RESETISO     (0x00001000u)
#define PSC_MDCTL_RESETISO_SHIFT    (0x0000000Cu)
#define PSC_MDCTL_RSTISOENABLE   (0x00001000u)

#define PSC_MDCTL_LRST         (0x00000100u)
#define PSC_MDCTL_LRST_SHIFT        (0x00000008u)
#define PSC_MDCTL_NEXT         (0x0000001Fu)
#define PSC_MDCTL_NEXT_SHIFT        (0x00000000u)



int PSCModuleControl(Psc0Peripheral clockDomain, unsigned int mdflags, unsigned int powerDomain, unsigned int pdflags)
{

    unsigned int baseAdd = SOC_PSC_0_REGS;

    volatile unsigned int timeout = 0xFFFFFF;
    int retVal = 0;
    unsigned int status = 0;

    HWREG(baseAdd + PSC_PDCTL(powerDomain)) = (pdflags & PSC_PDCTL_NEXT);
    HWREG(baseAdd + PSC_MDCTL(clockDomain)) = (mdflags & PSC_MDCTL_NEXT);
    HWREG(baseAdd + PSC_PTCMD) = (1 << powerDomain);

	do
	{
		status = HWREG(baseAdd + PSC_PTSTAT) & (1 << powerDomain);
	} while(status && timeout--);

    if(timeout != 0)
    {
        timeout = 0xFFFFFF;
        status = mdflags & PSC_MDCTL_NEXT;

        do
        {
            timeout--;
        } while(timeout && (HWREG(baseAdd + PSC_MDSTAT(clockDomain)) & PSC_MDSTAT_STATE) != status);
    }

    if(timeout == 0)
    {
        retVal = -1;
    }

    return retVal;
}






/** ============================================================================
 *   @n@b PSC_getVersionInfo
 *
 *   @b Description
 *   @n This function retrieves the PSC peripheral identification register
 *      contents.
 *
 *   @b Arguments
 *   @n None
 *
 *   <b> Return Value </b>  Uint32  - version value
 *
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *   @n  None
 *
 *   @b Reads
 *   @n PSC_PID
 *
 *   @b Example
 *   @verbatim
        Uint32      versionInfo;

        versionInfo =   PSC_getVersionInfo ();

     @endverbatim
 * =============================================================================
 */
Uint32 PSC_getVersionInfo (void)
{

    return hPscRegs->PID;
}


/** ============================================================================
 *   @n@b PSC_getVoltageControl
 *
 *   @b Description
 *   @n This function retrieves the Smart reflex bits from the voltage
 *      control identification register.
 *
 *   @b Arguments
 *   @n None
 *
 *   <b> Return Value </b>  Uint8
 *
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *   @n  None
 *
 *   @b Reads
 *   @n PSC_VCNTLID_VCNTL
 *
 *   @b Example
 *   @verbatim
        Uint32      vcntlInfo;

        vcntlInfo =   PSC_getVoltageControl ();

     @endverbatim
 * =============================================================================
 */
Uint8 PSC_getVoltageControl (void)
{

    return  (hPscRegs->VCNTLID &PSC_VCNTL)>>PSC_VCNTL_SHIFT;
}


/** ============================================================================
 *   @n@b PSC_setModuleNextState
 *
 *   @b Description
 *   @n This function sets up the "Next" state the module should be
 *      transitioned for a given module.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       LPSC Module for which the next state must be configured.
        state           Next state to which the module must be transitioned.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *   @n  Module is moved to configured next state after transition is triggered
 *       using @a PSC_startStateTransition () API.
 *
 *   @b  Writes
 *   @n  PSC_MDCTL_NEXT
 *
 *   @b  Example
 *   @verbatim
        ...
        // Enable Module 1's clock.
        PSC_setModuleNextState (1, PSC_MODSTATE_ENABLE);
        ...
     @endverbatim
 * ============================================================================
 */
void PSC_setModuleNextState (
    Uint32                  moduleNum,
    PSC_MODSTATE        state
)
{
   hPscRegs->MDCTL[moduleNum]  = (hPscRegs->MDCTL[moduleNum] & ~PSC_MDCTL_NEXT) | (state&PSC_MDCTL_NEXT);

    return;
}

/** ===========================================================================
 *   @n@b PSC_getModuleNextState
 *
 *   @b Description
 *   @n This function returns the next state configured for a given module.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the state must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  PSC_MODSTATE
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDCTL_NEXT
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's next state configured
        if (PSC_getModuleNextState (2) == PSC_MODSTATE_ENABLE)
        {
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
PSC_MODSTATE PSC_getModuleNextState  (
    Uint32                  moduleNum
)
{
    return (PSC_MODSTATE)  (hPscRegs->MDCTL[moduleNum]&PSC_MDCTL_NEXT);
}

/** ===========================================================================
 *   @n@b PSC_setModuleLocalReset
 *
 *   @b Description
 *   @n This function configures the Module local reset control.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
        resetState      Assert/Deassert module local reset.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Writes
 *   @n PSC_MDCTL_LRST
 *
 *   @b Example
 *   @verbatim
        ...
        // Assert Module 2's local reset
        PSC_setModuleLocalReset (2, PSC_MDLRST_ASSERTED);
        ...
     @endverbatim
 * ============================================================================
 */
void PSC_setModuleLocalReset  (
    Uint32               moduleNum,
    PSC_MDLRST          resetState
)
{
     hPscRegs->MDCTL[moduleNum] =  (hPscRegs->MDCTL[moduleNum] & ~PSC_MDCTL_LRST) |(PSC_MDCTL_LRST&resetState);

    return;
}

/** ===========================================================================
 *   @n@b PSC_getModuleLocalReset
 *
 *   @b Description
 *   @n This function reads the Module local reset control configured.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  PSC_MDLRST
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDCTL_LRST
 *
 *   @b Example
 *   @verbatim
 *      Uint32  resetState;

        ...
        // Check Module 2's local reset bit
        resetState = PSC_getModuleLocalReset (2);
        ...
     @endverbatim
 * ============================================================================
 */
 PSC_MDLRST PSC_getModuleLocalReset  (
    Uint32                  moduleNum
)
{
    return (PSC_MDLRST)  (hPscRegs->MDCTL[moduleNum]& ~PSC_MDCTL_LRST)>>PSC_MDCTL_RESETISO_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_enableModuleResetIsolation
 *
 *   @b Description
 *   @n This function enables the Module reset isolation control.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the configuration must be done.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Writes
 *   @n PSC_MDCTL_RSTISO=1
 *
 *   @b Example
 *   @verbatim
        ...
        // Enable Module 2's reset isolation
        PSC_enableModuleResetIsolation (2);
        ...
     @endverbatim
 * ============================================================================
 */
void PSC_enableModuleResetIsolation  (
    Uint32                  moduleNum
)
{
    hPscRegs->MDCTL[moduleNum] =  (hPscRegs->MDCTL[moduleNum] &  ~PSC_MDCTL_RESETISO) | PSC_MDCTL_RSTISOENABLE;

    return;
}


/** ===========================================================================
 *   @n@b PSC_disableModuleResetIsolation
 *
 *   @b Description
 *   @n This function disables the Module reset isolation control.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the configuration must be done.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Writes
 *   @n PSC_MDCTL_RSTISO=0
 *
 *   @b Example
 *   @verbatim
        ...
        // Disable Module 2's reset isolation
        PSC_disableModuleResetIsolation (2);
        ...
     @endverbatim
 * ============================================================================
 */
void PSC_disableModuleResetIsolation  (
    Uint32                  moduleNum
)
{
    hPscRegs->MDCTL[moduleNum] = (hPscRegs->MDCTL[moduleNum] & ~PSC_MDCTL_RESETISO);

    return;
}


/** ===========================================================================
 *   @n@b PSC_isModuleResetIsolationEnabled
 *
 *   @b Description
 *   @n This function reads the Module reset isolation control bit.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  Bool
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDCTL_RSTISO
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's reset isolation configuration
        if (PSC_isModuleResetIsolationEnabled (2) == TRUE)
        {
            // Module 2 reset isolation enabled
        }
        ...
     @endverbatim
 * ============================================================================
 */
Bool PSC_isModuleResetIsolationEnabled  (
    Uint32                  moduleNum
)
{
    return  (hPscRegs->MDCTL[moduleNum] & PSC_MDCTL_RESETISO)>>PSC_MDCTL_RESETISO_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_getModuleState
 *
 *   @b Description
 *   @n This function returns the current state of a given module.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the state must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  PSC_MODSTATE
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n Power Domain status is returned
 *
 *   @b Reads
 *   @n PSC_MDSTAT_STATE
 *
 *   @b Example
 *   @verbatim
        ...
        // Check if Module 2's clock is enabled.
        if (PSC_getModuleState (2) == PSC_MODSTATE_ENABLE)
        {
            // Module 2's clock is enabled.
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
PSC_MODSTATE PSC_getModuleState  (
    Uint32                  moduleNum
)
{
    return (PSC_MODSTATE) (hPscRegs->MDSTAT[moduleNum] & PSC_MDSTAT_STATE)>>PSC_MDSTAT_STATE_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_getModuleLocalResetStatus
 *
 *   @b Description
 *   @n This function returns the Module local reset actual status.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  PSC_MDLRST
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDSTAT_LRST
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's local reset status
        if (PSC_getModuleLocalResetStatus (2) == PSC_MDLRST_ASSERTED)
        {
            // Module 2's local reset asserted.
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
 PSC_MDLRST PSC_getModuleLocalResetStatus  (
    Uint32                  moduleNum
)
{
    return (PSC_MDLRST) (hPscRegs->MDSTAT[moduleNum] & PSC_MDSTAT_LRST)>>PSC_MDSTAT_LRST_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_isModuleLocalResetDone
 *
 *   @b Description
 *   @n This function returns the Module local reset initialization done status.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  Bool
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDSTAT_LRSTDONE
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's local reset initialization done status
        if (PSC_isModuleLocalResetDone (2))
        {
            // Module 2's local reset init done.
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
Bool PSC_isModuleLocalResetDone  (
    Uint32                  moduleNum
)
{
    return (Bool) (hPscRegs->MDSTAT[moduleNum] & PSC_MDSTAT_LRSTDONE)>>PSC_MDSTAT_LRSTDONE_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_getModuleResetStatus
 *
 *   @b Description
 *   @n This function returns the Module reset actual status.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  PSC_MDRST
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDSTAT_MRST
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's reset status
        if (PSC_getModuleResetStatus (2) == PSC_MDRST_ASSERTED)
        {
            // Module 2's reset asserted.
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
PSC_MDRST PSC_getModuleResetStatus  (
    Uint32                  moduleNum
)
{
    return (PSC_MDRST) (hPscRegs->MDSTAT[moduleNum] & PSC_MDSTAT_MRST)>>PSC_MDSTAT_MRST_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_isModuleResetDone
 *
 *   @b Description
 *   @n This function returns the Module reset initialization done status.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  Bool
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDSTAT_MRSTDONE
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's reset initialization done status
        if (PSC_isModuleResetDone (2))
        {
            // Module 2's reset init done.
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
 Bool PSC_isModuleResetDone  (
    Uint32                  moduleNum
)
{
    return (Bool) (hPscRegs->MDSTAT[moduleNum] & PSC_MDSTAT_MRSTDONE) >> PSC_MDSTAT_MRSTDONE_SHIFT;
}


/** ===========================================================================
 *   @n@b PSC_isModuleClockOn
 *
 *   @b Description
 *   @n This function returns the actual modclk output to module.
 *
 *   @b Arguments
 *   @verbatim
        moduleNum       Module number for which the clock status must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  Bool
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n None
 *
 *   @b Reads
 *   @n PSC_MDSTAT_MCKOUT
 *
 *   @b Example
 *   @verbatim
        ...
        // Check Module 2's modclk status
        if (PSC_isModuleClockOn (2))
        {
            // Module 2's modclk on.
            ...
        }
        else
        {
            // Module 2's modclk gated.
            ...
        }
        ...
     @endverbatim
 * ============================================================================
 */
 Bool PSC_isModuleClockOn  (
    Uint32                  moduleNum
)
{
    return (Bool) (hPscRegs->MDSTAT[moduleNum] & PSC_MDSTAT_MCKOUT)>>PSC_MDSTAT_MCKOUT_SHIFT;
}


/** ============================================================================
 *   @n@b PSC_enablePowerDomain
 *
 *   @b Description
 *   @n This function enables the specified power domain.
 *
 *   @b Arguments
 *   @verbatim
        pwrDmnNum       Power domain number that needs to be enabled.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *   @n  Power domain is enabled.
 *
 *   @b  Writes
 *   @n  PSC_PDCTL_NEXT=1
 *
 *   @b  Example
 *   @verbatim
        ...
        // On the power domain 2
        PSC_enablePowerDomain (2);
        ...
     @endverbatim
 * ============================================================================
 */
void PSC_enablePowerDomain (
    Uint32                  pwrDmnNum
)
{
     hPscRegs->PDCTL[pwrDmnNum] = (hPscRegs->PDCTL[pwrDmnNum] & ~PSC_PDCTL_NEXT) |  1;

    return;
}


/** ============================================================================
 *   @n@b PSC_disablePowerDomain
 *
 *   @b Description
 *   @n This function turns off the specified power domain.
 *
 *   @b Arguments
 *   @verbatim
        pwrDmnNum       Power domain number that needs to be disabled.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *   @n  Power domain is disabled.
 *
 *   @b  Writes
 *   @n  PSC_PDCTL_NEXT=0
 *
 *   @b  Example
 *   @verbatim
        ...
        // Off the power domain 2
        PSC_disablePowerDomain (2);
        ...
     @endverbatim
 * ============================================================================
 */
 void PSC_disablePowerDomain (
    Uint32                  pwrDmnNum
)
{
    hPscRegs->PDCTL[pwrDmnNum] =  (hPscRegs->PDCTL[pwrDmnNum] & ~PSC_PDCTL_NEXT);

    return;
}



/** ===========================================================================
 *   @n@b PSC_getPowerDomainState
 *
 *   @b Description
 *   @n This function returns the current state of a given power domain.
 *
 *   @b Arguments
 *   @verbatim
        pwrDmnNum       Power domain number for which the state must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  PSC_PDSTATE
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n Power Domain status is returned
 *
 *   @b Reads
 *   @n PSC_PDSTAT_STATE
 *
 *   @b Example
 *   @verbatim
        ...
        // Check if Power domain is On.
        if (PSC_getPowerDomainState(2) == PSC_PDSTATE_ON)
        {
            // Power domain 2 is on
            ...
        }
        else
        {
            // Power domain 2 is off
        }
        ...
     @endverbatim
 * ============================================================================
 */
 PSC_PDSTATE PSC_getPowerDomainState  (
    Uint32                  pwrDmnNum
)
{
    return (PSC_PDSTATE) (hPscRegs->PDSTAT[pwrDmnNum] & PSC_PDSTAT_STATE);
}



/** ============================================================================
 *   @n@b PSC_startStateTransition
 *
 *   @b Description
 *   @n This function sets the 'GO' bit in the PSC Command register. All state
 *      transitions marked for a specified power domain and all its modules are
 *      initiated by the hardware.
 *
 *      This function starts a given Power domain and all its modules state
 *      transition.
 *
 *   @b Arguments
 *   @verbatim
        pwrDmnNum       Power domain number for which the state transition
                        must be initiated.
     @endverbatim
 *
 *   <b> Return Value </b>  None
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *   @n  Power domain and modules are moved to a new "Next" state as marked
 *       earlier using APIs: @a PSC_setModuleNextState  (),
 *       @a PSC_enablePowerDomain (), @a PSC_disablePowerDomain ().
 *
 *   @b  Writes
 *   @n  PSC_PTCMD
 *
 *   @b  Example
 *   @verbatim
        ...
        // To Enable Power domain 2 and modules 3, 4
        PSC_enablePowerDomain (2);
        PSC_setModuleNextState  (3, PSC_MODSTATE_ENABLE);
        PSC_setModuleNextState (4, PSC_MODSTATE_ENABLE);
        PSC_startStateTransition (2);
        ...
     @endverbatim
 * ============================================================================
 */
void PSC_startStateTransition (
    Uint32                  pwrDmnNum
)
{
    hPscRegs->PTCMD =   (1 << pwrDmnNum);

    return;
}

/** ===========================================================================
 *   @n@b PSC_isStateTransitionDone
 *
 *   @b Description
 *   @n This function gets the transition status of the power domain. This function
 *      returns 0 to indicate that the state transition initiated earlier using
 *      @a PSC_startStateTransition () API for the specified power domain has not
 *      yet been completed, and is in progress still. This function returns 1
 *      to indicate when this transition is completed in the hardware.
 *
 *   @b Arguments
 *   @verbatim
        pwrDmnNum       Power domain number for which the state transition status
                        must be retrieved.
     @endverbatim
 *
 *   <b> Return Value </b>  Uint32
 *
 *   <b> Pre Condition </b>
 *   @n  None
 *
 *   <b> Post Condition </b>
 *    @n Power domain transition status value is read
 *
 *   @b Reads
 *   @n PSC_PTSTAT
 *
 *   @b Example
 *   @verbatim
        ...
        // Ensure no transition in progress for Power domain 2
        // before we start a new one.
        while (!PSC_isStateTransitionDone (2));

        // To Enable Power domain 2 and modules 3, 4
        PSC_enablePowerDomain (2);
        PSC_setModuleNextState (3, PSC_MODSTATE_ENABLE);
        PSC_setModuleNextState (4, PSC_MODSTATE_ENABLE);
        PSC_startStateTransition (2);

        // Wait until the transition process is completed.
        while (!PSC_isStateTransitionDone (2));
        ...
     @endverbatim
 * ============================================================================
 */
Uint32 PSC_isStateTransitionDone (
    Uint32                  pwrDmnNum
)
{
    Uint32  pdTransStatus;

    pdTransStatus =   (hPscRegs->PTSTAT >> pwrDmnNum) & 0x1;

    if (pdTransStatus)
    {
        /* Power domain transition is in progress. Return 0 to indicate not yet done. */
        return 0;
    }
    else
    {
        /* Power domain transition is done. */
        return 1;
    }
}



/*
 * powDomain            Power Domains
 * pdFlag               Power ON/OFF
 * moduleDomain         Clock Domains
 * moduleFlag           Clock ON/OFF
 */
Uint32 PSC_ClkControl(Uint32  powDomain, PSC_PDSTATE pdFlag,Uint32 moduleDomain, PSC_MODSTATE moduleFlag)
{
    while (!PSC_isStateTransitionDone (powDomain));


    if(PSC_PDSTATE_OFF == pdFlag)
   {
       PSC_enablePowerDomain(powDomain);
   }
   else
   {
       PSC_disablePowerDomain(powDomain);
   }

    PSC_setModuleNextState(moduleDomain,moduleFlag);

   PSC_startStateTransition(powDomain);

   while (!PSC_isStateTransitionDone (powDomain));

   return 0;

}

Uint32 PSC_PowerControl(Uint32  powDomain, PSC_PDSTATE pdFlag)
{
    while (!PSC_isStateTransitionDone (powDomain));

    if(PSC_PDSTATE_OFF == pdFlag)
    {
        PSC_enablePowerDomain(powDomain);
    }
    else
    {
        PSC_disablePowerDomain(powDomain);
    }

    PSC_startStateTransition(powDomain);

    while (!PSC_isStateTransitionDone (powDomain));

    return 0;
}



/*****************************END OF FILE*************************************/
