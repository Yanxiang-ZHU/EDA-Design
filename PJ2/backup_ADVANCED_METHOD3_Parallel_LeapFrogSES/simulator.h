#ifndef SIMULATOR_H
#define SIMULATOR_H

#define MAX_EVENTS_A 1000

typedef struct {
    double T;
    double V;
    double dT;
    double Vth;
    int bM;
} AEvent;

typedef struct {
    double T;
    int V;
    double dT;
    int bM;
} DEvent;

extern AEvent a_event_queue[MAX_EVENTS_A];

AEvent generate_next_A(AEvent prev, double T, double Vth);
DEvent generate_next_D(DEvent prev, double T);

#endif /* SIMULATOR_H */