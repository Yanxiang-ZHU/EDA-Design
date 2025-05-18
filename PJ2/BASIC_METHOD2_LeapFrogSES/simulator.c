#include "simulator.h"
#include <stdlib.h>

// 全局变量定义
AEvent a_event_queue[MAX_EVENTS_A];

// 生成min到max之间的随机双精度浮点数
double random_double(double min, double max) {
    return min + (rand() / (double)RAND_MAX) * (max - min);
}

// 生成下一个A事件
AEvent generate_next_A(AEvent prev, double T, double Vth) {
    AEvent a;
    a.T = T + prev.dT;
    a.V = random_double(0, 1);
    a.dT = prev.dT;
    a.Vth = Vth;
    
    // 判断是否跨过阈值Vth
    if ((prev.V <= Vth && a.V > Vth) || (prev.V > Vth && a.V <= Vth)) {  // 跨过Vth
        a.bM = 1;
    }
    else {
        a.bM = 0;
    }
    
    return a;
}

// 生成下一个D事件
DEvent generate_next_D(DEvent prev, double T) {
    DEvent d;
    d.dT = random_double(0.1, 0.3);  // 0.100 ~ 0.299
    d.T = T + prev.dT;
    d.V = 1 - prev.V;    // 翻转V值
    d.bM = 1;
    
    return d;
}