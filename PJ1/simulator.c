#include "simulator.h"
#include "event.h"
#include <stdlib.h>

double random_double(double min, double max) {
    return min + (rand() / (double)RAND_MAX) * (max - min);
}

void generate_next_A(AEvent prev, double T, double Vth) {
    AEvent a;
    a.T = T + prev.dT;
    a.V = random_double(0, 1);
    a.dT = prev.dT;
    a.Vth = Vth;
    if ((prev.V <= Vth && a.V > Vth) || (prev.V > Vth && a.V <= Vth)) {  // across Vth
        a.bM = 1;
    }
    else {
        a.bM = 0;
    }
    Event e = {a.T, 'A', a, {0}};
    insert_event(e);
}

void generate_next_D(DEvent prev, double T) {
    DEvent d;
    d.dT = random_double(0.1, 0.3);
    d.T = T + prev.dT;
    d.V = 1 - prev.V;    // flip flop in V
    d.bM = 1;

    Event e = {d.T, 'D', {0}, d};
    insert_event(e);
}
