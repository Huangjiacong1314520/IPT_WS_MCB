/*
 * refcurve.c
 *
 *  Created on: 2021年8月22日
 *      Author: lkx
 */


#include "refcurve.h"

double RefValueLast[6];
double RefValue[6];
int StateLast;
double SpeedLowConst;
double StayTime;
double SwitchDt;

// 正弦信号生成
double Sine_DatCreator(int count,double freq,double Amp,double phase)
{
    double t = 0.0002*count;
    double y = Amp*sin(2*3.1415*freq*t+phase);
    return y;
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

// 4阶S曲线生成-未完成
void S4_DataCreator(double* realValue, double* value, double* lastValue, double* para, int* stateLast, double originPos,double targetPos,unsigned long long count){
    double sampleTime = 0.0002;              // 采样时间200us
    unsigned long long SampleNum = 0;       // 采样点数
    int direction = 1;                       // targetPos > originPos
    double Sm = 0;
    double Vm = para[0];
    double Am = para[1];
    double Jm = para[2];
    double Cm = para[3];
    double SLast = lastValue[0];
    double VLast = lastValue[1];
    double ALast = lastValue[2];
    double JLast = lastValue[3];
    double CLast = lastValue[4];
    double t1 = 0;
    double t2 = 0;
    double t3 = 0;
    double t4 = 0;
    double nodes[16];
    double t = 0;
    int i = 0;
    double dt = 0;
    double C = 0;

    if(targetPos>=originPos){
        Sm = targetPos - originPos;
        direction = 1;
    }else{
        Sm = originPos - targetPos;
        direction = 0;
    }

    t1 = Jm / Cm;
    t2 = Am / Jm - t1;
    t3 = Vm / Am - 2*t1 - t2;
    t4 = Sm / Vm - 4*t1 - 2*t2 - t3;

    S4GetNodes(t1, t2, t3, t4, nodes);

    t = count*sampleTime;
    for(i = 0; i < 15; i++){
        if(t > nodes[i] && t <= nodes[i+1]){
            break;
        }
    }
    i = i + 1;
    if(i != *stateLast){
        DataCpy(lastValue, value, 6);
        *stateLast = i;
    }
    dt = t - nodes[i-1];

    if(i==1||i==7||i==11||i==13){
        C = Cm;
    }else if(i==2||i==4||i==6||i== 8||i==10||i==12||i==14||i==16){
        C = 0;
    }else if(i==3||i==5||i==9||i==15){
        C = -Cm;
    }
    value[4] = C;
    value[3] = C*dt + JLast;
    value[2] = 1/2*C*Pow(dt, 2) + JLast*dt + ALast;
    value[1] = 1/6*C*Pow(dt, 3) + 1/2*JLast*Pow(dt, 2) + ALast*dt + VLast;
    value[0] = 1/24*C*Pow(dt, 4) + 1/6*JLast*Pow(dt, 3) + 1/2*ALast*Pow(dt, 2) + VLast*dt + SLast;

    if(i==2||i==14){
        value[3] = Jm;
    }else if(i==6||i==10){
        value[3] = -Jm;
    }else if(i==4||i==8||i==12||i==16){
        value[3] = 0;
    }

    if(i==4){
        value[2] = Am;
    }else if(i==12){
        value[2] = -Am;
    }else if(i==8||i==16){
        value[2] = 0;
    }

    if(i==8){
        value[1] = Vm;
    }else if(i==16){
        value[1] = 0;
    }

    if(i==16){
        value[0] = Sm;
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

// 5阶S曲线生成
void S5_DataCreator(double* realValue, double* value, double* lastValue, double* para, int* stateLast, double* switchDt, double originPos,double targetPos,unsigned long long count){
    double Ts = 0.0002;              // 采样时间200us
    int direction = 1;                       // targetPos > originPos
    double Sm = 0;                           //
    double Vm = para[1];
    double Am = para[2];
    double Jm = para[3];
    double Cm = para[4];
    double Pm = para[5];
    double* SLast = lastValue;
    double* VLast = lastValue+1;
    double* ALast = lastValue+2;
    double* JLast = lastValue+3;
    double* CLast = lastValue+4;
    double* PLast = lastValue+5;
    double t1 = 0;
    double t2 = 0;
    double t3 = 0;
    double t4 = 0;
    double t5 = 0;
    double nodes[32];
    int i = 0;
    double dt = 0;
    double P = 0;
    int switchFlag = 0;
    double t = count*Ts;

    if(targetPos>=originPos){
        Sm = targetPos - originPos;
        direction = 1;
    }else{
        Sm = originPos - targetPos;
        direction = 0;
    }

    t1 = Cm / Pm;
    t2 = Jm / Cm - t1;
    t3 = Am / Jm - 2*t1 - t2;
    t4 = Vm / Am - 4*t1 - 2*t2 - t3;
    t5 = Sm / Vm - 8*t1 - 4*t2 - 2*t3 - t4;

    S5GetNodes(t1, t2, t3, t4, t5, nodes);

    if(count == 0){
        i = 1;
    }else{
        for(i = 0; i < 31; i++){
            if(t > nodes[i] && t <= nodes[i+1]){
                break;
            }
        }
        i = i + 1;
    }

    if(i == 32){
        value[5] = 0;
        value[4] = 0;
        value[3] = 0;
        value[2] = 0;
        value[1] = 0;
        value[0] = Sm;
    }else{
        if(i != *stateLast){
            DataCpy(lastValue, value, 6);
            *stateLast = i;
            *switchDt = t - nodes[i-1];
            switchFlag = 1;
        }
        dt = t - nodes[i-1] - *switchDt;

        if(i==1||i==7||i==11||i==13||i==19||i==21||i==25||i==31){
            P = Pm;
        }else if(i==3||i==5||i==9||i==15||i==17||i==23||i==27||i==29){
            P = -Pm;
        }else{
            P = 0;
        }
        if(switchFlag == 0){
            value[5] = P;
            value[4] = P*dt+(*CLast);
            value[3] = 1.0/2*P*Pow(dt, 2) + (*CLast)*dt + (*JLast);
            value[2] = 1.0/6*P*Pow(dt, 3) + 1.0/2*(*CLast)*Pow(dt, 2) + (*JLast)*dt + (*ALast);
            value[1] = 1.0/24*P*Pow(dt, 4) + 1.0/6*(*CLast)*Pow(dt, 3) + 1.0/2*(*JLast)*Pow(dt, 2) + (*ALast)*dt + (*VLast);
            value[0] = 1.0/120*P*Pow(dt, 5) + 1.0/24*(*CLast)*Pow(dt, 4) + 1.0/6*(*JLast)*Pow(dt, 3) + 1.0/2*(*ALast)*Pow(dt, 2) + (*VLast)*dt + (*SLast);
        }else{
            dt = Ts - *switchDt;
            value[5] = *PLast;
            value[4] = (*PLast)*dt+(*CLast);
            value[3] = 1.0/2*(*PLast)*Pow(dt, 2) + (*CLast)*dt + (*JLast);
            value[2] = 1.0/6*(*PLast)*Pow(dt, 3) + 1.0/2*(*CLast)*Pow(dt, 2) + (*JLast)*dt + (*ALast);
            value[1] = 1.0/24*(*PLast)*Pow(dt, 4) + 1.0/6*(*CLast)*Pow(dt, 3) + 1.0/2*(*JLast)*Pow(dt, 2) + (*ALast)*dt + (*VLast);
            value[0] = 1.0/120*(*PLast)*Pow(dt, 5) + 1.0/24*(*CLast)*Pow(dt, 4) + 1.0/6*(*JLast)*Pow(dt, 3) + 1.0/2*(*ALast)*Pow(dt, 2) + (*VLast)*dt + (*SLast);
            dt = *switchDt;
            DataCpy(lastValue, value, 6);
            value[5] = P;
            value[4] = P*dt+(*CLast);
            value[3] = 1.0/2*P*Pow(dt, 2) + (*CLast)*dt + (*JLast);
            value[2] = 1.0/6*P*Pow(dt, 3) + 1.0/2*(*CLast)*Pow(dt, 2) + (*JLast)*dt + (*ALast);
            value[1] = 1.0/24*P*Pow(dt, 4) + 1.0/6*(*CLast)*Pow(dt, 3) + 1.0/2*(*JLast)*Pow(dt, 2) + (*ALast)*dt + (*VLast);
            value[0] = 1.0/120*P*Pow(dt, 5) + 1.0/24*(*CLast)*Pow(dt, 4) + 1.0/6*(*JLast)*Pow(dt, 3) + 1.0/2*(*ALast)*Pow(dt, 2) + (*VLast)*dt + (*SLast);
            DataCpy(lastValue, value, 6);
        }
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
void RefCurveCreator(double *waveNum, double* _realValue, double* _para, double originPos, double targetPos, int times, unsigned long long int count)
{
    double waveNumSingle;
    double* realValue = (double*)_realValue;
    double* value = (double*)RefValue;
    double* lastValue = (double*)RefValueLast;
    double* para = (double*)_para;
    double speedLow = SpeedLowConst;

    if(S5Judge(para, Abs(originPos - targetPos), &waveNumSingle) == 1){
        // 如果S曲线可以生成
        waveNumSingle += (int)(StayTime/0.0002);
        *waveNum = times*waveNumSingle;
        // 开始下一段S曲线时，清空轨迹数据
        if(count % (int)waveNumSingle == 0){
            VectorSet(RefValueLast, 6, 0);
            VectorSet(RefValue, 6, 0);
        }
        if(count < *waveNum){
            if((count/(unsigned long long)waveNumSingle)%2 == 0)
                S5_DataCreator(realValue, value, lastValue, para, &StateLast, &SwitchDt, originPos, targetPos, count%(int)waveNumSingle);
            else
                S5_DataCreator(realValue, value, lastValue, para, &StateLast, &SwitchDt, targetPos, originPos, count%(int)waveNumSingle);
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
        waveNumSingle += (int)(StayTime/0.0002);
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
    int i = 0;
    int index = 0;
    //
    StayTime = 0;
    SwitchDt = 0;
    VectorSet(RefValueLast, 6, 0);
    VectorSet(RefValue, 6, 0);

    StateLast = 0;
    SpeedLowConst = 0.01;


    // 可用于计算核独立初始化S曲线参数
}


//// 自定义
//void pow(double data, int n){
//    int i = 0;
//    double result = 1;
//
//    for(i = 0; i < n; i++){
//        result = result * data;
//    }
//
//    return result;
//}

// 判断S曲线是否能正确生成
int S5Judge(double* para, double Smax, double* waveNum){
    double Sm = Smax;
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
    double J, A, V, S;
    double a = 0;
    int flag = 0;

    if(t1>0&&t2>0&&t3>0&&t4>0&&t5>0){
        S = Pm*64*Pow(t1, 5);
        if(S<Sm){
            a = 2*t1 + t2;
            S = Jm*8*Pow(a, 3);
            if(S<Sm){
                a = 4*t1+2*t2+t3;
                S = Am*2*Pow(a, 2);
                if(S<Sm){
                    flag = 1;
                }else{
                    flag = 0;
                }
            }else{
                flag = 0;
            }
        }else{
            flag = 0;
        }
    }else{
        flag = 0;
    }
    // 计算点数
    *waveNum = (int)((16*t1+8*t2+4*t3+2*t4+t5) / 0.0002 + 1);

    return flag;
}

// 获取分段时间点
void S5GetNodes(double t1, double t2, double t3, double t4, double t5, double* nodes){
    nodes[0] = 0;
    nodes[1] = t1;
    nodes[2] = nodes[1]+t2;
    nodes[3] = nodes[2]+t1;
    nodes[4] = nodes[3]+t3;
    nodes[5] = nodes[4]+t1;
    nodes[6] = nodes[5]+t2;
    nodes[7] = nodes[6]+t1;
    nodes[8] = nodes[7]+t4;
    nodes[9] = nodes[8]+t1;
    nodes[10] = nodes[9]+t2;
    nodes[11] = nodes[10]+t1;
    nodes[12] = nodes[11]+t3;
    nodes[13] = nodes[12]+t1;
    nodes[14] = nodes[13]+t2;
    nodes[15] = nodes[14]+t1;
    nodes[16] = nodes[15]+t5;
    nodes[17] = nodes[16]+t1;
    nodes[18] = nodes[17]+t2;
    nodes[19] = nodes[18]+t1;
    nodes[20] = nodes[19]+t3;
    nodes[21] = nodes[20]+t1;
    nodes[22] = nodes[21]+t2;
    nodes[23] = nodes[22]+t1;
    nodes[24] = nodes[23]+t4;
    nodes[25] = nodes[24]+t1;
    nodes[26] = nodes[25]+t2;
    nodes[27] = nodes[26]+t1;
    nodes[28] = nodes[27]+t3;
    nodes[29] = nodes[28]+t1;
    nodes[30] = nodes[29]+t2;
    nodes[31] = nodes[30]+t1;
}

void S4GetNodes(double t1, double t2, double t3, double t4, double* nodes){
    /*
    nodes = {0, t1, t1+t2, 2*t1+t2, 2*t1+t2+t3, 3*t1+t2+t3, 3*t1+2*t2+t3, 4*t1+2*t2+t3,
                            4*t1+2*t2+t3+t4, 5*t1+2*t2+t3+t4, 5*t1+3*t2+t3+t4, 6*t1+3*t2+t3+t4, 6*t1+3*t2+2*t3+t4,
                            7*t1+3*t2+2*t3+t4, 7*t1+4*t2+2*t3+t4, 8*t1+4*t2+2*t3+t4};
                            */
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





// 控制量输出保护
void ControllerOutProtect(double* posErr, double* FOut){
//    //误差限幅
//    if(Abs(posErr[AXIS_X])>0.01||Abs(posErr[AXIS_Y])>0.01||Abs(posErr[AXIS_Rz])>0.001){
//        FOut[MOTOR_X1] = 0;
//        FOut[MOTOR_X2] = 0;
//        FOut[MOTOR_Y] = 0;
//    }
//    //输出限幅
//    if(Abs(FOut[MOTOR_X1])>6553.5*2||Abs(FOut[MOTOR_X2])>6553.5*2||Abs(FOut[MOTOR_Y])>6553.5*2){
//    //if(Abs(FOut[MOTOR_Y])>6553.5*1){
//        FOut[MOTOR_X1] = 0;
//        FOut[MOTOR_X2] = 0;
//        FOut[MOTOR_Y] = 0;
//    }
//    //输出限幅
//    if(Abs(FOut[MOTOR_Z1])>6553.5*8||Abs(FOut[MOTOR_Z2])>6553.5*8||Abs(FOut[MOTOR_Z3])>6553.5*8){
//        FOut[MOTOR_Z1] = 0;
//        FOut[MOTOR_Z2] = 0;
//        FOut[MOTOR_Z3] = 0;
//    }
}
