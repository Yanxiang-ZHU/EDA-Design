#include "simulator.h"
#include "event.h"
#include <stdlib.h>

double random_double(double min, double max) {
    return min + (rand() / (double)RAND_MAX) * (max - min);
}

void generate_next_A(AEvent prev, double Vth) {
    AEvent a;
    a.T = prev.T + prev.dT;
    a.V = random_double(0, 1);
    a.dT = prev.dT;
    a.Vth = Vth;
    a.bM = 0;

    Event e = { a.T, 'A', a, {0} };
    insert_event(e);
}

void generate_next_D(DEvent prev) {
    DEvent d;
    d.dT = random_double(0.1, 0.3);
    d.T = prev.T + d.dT;
    d.V = 1 - prev.V; // 翻转
    d.bM = 0;

    Event e = { d.T, 'D', {0}, d };
    insert_event(e);
}
