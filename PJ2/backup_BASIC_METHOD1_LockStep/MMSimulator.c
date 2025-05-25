#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "config.h"
#include "event.h"
#include "simulator.h"

int main() {
    srand(time(NULL));

    // test the total time
    struct timeval total_start={0}, total_end={0};
    gettimeofday(&total_start, NULL);
    printf("Start Timing! ...\n");

    // read config from text
    read_config("con.txt");

    // initialize event (T = 0)
    double TD0 = random_double(0.1, 0.3);
    AEvent a0 = {0, random_double(0, 1), dT_A, Vth, 0};         // parameters: T, V, dT, Vth, bM
    Event eA = {0, 'A', a0, {0}};
    insert_event(eA);       // pop into the queue and sort based on time
    int Vdlast = 0;
    int tD = 0;
    int initial = 1;
    int wait_time = 0;

    // output to file <sim_res.txt>, filename is adjustable
    FILE* fout = fopen("sim_res.txt", "w");

    // event loop -- focus on event queue
    while (1) {
        Event e = pop_next_event();
        if (e.T > Tsim) break;  

        // write the text and deal with the communication between A and D
        if (e.type == 'A') {
            fprintf(fout, "%.3lf  A(%.2lf %.2lf %.2lf %-2d)  ",
                    e.a.T, e.a.V, e.a.Vth, e.a.dT, e.a.bM);
            // communication: A -> D
            if (e.a.bM == 1) {
                if (initial != 1) {
                    pop_next_event();   // pop the orginal D event out (discarded)
                } else {
                    initial = 0;
                }
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

            // printf("Waiting for A...\n");
            usleep(tA * 1000);    // sleep for tA ms (tA * 1000 us)
            wait_time += tA;
        } 
        else if (e.type == 'D') {
            pop_next_event();       // if D event pop out, A event definitely need refresh
            AEvent a_cut = {0, e.d.V, dT_A, Vth, -1}; 
            fprintf(fout, "%.3lf  A(%.2lf %.2lf %.2lf %-2d)  D(%d %.2lf %-2d)\n",
                    e.d.T, a_cut.V, a_cut.Vth, a_cut.dT, a_cut.bM, e.d.V, e.d.dT, e.d.bM);
            Vdlast = e.d.V;
            generate_next_A(a_cut, e.d.T, Vth);
            generate_next_D(e.d, e.d.T);

            // printf("Waiting for D...\n");
            tD = rand() % 10 + 1; // 1~10
            usleep(tD * 1000);    // sleep for tD ms (tD * 1000 us)
            wait_time += tD;
        }
    }

    // Add total time output before closing
    gettimeofday(&total_end, NULL);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_usec - total_start.tv_usec) / 1000000.0;
    printf("End Timing!\n\n*********TEST RESULT**********\nTotal Time:\t%.4f seconds\nWaiting Time:\t%.4f seconds\nRunning Time:\t%.4f seconds\n", total_time, wait_time/1000.0, total_time - wait_time/1000.0);
    
    fprintf(fout, "%.3lf  FINISH\n", Tsim);
    fclose(fout);
    return 0;
}
