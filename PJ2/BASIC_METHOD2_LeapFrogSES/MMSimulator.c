#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "config.h"
#include "simulator.h"


AEvent a;
AEvent a_prev;
DEvent d0;
DEvent d;
DEvent d_prev;

int tD = 0;
int initial = 1;
int wait_time = 0;
int Vdlast = 0;
int finish = 0;
int i = 1;

AEvent initialize_first_event(void) {
    // initialize event (T = 0) a0
    AEvent a = {0, (rand() % 1001) / 1000.0, dT_A, Vth, 0};         // parameters: T, V, dT, Vth, bM
    return a;
}

void waiting_for_first_D (FILE* fout){
    if (a.bM != 1) {
        fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)\n", a.T, a.V, a.Vth, a.dT, a.bM);
    } else {
        initial = 0;     // begin to generate the first D event
        DEvent DGEN = {a.T, 0, (101 + rand() % 199) / 1000.0, -1};
        d0 = DGEN;
        Vdlast = 0;
        fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)  D(%d %.3lf %-2d)\n", a.T, a.V, a.Vth, a.dT, a.bM, d0.V, d0.dT, d0.bM);
        d_prev = d0;
        d = generate_next_D(d0, a.T);
    }
    a_prev = a;
    a_event_queue[0] = a;
    a = generate_next_A(a, a.T, Vth);
}

int continue_all_A (FILE* fout) {
    if (a_event_queue[i].bM == 1) {
        DEvent DGEN = {a_event_queue[i].T, Vdlast * (int)(a_event_queue[i].V >= a_event_queue[i].Vth), (101 + rand() % 199) / 1000.0, -1};
        d = DGEN;
        fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)  D(%d %.3lf %-2d)\n", a_event_queue[i].T, a_event_queue[i].V, a_event_queue[i].Vth, a_event_queue[i].dT, a_event_queue[i].bM, d.V, d.dT, d.bM);
        Vdlast = d.V;
        d_prev = d;
        d = generate_next_D(d, a_event_queue[i].T);
        a_event_queue[0] = a_event_queue[i];
        i = 1;
        return 1;
    } else {
        fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)\n", a_event_queue[i].T, a_event_queue[i].V, a_event_queue[i].Vth, a_event_queue[i].dT, a_event_queue[i].bM);
    }
    return 0;
}

void D_happen (FILE* fout) {
    AEvent AGEN = {d.T, d.V, dT_A, Vth, -1}; 
    a = AGEN;
    fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)  D(%d %.3lf %-2d)\n", d.T, a.V, a.Vth, a.dT, a.bM, d.V, d.dT, d.bM);
    Vdlast = d.V;
    a_prev = a;
    a_event_queue[0] = a;
    d_prev = d;
    a = generate_next_A(a, d.T, Vth);  // d.T = t[n+1]
    d = generate_next_D(d, d.T);
    i = 1;
}



int main() {
    srand(time(NULL));

    // test the total time
    struct timeval total_start={0}, total_end={0};
    gettimeofday(&total_start, NULL);
    printf("Start Timing! ...\n");

    // read config from text
    read_config("con.txt");
    FILE* fout = fopen("sim_res.txt", "w");

    a = initialize_first_event(); 

    while (1) {
        if (a.T > Tsim || finish) break;

        if (initial == 1) {
            waiting_for_first_D(fout);
        }

        else {
            tD = rand() % 10 + 1; // 1~10
            usleep(tD * 1000);    // sleep for tD ms (tD * 1000 us)
            wait_time += tD;
            fprintf(fout, "***waiting for tD: %d ms***\n", tD);

            // find A's progress
            // case1: 
            if (dT_A / tA < d_prev.dT / tD) {  // A not reach t[n+1]
                // find if any bM=1 in A's current progress
                for (i = 1; i <= (tD/tA); i++) {
                    a_event_queue[i] = generate_next_A(a_event_queue[i-1], a_event_queue[i-1].T, Vth);
                    if (a_event_queue[i].T > Tsim) {
                        finish = 1;
                        i = 1;
                        break;
                    }

                    if (continue_all_A(fout)) {
                        break;
                    }
                }
                // no bM=1 in A's current progress
                if (i > (tD/tA)) {
                    for (i = (tD/tA) + 1; (i * dT_A) <= d_prev.dT; i=i+1) {
                        // A process
                        usleep(tA * 1000);    // sleep for tA ms (tA * 1000 us)
                        wait_time += tA;
                        fprintf(fout, "***waiting for tA: %d ms***\n", tA);

                        a_event_queue[i] = generate_next_A(a_event_queue[i-1], a_event_queue[i-1].T, Vth);
                        if (a_event_queue[i].T > Tsim) {
                            finish = 1;
                            i = 1;
                            break;
                        }
    
                        if (continue_all_A(fout)) {
                            break;
                        }
                    }
                }
                // no bM=1 in A's whole progress from t[n] to t[n+1] ---- A simulation bM=-1 at t[n+1]
                if ((i * dT_A) > d_prev.dT) {
                    if (d.T > Tsim) {
                        finish = 1;
                        i = 1;
                        break;
                    }
                    D_happen(fout);
                }
            }
            
            // case2:
            else {   // A already reach t[n+1]
                // find if any bM=1 in A's total progress
                for (i = 1; (i * dT_A) <= d_prev.dT; i=i+1) {
                    a_event_queue[i] = generate_next_A(a_event_queue[i-1], a_event_queue[i-1].T, Vth);
                    if (a_event_queue[i].T > Tsim) {
                        finish = 1;
                        i = 1;
                        break;
                    }
                    
                    if (continue_all_A(fout)) {
                        break;
                    }
                }
                // no bM=1 in A's total progress from t[n] to t[n+1] ---- A simulation bM=-1 at t[n+1]
                if ((i * dT_A) > d_prev.dT) {
                    if (d.T > Tsim) {
                        finish = 1;
                        i = 1;
                        break;
                    }
                    D_happen(fout);
                }
            }
        }
    }

    // Add total time output before closing
    gettimeofday(&total_end, NULL);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_usec - total_start.tv_usec) / 1000000.0;
    printf("End Timing!\n\n*********TEST RESULT**********\nTotal Time:  \t%.4f seconds\nWaiting Time:\t%.4f seconds\nRunning Time:\t%.4f  seconds\n", total_time, wait_time/1000.0, total_time - wait_time/1000.0);
    
    fprintf(fout, "%.3lf  FINISH\n", Tsim);
    fclose(fout);
    return 0;
}