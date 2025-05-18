#ifndef SIMULATOR_H
#define SIMULATOR_H

#define MAX_EVENTS_A 10000

// A事件定义
typedef struct {
    double T;    // 事件发生的时间点
    double V;    // 值，范围[0,1]
    double dT;   // 下一个事件的时间间隔
    double Vth;  // 阈值
    int bM;      // 1：产生M事件，-1：由M事件产生，0：无关
} AEvent;

// D事件定义
typedef struct {
    double T;    // 事件发生的时间点
    int V;       // 值，只能为0或1
    double dT;   // 下一个事件的时间间隔
    int bM;      // 1：产生M事件，-1：由M事件产生，0：无关
} DEvent;

// 全局变量
extern AEvent a_event_queue[MAX_EVENTS_A];

// 函数声明
double random_double(double min, double max);
AEvent generate_next_A(AEvent prev, double T, double Vth);
DEvent generate_next_D(DEvent prev, double T);

#endif /* SIMULATOR_H */