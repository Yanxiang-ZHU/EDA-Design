#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "config.h"
#include "simulator.h"

#define NUM_THREADS 12

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

// generate D events parallelly (two events a time)
int tD_1;
int tD_2;

typedef struct {
    double tD;
    int idx;
} DThreadArgs;


AEvent first_A_events[NUM_THREADS];
DEvent d_event_queue[3];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

AEvent initialize_first_event(void) {
    // initialize event (T = 0) a0
    AEvent a = {0, (rand() % 1001) / 1000.0, dT_A, Vth, 0};         // parameters: T, V, dT, Vth, bM
    return a;
}

// three queues for A and D events
void* generate_A_parallel(void* arg) {
    int index = *(int*)arg;
    
    usleep(tA * 1000);
    pthread_mutex_lock(&lock);
    wait_time += tA;
    pthread_mutex_unlock(&lock);

    AEvent a_parallel = generate_next_A(a, a.T, Vth);

    pthread_mutex_lock(&lock);
    first_A_events[index] = a_parallel;
    pthread_mutex_unlock(&lock);
    free(arg);
    return NULL;
}

void* generate_A(void* arg) {
    int index = *(int*)arg;
    
    usleep(tA * 1000);
    pthread_mutex_lock(&lock);
    wait_time += tA;
    pthread_mutex_unlock(&lock);

    AEvent a_parallel = generate_next_A(a, a.T, Vth);

    pthread_mutex_lock(&lock);
    a_event_queue[index] = a_parallel;
    pthread_mutex_unlock(&lock);
    free(arg);
    return NULL;
}

void* generate_D (void* arg) {
    DThreadArgs* args = (DThreadArgs*)arg;

    usleep(args->tD * 1000);
    pthread_mutex_lock(&lock);
    wait_time += args->tD;
    pthread_mutex_unlock(&lock);

    DEvent d_parallel = generate_next_D(d, d.T);

    pthread_mutex_lock(&lock);
    d_event_queue[args->idx] = d_parallel;
    pthread_mutex_unlock(&lock);
    free(arg);
    return NULL;    
}

// waiting for first D event (initialization)
void waiting_for_first_D(FILE* fout) {
    while (initial == 1) {
        pthread_t threads[NUM_THREADS];
        first_A_events[0] = a;

        for (int i = 1; i <= NUM_THREADS; i++) {
            int* idx = malloc(sizeof(int));
            *idx = i;
            pthread_create(&threads[i], NULL, generate_A_parallel, idx);
        }
        for (int i = 1; i <= NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        // further process
        for (int i = 0; i <= NUM_THREADS; i++) {
            if (first_A_events[i].bM == 1) {
                initial = 0;
                DEvent DGEN = {a.T, 0, (101 + rand() % 199) / 1000.0, -1};
                d0 = DGEN;
                Vdlast = 0;
                fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)  D(%d %.3lf %-2d)\n",
                    first_A_events[i].T, first_A_events[i].V, first_A_events[i].Vth,
                    first_A_events[i].dT, first_A_events[i].bM, d0.V, d0.dT, d0.bM);
                d_prev = d0;
                d = generate_next_D(d0, a.T);
                d_event_queue[0] = d;
                a_event_queue[0] = first_A_events[i];
                break;
            } else if (i != NUM_THREADS){
                fprintf(fout, "%.3lf  A(%.3lf %.3lf %.3lf %-2d)\n",
                    first_A_events[i].T, first_A_events[i].V, first_A_events[i].Vth,
                    first_A_events[i].dT, first_A_events[i].bM);
            }

            // update bM and T value
            if ((first_A_events[i].V <= Vth && first_A_events[i+1].V > Vth) ||
                (first_A_events[i].V > Vth && first_A_events[i+1].V <= Vth)) {
                first_A_events[i + 1].bM = 1;
            } else {
                first_A_events[i + 1].bM = 0;
            }
            first_A_events[i + 1].T = first_A_events[i].T + first_A_events[i].dT;
        }

        a = first_A_events[NUM_THREADS];
    }
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


    if (initial == 1) {
        waiting_for_first_D(fout);
    }

    while (1) {
        if (a.T > Tsim || finish) break;

        tD_1 = rand() % 10 + 1; // 1~10
        tD_2 = rand() % 10 + 1; // 1~10

        // generate two D events (and several A events) parallelly
        pthread_t threads[NUM_THREADS];

        DThreadArgs* args_1 = malloc(sizeof(DThreadArgs));
        args_1->tD = tD_1;
        args_1->idx = 1;
        pthread_create(&threads[0], NULL, generate_D, args_1);

        DThreadArgs* args_2 = malloc(sizeof(DThreadArgs));
        args_2->tD = tD_2;
        args_2->idx = 2;
        pthread_create(&threads[1], NULL, generate_D, args_2);

        for (int i = 2; i < NUM_THREADS; i++) {
            int* idx = malloc(sizeof(int));
            *idx = i-1;   // starting from a_event_queue[1] (a_event_queue for t[n])
            pthread_create(&threads[i], NULL, generate_A, idx);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        // further process
        if (d_event_queue[1].)

        

    }

    // Add total time output before closing
    gettimeofday(&total_end, NULL);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_usec - total_start.tv_usec) / 1000000.0;
    printf("End Timing!\n\n*********TEST RESULT**********\nTotal Time:  \t%.4f seconds\nWaiting Time:\t%.4f seconds\nRunning Time:\t%.4f  seconds\n", total_time, wait_time/1000.0, total_time - wait_time/1000.0);
    
    fprintf(fout, "%.3lf  FINISH\n", Tsim);
    fclose(fout);
    return 0;
}