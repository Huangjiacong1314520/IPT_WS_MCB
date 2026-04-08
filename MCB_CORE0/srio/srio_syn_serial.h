#ifndef   __SRIO_AD_H_
#define  __SRIO_AD_H_


/****************************************************************************/
/*                                                                          */
/*              AD卡用户寄存器列表                                                 */
/*                                                                          */
/****************************************************************************/
#if  0
#define  SRIO_AD_SYS_VERTIME    0x20
#define  SRIO_AD_SYS_VERSION    0x21
#define  SRIO_AD_SAMPLE_MODE  0x24
#define  SRIO_AD_SAMPLE_RATE    0x25
#define  SRIO_AD_SPI_BAUD            0x26
#define  SRIO_AD_SYNC             0x27
//ADC寄存器,  0~F,     具体参考AD4110-1芯片手册中的寄存器说明, P66
#define  SRIO_ADC_REG_BEGIN
#define  SRIO_ADC_REG_STATUS          0x0
#define  SRIO_ADC_REG_MODE             0x1
#define  SRIO_ADC_REG_INTERFACE    0x2
#define  SRIO_ADC_REG_CONFIG          0x3
#define  SRIO_ADC_REG_DATA              0x4
#define  SRIO_ADC_REG_FILTER            0x5
#define  SRIO_ADC_REG_GPIO_CONFIG  0x6
#define  SRIO_ADC_REG_ID                        0x7
#define  SRIO_ADC_REG_OFFSET0            0x8
#define  SRIO_ADC_REG_OFFSET1            0x9
#define  SRIO_ADC_REG_OFFSET2            0xA
#define  SRIO_ADC_REG_OFFSET3            0xB
#define  SRIO_ADC_REG_GAIN0                0xC
#define  SRIO_ADC_REG_GAIN1                0xD
#define  SRIO_ADC_REG_GAIN2                0xE
#define  SRIO_ADC_REG_GAIN3                0xF
#define  SRIO_ADC_REG_END

//AFE寄存器,  0x10 ~ 0x1F,  具体参考AD4110-1芯片手册中的寄存器说明, P58
#define  SRIO_AFE_REG_BEGIN
#define  SRIO_AFE_REG_TOP_STATUS       0x0
#define  SRIO_AFE_REG_CNTRL1                 0x1
#define  SRIO_AFE_REG_RESERVE_2           0x2
#define  SRIO_AFE_REG_CLK_CTRL              0x3
#define  SRIO_AFE_REG_CNTRL2                 0x4
#define  SRIO_AFE_REG_PGA_RTD_CTRL   0x5
#define  SRIO_AFE_REG_ERR_DISABLE       0x6
#define  SRIO_AFE_REG_DETAIL_STATUS  0x7
#define  SRIO_AFE_REG_RESERVE_8           0x8
#define  SRIO_AFE_REG_RESERVE_9           0x9
#define  SRIO_AFE_REG_RESERVE_A          0xA
#define  SRIO_AFE_REG_RESERVE_B           0xB
#define  SRIO_AFE_REG_CAL_DATA            0xC
#define  SRIO_AFE_REG_RSENSE_DATA    0xD
#define  SRIO_AFE_REG_NO_PWR_DEFAULT_SEL           0xE
#define  SRIO_AFE_REG_NO_PWR_DEFAULT_STATUS   0xF

#define   SRIO_AFE_REG_END

#endif


/****************************************************************************/
/*                                                                          */
/*              光栅卡用户寄存器列表                                                 */
/*                                                                          */
/****************************************************************************/
#define   SRIO_RASTER_SYS_VERTIME    0x20
#define   SRIO_RASTER_SYS_VERSION    0x21
#define   SRIO_OG_MODE         0x30
#define   SRIO_OG_READ           0x31


#endif
