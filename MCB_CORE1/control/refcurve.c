/*
 * refcurve.c
 *
 *  Created on: 2021年8月22日
 *      Author: lkx
 */


#include "refcurve.h"

double RefValueLast[SlotCount][6];
double RefValue[SlotCount][6];
int StateLast[SlotCount];
double SpeedLowConst[SlotCount];
double StayTime[SlotCount];
double SwitchDt[SlotCount];
double TNodes[SlotCount][32];

// 正弦信号生成
double Sine_DatCreator(int count,double freq,double Amp,double phase)
{
    double t = 0.0002*count;
    double y = Amp*sin(2*3.1415*freq*t+phase);
    return y;
}

void VectorSetInt(int* p, int num, int value){
    int i = 0;
    for(i = 0; i < num; i++){
        p[i] = value;
    }
}

// 待修改
void S3_DataCreator(double* realValue, double* value, double* lastValue, double* para,double originPos,double targetPos,unsigned long long count){
    double SampleTime = 0.0002;           //采样时间200us
    unsigned long long int SampleNum = 0; //采样点数
    double Sm = 0;
    double Vm = para[1];
    double Am = para[2];
    double Jm = para[3];
    double t1 = 0;       //时间拐点t1
    double t2 = 0;       //时间拐点t2
    double t3 = 0;       //时间拐点t3
    double t4 = 0;       //时间拐点t4
    double t5 = 0;       //时间拐点t5
    double t6 = 0;       //时间拐点t6
    double t7 = 0;       //时间拐点t7
    double t_total = 0;  //总运行时间
    double t = 0;
    float CurveData = 0;

    if(targetPos>=originPos)
        Sm = targetPos - originPos;
    else
        Sm = originPos - targetPos;

    /*规划S曲线*/
    if((Am*Am/Jm < Vm)  &&          //2V(t1)<Vm，有匀加速段
       (Vm*(Am/Jm+Vm/Am) < Sm) )    //2S(t3)<Sm，有匀速段
    {
        //计算时间拐点
        t1 = Am/Jm;
        t2 = Vm/Am;
        t3 = t1 + t2;
        t4 = Sm/Vm;
        t5 = t4 + t1;
        t6 = t4 + t2;
        t7 = t6 + t1;
        t_total = t7;
        //计算采样点数
        SampleNum = (int)(t_total/SampleTime) + 1;

        //计算轨迹
        if(count > SampleNum-1){
            realValue[0] = targetPos;
            return ;
        }


        if(targetPos < originPos)
            count = SampleNum-1-count ;

        t = (double)count*SampleTime;
        if((t>=0)&&(t<=t1))
            CurveData = (float)(1.0/6.0*Jm*t*t*t);
        else if((t>t1)&&(t<=t2))
            CurveData = (float)(0.5*Am*t*t-0.5*Am*t1*t+1.0/6.0*Jm*t1*t1*t1);
        else if((t>t2)&&(t<=t3))
            CurveData = (float)(-1.0/6.0*Jm*t*t*t+0.5*(Am+Jm*t2)*t*t-(0.5*Jm*t2*t2+0.5*Am*t1)*t+1.0/6.0*Jm*(t1*t1*t1+t2*t2*t2));
        else if((t>t3)&&(t<=t4))
            CurveData =(float)( Vm*t-0.5*Vm*(t1+t2));
        else if((t>t4)&&(t<=t5))
            CurveData = (float)(-1.0/6.0*Jm*t*t*t+0.5*Jm*t4*t*t+(Vm-0.5*Jm*t4*t4)*t+1.0/6.0*Jm*t4*t4*t4-0.5*Vm*(t1+t2));
        else if((t>t5)&&(t<=t6))
            CurveData =(float)( -0.5*Am*t*t+(Am*(t5-0.5*t1)+Vm)*t-1.0/6.0*Jm*t1*t1*t1-0.5*Am*t4*t5-0.5*Vm*(t1+t2));
        else if((t>t6)&&(t<=t7))
            CurveData =(float)(1.0/6.0*Jm*t*t*t-0.5*(Jm*t6+Am)*t*t+(0.5*Jm*t6*t6+Am*(t5-0.5*t1)+Vm)*t-Am*(t5-0.5*t1)*t6-Vm*t2-1.0/6.0*Jm*(t1*t1*t1+t6*t6*t6)+0.5*Am*t6*t6);
        else
            CurveData = (float)Sm;

        if(targetPos>=originPos)
            CurveData += (float)originPos;
        else
            CurveData += (float)targetPos;
    }
    /*规划斜坡曲线*/
    else
    {
        Vm = 0.01;
        t_total = Sm/Vm;                           //按Vm的速度匀速运行
        SampleNum= (int)(t_total/SampleTime)+1;    //计算采样点数

        t = (double)count*SampleTime;
        if((t>=0)&&(t<=t_total))
            if(targetPos>=originPos)
                CurveData = (float)(Vm*t + originPos);
            else
                CurveData = (float)(-Vm*t + originPos);
        else
            CurveData = (float)targetPos;
    }
    //
    realValue[0] = CurveData;
}

// 4阶S曲线生成
void S4_DataCreator(double* realValue, double* value, double* lastValue, double* para, int stateLast,
                    double originPos,double targetPos,unsigned long long count, int* tempindex  ,Slot slot)
{
    double Ts = 0.0002;              // 采样时间200us
    int direction = 1;                       // targetPos > originPos
    double Smax = para[0];                           //
    double Vmax = para[1];
    double Amax = para[2];
    double Jmax = para[3];
    double Cmax = para[4];
    double* SLast = lastValue;
    double* VLast = lastValue+1;
    double* ALast = lastValue+2;
    double* JLast = lastValue+3;
//    double* CLast = lastValue+4;
    int i = 0;
    int index = 1;
    int indexLast = stateLast;
    double dt = 0;
    double t1 = count*Ts;
    double t0 = (count-1)*Ts;
    double CNew = 0;

    if(targetPos>=originPos){
        direction = 1;
    }else{
        direction = 0;
    }

    if(count == 0){
        value[4] = CNew;
        value[3] = 0;
        value[2] = 0;
        value[1] = 0;
        value[0] = 0;
        StateLast[slot] = 1;
        *tempindex = 1;
    }else{
        for(i = 0; i < 16; i++){
            if(TNodes[slot][i] <= t1){
                if(i == 15){
                    index = 16;
                    break;
                }
                if(TNodes[slot][i+1] > t1){
                    index = i + 1;
                    break;
                }
            }
        }

        if(index == 16){
            value[5] = 0;
            value[4] = 0;
            value[3] = 0;
            value[2] = 0;
            value[1] = 0;
            value[0] = Smax;
        }else{
            for(i = 0; i <= index - stateLast; i++){
                if(t1 > TNodes[slot][indexLast])
                   dt =  TNodes[slot][indexLast] - t0;
                else
                    dt = t1 - t0;

                if(indexLast==1||indexLast==7||indexLast==11||indexLast==13){
                    CNew = Cmax;
                }else if(indexLast==3||indexLast==5||indexLast==9||indexLast==15){
                    CNew = -Cmax;
                }else{
                    CNew = 0;
                }

                value[5] = 0;
                value[4] = CNew;
                value[3] = CNew*dt + *JLast;
                value[2] = 1.0/2.0*CNew*Pow(dt, 2) + (*JLast)*dt + *ALast;
                value[1] = 1.0/6.0*CNew*Pow(dt, 3) + 1.0/2.0*(*JLast)*Pow(dt, 2) + (*ALast)*dt + *VLast;
                value[0] = 1.0/24.0*CNew*Pow(dt, 4) + 1.0/6.0*(*JLast)*Pow(dt, 3) + 1.0/2.0*(*ALast)*Pow(dt, 2) + (*VLast)*dt + *SLast;

                DataCpy(lastValue, value, 6);

                t0 = TNodes[slot][indexLast];
                indexLast = indexLast + 1;
            }

            if(index==2||index==14)
                value[3] = Jmax;
            else if(index==6||index==10)
                value[3] = -Jmax;
            else if(index==4||index==8||index==12)
                value[3] = 0;

            if(index==4)
                value[2] = Amax;
            else if(index==12)
                value[2] = -Amax;
            else if(index==8)
                value[2] = 0;

            if(index==8)
                value[1] = Vmax;

            DataCpy(lastValue, value, 6);
        }
        StateLast[slot] = index;
        *tempindex = index;

//        // 上传运动阶段
//         if(index == 1){
//             MultiCoreShareData->CurState = 0;
//         }else if(index == 8){
//             MultiCoreShareData->CurState = 1;
//         }else if(index == 9){
//             MultiCoreShareData->CurState = 2;
//         }else if(index == 16){
//             MultiCoreShareData->CurState = 3;
//         }else{
//             ;
//         }
    }

    if(direction == 0){
        realValue[0] = originPos - value[0];
        realValue[1] = -value[1];
        realValue[2] = -value[2];
        realValue[3] = -value[3];
        realValue[4] = -value[4];
        realValue[5] = -value[5];
    }else{
        realValue[0] = originPos + value[0];
        realValue[1] = value[1];
        realValue[2] = value[2];
        realValue[3] = value[3];
        realValue[4] = value[4];
        realValue[5] = value[5];
    }
}
//
//// 5阶S曲线生成
void S5_DataCreator(double* realValue, double* value, double* lastValue, double* para, int stateLast, double originPos,double targetPos,unsigned long long count,Slot slot){
    double Ts = 0.0002;              // 采样时间200us
    int direction = 1;                       // targetPos > originPos
    double Smax = para[0];                           //
    double Vmax = para[1];
    double Amax = para[2];
    double Jmax = para[3];
    double Cmax = para[4];
    double Pmax = para[5];
    double* SLast = lastValue;
    double* VLast = lastValue+1;
    double* ALast = lastValue+2;
    double* JLast = lastValue+3;
    double* CLast = lastValue+4;
   // double* PLast = lastValue+5;
    int i = 0;
    int index = 1;
    int indexLast = stateLast;
    double dt = 0;
    double t1 = count*Ts;
    double t0 = (count-1)*Ts;
    double PNew = 0;

    if(targetPos>=originPos){
        direction = 1;
    }else{
        direction = 0;
    }

    if(count == 0){
        value[5] = Pmax;
        value[4] = 0;
        value[3] = 0;
        value[2] = 0;
        value[1] = 0;
        value[0] = 0;
        StateLast[slot] = 1;
    }else{
        for(i = 0; i < 32; i++){
            if(TNodes[slot][i] <= t1){
                if(i == 31){
                    index = 32;
                    break;
                }
                if(TNodes[slot][i+1] > t1){
                    index = i + 1;
                    break;
                }
            }
        }

        if(index == 32){
            value[5] = 0;
            value[4] = 0;
            value[3] = 0;
            value[2] = 0;
            value[1] = 0;
            value[0] = Smax;
        }else{
            for(i = 0; i <= index - stateLast; i++){
                if(t1 > TNodes[slot][indexLast])
                   dt =  TNodes[slot][indexLast] - t0;
                else
                    dt = t1 - t0;

                if(indexLast==1||indexLast==7||indexLast==11||indexLast==13||indexLast==19||indexLast==21||indexLast==25||indexLast==31)
                    PNew = Pmax;
                else if(indexLast==3||indexLast==5||indexLast==9||indexLast==15||indexLast==17||indexLast==23||indexLast==27||indexLast==29)
                    PNew = -Pmax;
                else
                    PNew = 0;

                value[5] = PNew;
                value[4] = PNew*dt+(*CLast);
                value[3] = 1.0/2.0*PNew*Pow(dt, 2) + (*CLast)*dt + *JLast;
                value[2] = 1.0/6.0*PNew*Pow(dt, 3) + 1.0/2.0*(*CLast)*Pow(dt, 2) + (*JLast)*dt + *ALast;
                value[1] = 1.0/24.0*PNew*Pow(dt, 4) + 1.0/6.0*(*CLast)*Pow(dt, 3) + 1.0/2.0*(*JLast)*Pow(dt, 2) + (*ALast)*dt + *VLast;
                value[0] = 1.0/120.0*PNew*Pow(dt, 5) + 1.0/24.0*(*CLast)*Pow(dt, 4) + 1.0/6.0*(*JLast)*Pow(dt, 3) + 1.0/2.0*(*ALast)*Pow(dt, 2) + (*VLast)*dt + *SLast;

                DataCpy(lastValue, value, 6);

                t0 = TNodes[slot][indexLast];
                indexLast = indexLast + 1;
            }
            if(index==2||index==14||index==22||index==26)
                value[4] = Cmax;
            else if(index==6||index==10||index==18||index==30)
                value[4] = -Cmax;
            else if(index==4||index==8||index==12||index==16||index==20||index==24||index==28||index==32)
                value[4] = 0;

            if(index==4||index==28)
                value[3] = Jmax;
            else if(index==12||index==20)
                value[3] = -Jmax;
            else if(index==8||index==16||index==24||index==32)
                value[3] = 0;

            if(index==8)
                value[2] = Amax;
            else if(index==24)
                value[2] = -Amax;
            else if(index==16||index==32)
                value[2] = 0;

            if(index==16)
                value[1] = Vmax;

            DataCpy(lastValue, value, 6);

        }
        StateLast[slot] = index;
    }

    if(direction == 0){
        realValue[0] = originPos - value[0];
        realValue[1] = -value[1];
        realValue[2] = -value[2];
        realValue[3] = -value[3];
        realValue[4] = -value[4];
        realValue[5] = -value[5];
    }else{
        realValue[0] = originPos + value[0];
        realValue[1] = value[1];
        realValue[2] = value[2];
        realValue[3] = value[3];
        realValue[4] = value[4];
        realValue[5] = value[5];
    }
}

// S曲线无法生成时，使用斜坡信号
void RampCreator(double* realValue, double* value, double originPos, double targetPos, double speed, unsigned long long count){
    double sampleTime = 0.0002;
    double Sm;
    int direction = 0;
    unsigned long long waveNum;
    double t = count*sampleTime;

    if(targetPos>=originPos){
        Sm = targetPos - originPos;
        direction = 1;
    }else{
        Sm = originPos - targetPos;
        direction = 0;
    }

    waveNum = (int)(Sm/speed/sampleTime) + 1;
    VectorSet(value, 6, 0);
    if(speed*t <= Sm){
        value[0] = speed * t;
        value[1] = speed;
    }else{
        value[0] = Sm;
    }

    //
    if(direction == 0){
        realValue[0] = originPos - value[0];
        realValue[1] = -value[1];
        realValue[2] = -value[2];
        realValue[3] = -value[3];
        realValue[4] = -value[4];
        realValue[5] = -value[5];
    }else{
        realValue[0] = originPos + value[0];
        realValue[1] = value[1];
        realValue[2] = value[2];
        realValue[3] = value[3];
        realValue[4] = value[4];
        realValue[5] = value[5];
    }
}

// 生成参考轨迹
void RefCurveCreator(int order, double *waveNum, double* _realValue, double* _para, double originPos, double targetPos, int times, unsigned long long int count,int *index,Slot slot)
{
    double waveNumSingle;
    double* realValue = (double*)_realValue;
    double* value = (double*)RefValue[slot];
    double* lastValue = (double*)RefValueLast[slot];
    double* para = (double*)_para;
    double speedLow = SpeedLowConst[slot];
    para[0] = Abs(originPos - targetPos);
    if(order == 5){
        if(S5CheckData(para, &waveNumSingle,slot) == 1){
            // 如果S曲线可以生成
            waveNumSingle += (int)(StayTime[slot]/0.0002);
            *waveNum = times*waveNumSingle;
            // 开始下一段S曲线时，清空轨迹数据
            if(count % (int)waveNumSingle == 0){
                VectorSet((double *)&RefValueLast[slot], 6, 0);
                VectorSet((double *)&RefValue[slot], 6, 0);
                StateLast[slot] = 0;
            }
            if(count < *waveNum){
                if((count/(unsigned long long)waveNumSingle)%2 == 0)
                    S5_DataCreator(realValue, value, lastValue, para, StateLast[slot], originPos, targetPos, count%(int)waveNumSingle,slot);
                else
                    S5_DataCreator(realValue, value, lastValue, para, StateLast[slot], targetPos, originPos, count%(int)waveNumSingle,slot);
            }
            else{
                if(times%2 == 1){
                    realValue[0] = targetPos;
                }else{
                    realValue[0] = originPos;
                }
            }
        }else{
            waveNumSingle = (int)(Abs(originPos-targetPos)/speedLow/0.0002) + 1;
            waveNumSingle += (int)(StayTime[slot]/0.0002);
            *waveNum = times*waveNumSingle;
            //
            if(count < *waveNum){
                if((count/(unsigned long long)waveNumSingle)%2 == 0)
                    RampCreator(realValue, value, originPos, targetPos, speedLow, count%(int)waveNumSingle);
                else
                    RampCreator(realValue, value, targetPos, originPos, speedLow, count%(int)waveNumSingle);
            }
            else{
                if(times%2 == 1){
                    realValue[0] = targetPos;
                }else{
                    realValue[0] = originPos;
                }
            }
        }
    }else if(order == 4){
        if(S4CheckData(para, &waveNumSingle,slot) == 1){
            // 如果S曲线可以生成
            waveNumSingle += (int)(StayTime[slot]/0.0002);
            *waveNum = times*waveNumSingle;
            // 开始下一段S曲线时，清空轨迹数据
            if(count % (int)waveNumSingle == 0){
                VectorSet((double *)&RefValueLast[slot], 6, 0);
                VectorSet((double *)&RefValue[slot], 6, 0);
                StateLast[slot] = 0;
            }
            if(count < *waveNum){
                if((count/(unsigned long long)waveNumSingle)%2 == 0)
                    S4_DataCreator(realValue, value, lastValue, para, StateLast[slot], originPos, targetPos, count%(int)waveNumSingle,index,slot);
                else
                    S4_DataCreator(realValue, value, lastValue, para, StateLast[slot], targetPos, originPos, count%(int)waveNumSingle,index,slot);
            }
            else{
                StateLast[slot] = 16;
                *index = 16;
                if(times%2 == 1){
                    realValue[0] = targetPos;
                }else{
                    realValue[0] = originPos;
                }
            }
        }else{
            waveNumSingle = (int)(Abs(originPos-targetPos)/speedLow/0.0002) + 1;
            waveNumSingle += (int)(StayTime[slot]/0.0002);
            *waveNum = times*waveNumSingle;
            //
            if(count <= *waveNum){
                if((count/(unsigned long long)waveNumSingle)%2 == 0)
                    RampCreator(realValue, value, originPos, targetPos, speedLow, count%(int)waveNumSingle);
                else
                    RampCreator(realValue, value, targetPos, originPos, speedLow, count%(int)waveNumSingle);
            }
            else{
                if(times%2 == 1){
                    realValue[0] = targetPos;
                }else{
                    realValue[0] = originPos;
                }
            }

        }


    }
}

// 数组内容拷贝
void DataCpy(double* dst, double* src, int len){
    int i = 0;

    for(i = 0; i < len; i++){
        dst[i] = src[i];
    }
}

// 曲线参数初始化
void CurveValueSetAll(){
//    int i = 0;
//    int index = 0;
    //
//    StayTime = 0;
//    SwitchDt = 0;
    VectorSet(StayTime, SlotCount, 0);
    VectorSet(SpeedLowConst, SlotCount, 0.001);
    VectorSet(StayTime, SlotCount, 0);
    VectorSet(SwitchDt, SlotCount, 0);
    Vector2Set((double *)TNodes, SlotCount, 32, 0);
    Vector2Set((double *)RefValue, SlotCount, 6, 0);
    Vector2Set((double *)RefValueLast, SlotCount, 6, 0);
    VectorSetInt(StateLast, SlotCount, 0);
    // 可用于计算核独立初始化S曲线参数
}

// 计算绝对值
double Abs(double a){
    if(a >= 0)
        return a;
    else
        return -a;

}

double Pow(double v, int n){
    int i = 0;
    double res = 1;

    for(i = 0; i < n; i++){
        res = res * v;
    }

    return res;
}


int Round(double v){
    if(2*v > 2*(int)v+1){
        return (int)v+1;
    }else{
        return (int)v;
    }
}

int S4CheckData(double* para, double* waveNum,Slot slot){
    double Smax = para[0];
    double Vmax = para[1];
    double Amax = para[2];
    double Jmax = para[3];
    double Cmax = para[4];
    double t1 = Jmax/Cmax;
    double t2 = (Amax - t1*Jmax)/Jmax;
    double t3 = (Vmax - (2*t1+t2)*Amax)/Amax;
    double t4 = (Smax - (4*t1+2*t2+t3)*Vmax)/Vmax;
   // int flag = 0;

    if(t1 <= 0 || t2 < 0 || t3 < 0 || t4 < 0)
       return  0;
    else{
        TNodes[slot][0] = 0;
        TNodes[slot][1] = TNodes[slot][0] + t1;
        TNodes[slot][2] = TNodes[slot][1] + t2;
        TNodes[slot][3] = TNodes[slot][2] + t1;
        TNodes[slot][4] = TNodes[slot][3] + t3;
        TNodes[slot][5] = TNodes[slot][4] + t1;
        TNodes[slot][6] = TNodes[slot][5] + t2;
        TNodes[slot][7] = TNodes[slot][6] + t1;
        TNodes[slot][8] = TNodes[slot][7] + t4;
        TNodes[slot][9] = TNodes[slot][8] + t1;
        TNodes[slot][10] = TNodes[slot][9] + t2;
        TNodes[slot][11] = TNodes[slot][10] + t1;
        TNodes[slot][12] = TNodes[slot][11] + t3;
        TNodes[slot][13] = TNodes[slot][12] + t1;
        TNodes[slot][14] = TNodes[slot][13] + t2;
        TNodes[slot][15] = TNodes[slot][14] + t1;

        *waveNum = (int)((TNodes[slot][15]) / 0.0002 + 1);

        return 1;
    }
}

// 判断S曲线是否能正确生成
int S5CheckData(double* para, double* waveNum,Slot slot){
    double Sm = para[0];
    double Vm = para[1];
    double Am = para[2];
    double Jm = para[3];
    double Cm = para[4];
    double Pm = para[5];
    double t1 = Cm/Pm;
    double t2 = Jm/Cm - t1;
    double t3 = Am/Jm - 2*t1 - t2;
    double t4 = Vm/Am - 4*t1 -2*t2 - t3;
    double t5 = Sm/Vm - 8*t1 - 4*t2 - 2*t3 - t4;
  //  int flag = 0;

    if(t1 <= 0 || t2 <= 0 || t3 <= 0 || t4 <= 0 || t5 <= 0)
       return  0;
    else{
        TNodes[slot][0] = 0;
        TNodes[slot][1] = t1;
        TNodes[slot][2] = TNodes[slot][1]+t2;
        TNodes[slot][3] = TNodes[slot][2]+t1;
        TNodes[slot][4] = TNodes[slot][3]+t3;
        TNodes[slot][5] = TNodes[slot][4]+t1;
        TNodes[slot][6] = TNodes[slot][5]+t2;
        TNodes[slot][7] = TNodes[slot][6]+t1;
        TNodes[slot][8] = TNodes[slot][7]+t4;
        TNodes[slot][9] = TNodes[slot][8]+t1;
        TNodes[slot][10] = TNodes[slot][9]+t2;
        TNodes[slot][11] = TNodes[slot][10]+t1;
        TNodes[slot][12] = TNodes[slot][11]+t3;
        TNodes[slot][13] = TNodes[slot][12]+t1;
        TNodes[slot][14] = TNodes[slot][13]+t2;
        TNodes[slot][15] = TNodes[slot][14]+t1;
        TNodes[slot][16] = TNodes[slot][15]+t5;
        TNodes[slot][17] = TNodes[slot][16]+t1;
        TNodes[slot][18] = TNodes[slot][17]+t2;
        TNodes[slot][19] = TNodes[slot][18]+t1;
        TNodes[slot][20] = TNodes[slot][19]+t3;
        TNodes[slot][21] = TNodes[slot][20]+t1;
        TNodes[slot][22] = TNodes[slot][21]+t2;
        TNodes[slot][23] = TNodes[slot][22]+t1;
        TNodes[slot][24] = TNodes[slot][23]+t4;
        TNodes[slot][25] = TNodes[slot][24]+t1;
        TNodes[slot][26] = TNodes[slot][25]+t2;
        TNodes[slot][27] = TNodes[slot][26]+t1;
        TNodes[slot][28] = TNodes[slot][27]+t3;
        TNodes[slot][29] = TNodes[slot][28]+t1;
        TNodes[slot][30] = TNodes[slot][29]+t2;
        TNodes[slot][31] = TNodes[slot][30]+t1;

        *waveNum = (int)((TNodes[slot][31]) / 0.0002 + 1);

        return 1;
    }
}
