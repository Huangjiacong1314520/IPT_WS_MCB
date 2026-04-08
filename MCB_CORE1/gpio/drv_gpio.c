

/* Driver APIs */
#include "../drv_gpio.h"
#include "hw_types.h"
#include "../drv_intc.h"
#define GPIO_BaseAddr 0x02320000



#define GPIO_REVID              (0x0)
#define GPIO_BINTEN             (0x8)
#define GPIO_DIR                (0x10)
#define GPIO_OUT_DATA           (0x14)
#define GPIO_SET_DATA           (0x18)
#define GPIO_CLR_DATA           (0x1C)
#define GPIO_IN_DATA            (0x20)
#define GPIO_SET_RIS_TRIG       (0x24)
#define GPIO_CLR_RIS_TRIG       (0x28)
#define GPIO_SET_FAL_TRIG       (0x2C)
#define GPIO_CLR_FAL_TRIG       (0x30)

/***********************************************************************/
/*              API FUNCTION DEFINITIONS.                              */ 
/***********************************************************************/

/**
 * \brief    This function configures the direction of a pin as input or 
 *           output.
 *
 * \param    GPIO_BaseAddr     The memory address of the GPIO instance being used.
 * \param    pinNumber   The serial number of the GPIO pin.
 *                       The 32 GPIO pins have serial numbers from 1 to 32.
 *                       
 * \param    pinDir      The direction to be set for the pin.
 *                       This can take the values:
 *                       1> GPIO_DIR_INPUT, for configuring the pin as input.
 *                       2> GPIO_DIR_OUTPUT, for configuring the pin as output.
 * 
 * \return   None.
 *
 * \note     Here we write to the DIRn register. Writing a logic 1 configures 
 *           the pin as input and writing logic 0 as output. By default, all
 *           the pins are set as input pins.
 */
void GPIO_DirModeSet( unsigned int pinNumber,
                    unsigned int pinDir)

{
    if(GPIO_DIR_OUTPUT == pinDir)
    {
        HWREG(GPIO_BaseAddr + GPIO_DIR) &= ~(1 << pinNumber);
    }
    else
    {
        HWREG(GPIO_BaseAddr + GPIO_DIR) |= (1 << pinNumber);
    }
}

/**
 * \brief  This function gets the direction of a pin which has been configured
 *         as an input or an output pin.
 * 
 * \param   GPIO_BaseAddr    The memory address of the GPIO instance being used.
 * \param   pinNumber  The serial number of the GPIO pin.
 *                     The 32 GPIO pins have serial numbers from 1 to 32.
 *
 * \return  This returns one of the following two values:
 *          1> GPIO_DIR_INPUT, if the pin is configured as an input pin.
 *          2> GPIO_DIR_OUTPUT, if the pin is configured as an output pin.
 *
 */
unsigned int GPIO_DirModeGet( unsigned int pinNumber)
{
    unsigned int dir = GPIO_DIR_INPUT;

    dir = (HWREG(GPIO_BaseAddr + GPIO_DIR) & (1 << pinNumber));

    return (dir >> pinNumber);
}


/**
 * \brief   This function writes a logic 1 or a logic 0 to the specified pin.
 *
 * \param   GPIO_BaseAddr    The memory address of the GPIO instance being used.
 * \param   pinNumber  The serial number of the GPIO pin.
 *                     The 32 GPIO pins have serial numbers from 1 to 32.
 *
 * \param   bitValue   This signifies whether to write a logic 0 or logic 1 
 *                     to the specified pin.This variable can take any of the 
 *                     following two values:
 *                     1> GPIO_PIN_LOW, which indicates to clear(logic 0) the bit.
 *                     2> GPIO_PIN_HIGH, which indicates to set(logic 1) the bit.
 *
 * \return  None.
 *
 * \note    The pre-requisite to write to any pin is that the pin has to
 *          be configured as an output pin.
 */
void GPIO_PinWrite( unsigned int pinNumber,
                  unsigned int bitValue)
{
    if(GPIO_PIN_LOW == bitValue)
    {
        HWREG(GPIO_BaseAddr + GPIO_CLR_DATA) = (1 << pinNumber);
    }
    else if(GPIO_PIN_HIGH == bitValue)
    {
        HWREG(GPIO_BaseAddr + GPIO_SET_DATA) = (1 << pinNumber);
    }
}

/**
 * \brief    This function reads the value(logic level) of an input or an
 *           output pin.
 * 
 * \param    GPIO_BaseAddr     The memory address of the GPIO instance being used.
 * \param    pinNumber   The serial number of the GPIO pin.
 *                       The 32 GPIO pins have serial numbers from 1 to 32.
 *
 * \return   This returns the value present on the specified pin. This returns
 *           one of the following values:
 *           1> GPIO_PIN_LOW, if the value on the pin is logic 0.
 *           2> GPIO_PIN_HIGH, if the value on the pin is logic 1.
 *
 * \note     Using this function, we can read the values of both input and 
 *           output pins.
 */
int GPIO_PinRead( unsigned int pinNumber)
{
    unsigned int val = 0;

    val = HWREG(GPIO_BaseAddr + GPIO_IN_DATA) & (1 << pinNumber);

    return (val >> pinNumber);
}

/**
 * \brief   This function configures the trigger level type for which an 
 *          interrupt is required to occur.
 * 
 * \param   GPIO_BaseAddr    The memory address of the GPIO instance being used.
 *
 * \param   pinNumber  The serial number of the GPIO pin.
 *                     The 32 GPIO pins have serial numbers from 1 to 32.
 *
 * \param   intType    This specifies the trigger level type. This can take 
 *                     one of the following four values:
 *                     1> GPIO_INT_TYPE_NOEDGE, to not generate any interrupts.
 *                     2> GPIO_INT_TYPE_FALLEDGE, to generate an interrupt on 
 *                        the falling edge of a signal on that pin. 
 *                     3> GPIO_INT_TYPE_RISEDGE, to generate an interrupt on the 
 *                        rising edge of a signal on that pin.
 *                     4> GPIO_INT_TYPE_BOTHEDGE, to generate interrupts on both
 *                        rising and falling edges of a signal on that pin.
 *
 * \return   None.
 *
 * \note     Configuring the trigger level type for generating interrupts is not 
 *           enough for the GPIO module to generate interrupts. The user should 
 *           also enable the interrupt generation capability for the bank to which
 *           the pin belongs to. Use the function GPIOBankIntEnable() to do the same.             
 */
 

void GPIO_IntTypeSet( unsigned int pinNumber,
                    unsigned int intType)
{
    switch (intType)
    {
        case GPIO_INT_TYPE_RISEDGE:
            /* Setting Rising edge and clearing Falling edge trigger levels.*/
            HWREG(GPIO_BaseAddr + GPIO_SET_RIS_TRIG) = (1 << pinNumber);
            HWREG(GPIO_BaseAddr + GPIO_CLR_FAL_TRIG) = (1 << pinNumber);
            break;

        case GPIO_INT_TYPE_FALLEDGE:
            /* Setting Falling edge and clearing Rising edge trigger levels.*/ 
            HWREG(GPIO_BaseAddr + GPIO_SET_FAL_TRIG) = (1 << pinNumber);
            HWREG(GPIO_BaseAddr + GPIO_CLR_RIS_TRIG) = (1 << pinNumber);
            break;

        case GPIO_INT_TYPE_BOTHEDGE:
            /* Setting both Rising and Falling edge trigger levels.*/
            HWREG(GPIO_BaseAddr + GPIO_SET_RIS_TRIG) = (1 << pinNumber);
            HWREG(GPIO_BaseAddr + GPIO_SET_FAL_TRIG) = (1 << pinNumber);
            break;

        case GPIO_INT_TYPE_NOEDGE:
            /* Clearing both Rising and Falling edge trigger levels. */
            HWREG(GPIO_BaseAddr + GPIO_CLR_RIS_TRIG) = (1 << pinNumber);
            HWREG(GPIO_BaseAddr + GPIO_CLR_FAL_TRIG) = (1 << pinNumber);
            break;

        default:
            break;
    }
}

/**
 * \brief   This function reads the trigger level type being set for interrupts
 *          to be generated.
 * 
 * \param   GPIO_BaseAddr    The memory address of the GPIO instance being used.
 *
 * \param   pinNumber  The serial number of the GPIO pin to be accessed.
 *                     The 32 GPIO pins have serial numbers from 1 to 32.
 *
 * \return  This returns a value which indicates the type of trigger level 
 *          type being set. One of the following values is returned.
 *          1> GPIO_INT_TYPE_NOEDGE, indicating no interrupts will be 
 *             generated over the corresponding pin.
 *          2> GPIO_INT_TYPE_FALLEDGE, indicating a falling edge on the 
 *             corresponding pin signifies an interrupt generation.
 *          3> GPIO_INT_TYPE_RISEDGE, indicating a rising edge on the 
 *             corresponding pin signifies an interrupt generation.
 *          4> GPIO_INT_TYPE_BOTHEDGE, indicating both edges on the
 *             corresponding pin signifies an interrupt each being generated.
 *
 */
unsigned int GPIO_IntTypeGet( unsigned int pinNumber)
{
    unsigned int intType = GPIO_INT_TYPE_NOEDGE;

    if ((HWREG(GPIO_BaseAddr + GPIO_SET_FAL_TRIG)) & (1 << pinNumber))
    {
        intType = GPIO_INT_TYPE_FALLEDGE;
    }

    if ((HWREG(GPIO_BaseAddr + GPIO_SET_RIS_TRIG)) & (1 << pinNumber))
    {
        intType |= GPIO_INT_TYPE_RISEDGE;
    }
  
    return intType;    
}

/**
 * \brief   This function enables the interrupt generation capability for the
 *          bank of GPIO pins specified.
 *
 * \param   GPIO_BaseAddr     The memory address of the GPIO instance being used.
 * \param   bankNumber  This is the bank for whose pins interrupt generation 
 *                      capabiility needs to be enabled.
 *                      bankNumber is 0 for bank 0, 1 for bank 1 and so on.
 * \return  None. 
 *
 */

void GPIO_IntcEnable(void)
{
    HWREG(GPIO_BaseAddr + GPIO_BINTEN) |= (1 << 0);
} 

/**
 * \brief   This function disables the interrupt generation capability for the
 *          bank of GPIO pins specified.
 *
 * \param   GPIO_BaseAddr     The memory address of the GPIO instance being used.
 * \param   bankNumber  This is the bank for whose pins interrupt generation
 *                      capability needs to be disabled.
 *                      bankNumber is 0 for bank 0, 1 for bank 1 and so on.
 * \return  None.
 *
 */

void GPIO_IntcDisable(void)
{
    HWREG(GPIO_BaseAddr + GPIO_BINTEN) &= ~(1 << 0);
}




/*****************************END OF FILE*************************************/ 
