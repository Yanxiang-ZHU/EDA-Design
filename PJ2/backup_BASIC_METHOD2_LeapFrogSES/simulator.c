#include "simulator.h"
#include <stdlib.h>

AEvent a_event_queue[MAX_EVENTS_A];

AEvent generate_next_A(AEvent prev, double T, double Vth) {
    AEvent a;
    a.T = T + prev.dT;
    a.V = (rand() % 1001) / 1000.0;   // a.V = 0.000 ~ 1.000, including boundary
    a.dT = prev.dT;
    a.Vth = Vth;
    
    if ((prev.V <= Vth && a.V > Vth) || (prev.V > Vth && a.V <= Vth)) {
        a.bM = 1;
    }
    else {
        a.bM = 0;
    }
    
    return a;
}

DEvent generate_next_D(DEvent prev, double T) {
    DEvent d;
    d.dT = (101 + rand() % 199) / 1000.0;  // 0.101 ~ 0.299, without boundary
    d.T = T + prev.dT;
    d.V = 1 - prev.V;
    d.bM = 1;
    
    return d;
}