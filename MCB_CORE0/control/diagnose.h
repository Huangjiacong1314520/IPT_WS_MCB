/*
 * diagnose.h
 *
 *  Created on: 2022Äê5ÔÂ31ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_DIAGNOSE_H_
#define CONTROL_DIAGNOSE_H_

//typedef enum{
//    SfpChannel1 = 1,
//    SfpChannel2,
//    SfpChannel3,
//    SfpChannel4,
//    SfpChannel5,
//    SfpChannel6,
//    J30J1,
//    J30J2,
//    J30J3,
//    J30J4,
//    J30J5,
//    J30J6,
//    J30J7,
//    J30J8,
//    J30J9,
//    J30J10,
//    J30J11,
//    J30J12,
//
//}Peripheral;

typedef enum{
    Ruleroffline = 1,
    ADoffline,
    DAoffline,
    J30J1,
    J30J2,
    J30J3,
    J30J4,
    J30J5,
    J30J6,
    J30J7,
    J30J8,
    J30J9,
    J30J10,
    J30J11,
    J30J12,
    CalculateTIMEOUT,

    ErrorCount
}SystemError;
#define ERRCODEMASK 0x0003FFFF

unsigned long long int GetErrorCode();
void SetErrorCode(SystemError error);



#endif /* CONTROL_DIAGNOSE_H_ */
