#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "event.h"
#include "simulator.h"

int main() {
    srand(time(NULL));

    // read config from text
    read_config("con.txt");

    // initialize event
    AEvent a0 = {0, random_double(0, 1), dT_A, Vth, 0};
    DEvent d0 = {0, rand() % 2, random_double(0.1, 0.3), 0};

    Event eA = {0, 'A', a0, {0}};
    Event eD = {0, 'D', {0}, d0};
    insert_event(eA);
    insert_event(eD);

    FILE* fout = fopen("sim_res.txt", "w");

    while (event_count > 0) {
        Event e = pop_next_event();
        if (e.T > Tsim) break;

        if (e.type == 'A') {
            fprintf(fout, "%.2lf A(%.2lf %.2lf %.2lf %d)\n",
                    e.a.T, e.a.V, e.a.Vth, e.a.dT, e.a.bM);
            generate_next_A(e.a, Vth);
        } else {
            fprintf(fout, "%.2lf D(%d %.2lf %d)\n",
                    e.d.T, e.d.V, e.d.dT, e.d.bM);
            generate_next_D(e.d);
        }
    }

    fclose(fout);
    return 0;
}
