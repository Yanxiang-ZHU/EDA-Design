#ifndef EVENT_H
#define EVENT_H

typedef struct {
    double T;
    double V;
    double dT;
    double Vth;
    int bM;
} AEvent;

typedef struct {
    double T;
    int V;
    double dT;
    int bM;
} DEvent;

typedef struct {
    double T;
    char type;
    AEvent a;
    DEvent d;
} Event;

#define MAX_EVENTS 100000
extern Event event_queue[MAX_EVENTS];
extern int event_count;

void insert_event(Event e);
Event pop_next_event();
void sort_event_queue();

#endif
