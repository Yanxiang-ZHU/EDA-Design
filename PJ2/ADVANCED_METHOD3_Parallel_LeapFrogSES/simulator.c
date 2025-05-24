#include "simulator.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


AEvent a_event_queue[MAX_EVENTS_A];

unsigned int generate_seed() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (unsigned int)(ts.tv_nsec ^ ts.tv_sec ^ (unsigned long)pthread_self());
}

AEvent generate_next_A(AEvent prev, double T, double Vth) {
    AEvent a;
    a.T = T + prev.dT;
    unsigned int seed = generate_seed();
    a.V = (rand_r(&seed) % 1001) / 1000.0;   // a.V = 0.000 ~ 1.000, including boundary
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