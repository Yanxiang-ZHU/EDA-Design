#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "event.h"

void generate_next_A(AEvent prev, double Vth);
void generate_next_D(DEvent prev);

#endif
