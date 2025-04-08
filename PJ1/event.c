#include "event.h"
#include <stdlib.h>

Event event_queue[MAX_EVENTS];
int event_count = 0;    // initialization: record staffs in this quene

int compare(const void* a, const void* b) {
    Event* ea = (Event*)a;
    Event* eb = (Event*)b;
    int compare;
    if (ea->T > eb->T) {
        compare = 1;
    } else if (ea->T < eb->T) {
        compare = -1;
    } else if (ea->T == eb->T) {
        if (ea->type == 'A') {
            compare = -1;
        } else {
            compare = 1;
        }
    }
    return compare;
}

void insert_event(Event e) {
    if (event_count < MAX_EVENTS) {
        event_queue[event_count++] = e;
        sort_event_queue();
    }
}

void sort_event_queue() {
    qsort(event_queue, event_count, sizeof(Event), compare);   // sort based on time
}

Event pop_next_event() {
    Event e = event_queue[0];           // pop out the most recent event
    event_queue[0] = event_queue[1];    // after pop out, the queue only exist one element: A or M
    event_count--;
    return e;
}
