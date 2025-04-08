#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

double dT_A, Vth, Tsim;

void read_config(const char* filename) {
    FILE* f = fopen(filename, "r");
    char name[32];
    while (fscanf(f, "%s", name) != EOF) {
        if (strcmp(name, "dT") == 0) fscanf(f, "%lf", &dT_A);
        else if (strcmp(name, "Vth") == 0) fscanf(f, "%lf", &Vth);
        else if (strcmp(name, "Tsim") == 0) fscanf(f, "%lf", &Tsim);
        else {
            printf("ERROR: <con.txt> format!\n");
            exit(0);
        }
    }
    if ((dT_A>=0.1) || (dT_A <=0.0)) {
        printf("ERROR: dT %lf is not allowed!\n", dT_A);
        exit(0);
    }
    if ((Vth>=1.0) || (Vth<=0.0)) {
        printf("ERROR: Vth %lf is not allowed!\n", Vth);
        exit(0);
    }
    if (Tsim <= 0.0) {
        printf("ERROR: Tsim %lf should be a positive number!\n", Tsim);
        exit(0);
    }
    fclose(f);
}
