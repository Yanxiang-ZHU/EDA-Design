#include "event.h"
#include <stdlib.h>

Event event_queue[MAX_EVENTS];
int event_count = 0;

int compare(const void* a, const void* b) {
    Event* ea = (Event*)a;
    Event* eb = (Event*)b;
    return (ea->T > eb->T) - (ea->T < eb->T);
}

void insert_event(Event e) {
    if (event_count < MAX_EVENTS) {
        event_queue[event_count++] = e;
        sort_event_queue();
    }
}

void sort_event_queue() {
    qsort(event_queue, event_count, sizeof(Event), compare);
}

Event pop_next_event() {
    Event e = event_queue[0];
    for (int i = 1; i < event_count; ++i) {
        event_queue[i - 1] = event_queue[i];
    }
    event_count--;
    return e;
}
