#include "config.h"
#include <stdio.h>

double dT_A, Vth, Tsim;

void read_config(const char* filename) {
    FILE* f = fopen(filename, "r");
    char name[32];
    while (fscanf(f, "%s", name) != EOF) {
        if (strcmp(name, "dT") == 0) fscanf(f, "%lf", &dT_A);
        else if (strcmp(name, "Vth") == 0) fscanf(f, "%lf", &Vth);
        else if (strcmp(name, "Tsim") == 0) fscanf(f, "%lf", &Tsim);
    }
    fclose(f);
}
