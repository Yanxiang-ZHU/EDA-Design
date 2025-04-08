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

    // initialize event (T = 0)
    double TD0 = random_double(0.1, 0.3);
    AEvent a0 = {0, random_double(0, 1), dT_A, Vth, 0};         // parameters: T, V, dT, Vth, bM
    DEvent d0 = {TD0, rand() % 2, random_double(0.1, 0.3), 1};    // parameters: T, V, dT, bM
    Event eA = {0, 'A', a0, {0}};
    Event eD = {TD0, 'D', {0}, d0};
    insert_event(eA);       // pop into the queue and sort based on time
    insert_event(eD);
    int Vdlast = 0;

    // output to file <sim_res.txt>, filename is adjustable
    FILE* fout = fopen("sim_res.txt", "w");

    // event loop -- focus on event queue
    while (1) {
        Event e = pop_next_event();
        // dead loop if Tsim is not read correctly
        if (e.T > Tsim) break;  

        // write the text and deal with the communication between A and D
        if (e.type == 'A') {
            fprintf(fout, "%.2lf  A(%.2lf %.2lf %.2lf %-2d)  ",
                    e.a.T, e.a.V, e.a.Vth, e.a.dT, e.a.bM);
            // communication: A -> D
            if (e.a.bM == 1) {
                pop_next_event();   // pop the orginal D event out (discarded)
                DEvent d_cut = {0, Vdlast * (int)(e.a.V >= e.a.Vth), random_double(0.1, 0.3), -1};
                fprintf(fout, "D(%d %.2lf %-2d)\n",
                    d_cut.V, d_cut.dT, d_cut.bM);
                Vdlast = d_cut.V;
                generate_next_D(d_cut, e.a.T);
            }
            else {
                fprintf(fout, "\n");
            }
            generate_next_A(e.a, e.a.T, Vth);
        } 
        else if (e.type == 'D') {
            Event ee = pop_next_event();       // if D event pop out, A event definitely need refresh
            AEvent a_cut = {0, random_double(0, 1), dT_A, Vth, -1}; 
            fprintf(fout, "%.2lf  A(%.2lf %.2lf %.2lf %-2d)  D(%d %.2lf %-2d)\n",
                    e.d.T, a_cut.V, a_cut.Vth, a_cut.dT, a_cut.bM, e.d.V, e.d.dT, e.d.bM);
            Vdlast = e.d.V;
            generate_next_A(a_cut, e.d.T, Vth);
            generate_next_D(e.d, e.d.T);
        }
    }

    fclose(fout);
    return 0;
}
