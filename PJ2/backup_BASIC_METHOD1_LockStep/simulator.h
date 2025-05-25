#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "event.h"

void generate_next_A(AEvent prev, double T, double Vth);
void generate_next_D(DEvent prev, double T);

double random_double(double min, double max);

#endif
