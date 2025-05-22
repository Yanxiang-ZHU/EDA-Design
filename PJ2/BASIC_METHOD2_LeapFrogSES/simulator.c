#include "simulator.h"
#include <stdlib.h>

AEvent a_event_queue[MAX_EVENTS_A];

double random_double(double min, double max) {
    return min + (rand() / (double)RAND_MAX) * (max - min);
}

AEvent generate_next_A(AEvent prev, double T, double Vth) {
    AEvent a;
    a.T = T + prev.dT;
    a.V = random_double(0, 1);
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
    d.dT = random_double(0.1, 0.3);  // 0.100 ~ 0.299
    d.T = T + prev.dT;
    d.V = 1 - prev.V;
    d.bM = 1;
    
    return d;
}