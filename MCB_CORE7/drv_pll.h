/*
 * pll.h
 *
 *  Created on: 2020Äê9ÔÂ3ÈÕ
 *      Author: zhuyb
 */

#ifndef PLL_PLL_H_
#define PLL_PLL_H_


typedef enum
{
	DDR3_TIMING_800MHZ=0,
	DDR3_TIMING_1066MHZ,
	DDR3_TIMING_1333MHZ,
	DDR3_TIMING_1600MHZ
}DDR3Timing;


// +--------------------+---------------+--------+--------+
// | (CLK)Desired       | (CLKIN) Input |        |        |
// | Device Speed (MHz) | Clock (MHz)   | PLL1_M | PLL1_D |
// +--------------------+---------------+--------+--------+
// | 1000               | 100           | 19     | 0      |
// | 1000               | 100 (EVM)     | 39     | 1      |
// | 1250               | 100           | 24     | 0      |
// | 1000               | 50            | 39     | 1      |
// | 1000               | 156.25        | 63     | 4      |
// +--------------------+---------------+--------+--------+

int Init_PLL(int pll_mult, int pll_div);


// +--------------------+---------------+--------+--------+
// | DDR3 PLL VCO       | (CLKIN) Input |        |        |
// | Rate (MHz)         | Clock (MHz)   | PLL1_M | PLL1_D |
// +--------------------+---------------+--------+--------+
// | 1600				| 66.667        | 23     | 0      |
// | 1333               | 66.667 (EVM)  | 19     | 0      |
// | 1066               | 66.667        | 31     | 1      |
// | 800                | 66.667        | 11     | 0      |
// +--------------------+---------------+--------+--------+

void Init_DDR_Pll(unsigned int multiplier, unsigned int divider);


// +--------------------+---------------+--------+--------+
// | PA PLL VCO         | (CLKIN) Input |        |        |
// | Rate (MHz)         | Clock (MHz)   | PLL1_M | PLL1_D |
// +--------------------+---------------+--------+--------+
// | 1050               | 100.00 (EVM)  | 20     | 0      |
// | 1044               | 122.88        | 31     | 1      |
// | 1050               | 122.88        | 204    | 11     |
// | 1050               | 156.25        | 335    | 24     |
// +--------------------+---------------+--------+--------+

void Init_PASS_Pll(unsigned int multiplier,unsigned int divider);

void Init_DDR3_Timing(DDR3Timing Timing);

#endif /* PLL_PLL_H_ */
