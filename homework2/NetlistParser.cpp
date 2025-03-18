#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define BORDER_CHAR '*'
#define BORDER_WIDTH 45

typedef struct {
    int node1;
    int node2;
    double value;
} Element;

// structure for components connected to nodes
typedef struct Connection {
    char type; // store type: V, I, R, C, L
    int index;
    struct Connection* next;
} Connection;

// structure for node (for hash calculation)
typedef struct Node {
    int id;
    Connection* conns; // components connected to this node
    struct Node* next;
} Node;

#define HASH_SIZE 100000000
Node* hash_table[HASH_SIZE] = {NULL};

unsigned int hash(int key) {
    return (unsigned int)key % HASH_SIZE;
}

Node* find_or_insert_node(int id) {
    unsigned int h = hash(id);
    Node* n = hash_table[h];
    while (n) {
        if (n->id == id) return n;
        n = n->next;
    }
    n = (Node*)malloc(sizeof(Node));
    n->id = id;
    n->conns = NULL;
    n->next = hash_table[h];
    hash_table[h] = n;
    return n;
}

void add_connection(Node* node, char type, int index) {
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    conn->type = type;
    conn->index = index;
    conn->next = node->conns;
    node->conns = conn;
}

typedef struct {
    Element* elements;
    int size;
    int capacity;
} ElementArray;

// design for a dynamic element array (meet the requirement of the large-scale netlist)
void init_array(ElementArray* arr) {
    arr->size = 0;
    arr->capacity = 1000;
    arr->elements = (Element*)malloc(arr->capacity * sizeof(Element));
}

void append(ElementArray* arr, int node1, int node2, double value) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->elements = (Element*)realloc(arr->elements, arr->capacity * sizeof(Element));
    }
    arr->elements[arr->size].node1 = node1;
    arr->elements[arr->size].node2 = node2;
    arr->elements[arr->size].value = value;
    arr->size++;
}

// adjacency node list (for non-terrestrial nodes)
typedef struct AdjNode {
    int id;
    struct AdjNode* next;
} AdjNode;

typedef struct {
    AdjNode* head;
} AdjList;

AdjList* adj = NULL;
int num_nodes = 0;

void add_edge(int n1, int n2) {
    if (n1 >= num_nodes || n2 >= num_nodes) {
        int new_size = (n1 > n2 ? n1 : n2) + 1;
        adj = (AdjList*)realloc(adj, new_size * sizeof(AdjList));
        for (int i = num_nodes; i < new_size; i++) {
            adj[i].head = NULL;
        }
        num_nodes = new_size;
    }
    AdjNode* node1 = (AdjNode*)malloc(sizeof(AdjNode));
    node1->id = n2;
    node1->next = adj[n1].head;
    adj[n1].head = node1;

    AdjNode* node2 = (AdjNode*)malloc(sizeof(AdjNode));
    node2->id = n1;
    node2->next = adj[n2].head;
    adj[n2].head = node2;
}

// find the connection via depth first search argorithm
void dfs(int v, bool* visited, ElementArray* vs, ElementArray* is, bool* has_power) {
    visited[v] = true;
    Node* n = find_or_insert_node(v);
    for (Connection* c = n->conns; c; c = c->next) {
        if (c->type == 'V' || c->type == 'I') {
            *has_power = true;
            break;
        }
    }
    for (AdjNode* a = adj[v].head; a; a = a->next) {
        if (!visited[a->id] && a->id!=0) {      // avoid connect to ground node (because ground node would connect to other circuits unexpectedly)
            dfs(a->id, visited, vs, is, has_power);      // loop of DFS argorithm here
        }
    }
}

void dfs_component(int v, bool* visited, int comp_id, int* component, bool* has_power, bool* has_ground) {
    if (visited[v]) return;
    visited[v] = true;
    component[v] = comp_id;
    // check if it is connected to the ground node
    if (v==0) {
        has_ground[comp_id] = true;
    }
    // chech if it is connected to sources
    Node* n = find_or_insert_node(v);
    for (Connection* c = n->conns; c; c = c->next) {
        if (c->type == 'V' || c->type == 'I') {
            has_power[comp_id] = true;
        }
    }
    // traversing adjacent nodes
    for (AdjNode* a = adj[v].head; a; a = a->next) {
        dfs_component(a->id, visited, comp_id, component, has_power, has_ground);
    }
}

// generate log file (redirecting)
void logSummary(FILE *logFile, const char *message) {
    fprintf(logFile, "* %-38s\n", message);
}

// interesting decoration (create border!)
void printBorder(FILE *file) {
    for (int i = 0; i < BORDER_WIDTH; i++) {
        fprintf(file, "%c", BORDER_CHAR);
    }
    fprintf(file, "\n");
}

void log_run_end(FILE* logFile, time_t start_time, int count_components, int error_state, const char* errorMsg) {
    time_t end_time = time(NULL);
    fprintf(logFile, "* RUN END:   %s", ctime(&end_time));
    fprintf(logFile, "* TOTAL OPERATING DURATION: %ld seconds\n", end_time - start_time);
    fprintf(logFile, "* TOTAL COMPONENTS: %-38d\n", count_components);
    if (error_state == 0) {
        fprintf(logFile, "* %s\n", "INFO: Run successfully. (0 error).");
    } else if (error_state == 1) {
        logSummary(logFile, errorMsg);
    }
    printBorder(logFile);
}

int main(int argc, char* argv[]) {
    /////////////////////////////////////////////////////////////
    FILE *logFile = fopen("RunSummary.txt", "w");
    if (logFile == NULL) {
        printf("Can't create RunSummary.txt\n");
        return 1;
    }

    printBorder(logFile);
    logSummary(logFile, "SOFTWARE NAME: ZYX-EDA NetlistParser");
    logSummary(logFile, "DEVELOPER: ZHU Yanxiang");
    logSummary(logFile, "VERSION: 1.0.0");
    logSummary(logFile, "RELEASE DATE: 2025-03-20");

    // record the starting time
    time_t start_time = time(NULL);
    fprintf(logFile, "* RUN START: %s", ctime(&start_time));
    int error_state = 0;   // if error occurs, turn the state to 1, and output the error info in the end
    char errorMsg[300];
    strcpy(errorMsg, "Error somewhere. Need further examing!"); 
    // if the error is not specified errors we labeled, output the error info like this

    ///////////////////////////////////////////////////////////////
    bool detail = false; // state of detail analyzing mode (#output floating components; #output short nodes; #output open nodes)
    bool fix = false;    // state of fixing mode (#delete floating components)

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            detail = true;
        }
        if (strcmp(argv[i], "-c") == 0) {
            fix = true;
        }
    }

    FILE* fin;
    // recognize the user's needs
    if (argc == 1) {
        sprintf(errorMsg, "ERROR Usage: %s <-c /-v /-cv /-vc> <input.sp>", argv[0]);
        error_state = 1;
        log_run_end(logFile, start_time, 0, error_state, errorMsg);
        return 1;
    } else if (argc == 2) {
        fin = fopen(argv[1], "r");
    } else if (argc == 3) {
        if (strcmp(argv[1], "-v") == 0) detail = true;
        else if (strcmp(argv[1], "-c") == 0) fix = true;
        else if (strcmp(argv[1], "-cv") == 0 || strcmp(argv[1], "-vc") == 0) {detail = true; fix = true;}
        else {
            sprintf(errorMsg, "2ERROR Usage: %s <-c /-v /-cv /-vc> <input.sp>", argv[0]);
            error_state = 1;
            log_run_end(logFile, start_time, 0, error_state, errorMsg);
            return 1;
        }
        fin = fopen(argv[2], "r");
    } else if (argc == 4) {
        if ((strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-c") == 0) || (strcmp(argv[2], "-v") == 0 && strcmp(argv[1], "-c") == 0)) {
            detail = true;
            fix = true;
        } else {
            sprintf(errorMsg, "3ERROR Usage: %s <-c /-v /-cv /-vc> <input.sp>", argv[0]);
            error_state = 1;
            log_run_end(logFile, start_time, 0, error_state, errorMsg);
            return 1;
        }
        fin = fopen(argv[3], "r");
    } else {
        sprintf(errorMsg, "ERROR Usage: %s <-c /-v /-cv /-vc> <input.sp>", argv[0]);
        error_state = 1;
        log_run_end(logFile, start_time, 0, error_state, errorMsg);
        return 1;
    }


    // open the spice file (need to get the filename, like 'check')
    if (!fin) {
        sprintf(errorMsg, "ERROR: Cannot open file!");
        error_state = 1;
        log_run_end(logFile, start_time, 0, error_state, errorMsg);
        return 1;
    }

    // initialization
    ElementArray voltage_sources, current_sources, resistors, inductors, capacitors;
    init_array(&voltage_sources);
    init_array(&current_sources);
    init_array(&resistors);
    init_array(&inductors);
    init_array(&capacitors);

    // analyze the file
    char line[256];
    int count_components = 0;
    while (fgets(line, sizeof(line), fin)) {
        if (line[0] == '*' || line[0] == '\n') continue;   // dismiss other infomation
        char type = line[0];
        int node1, node2;
        double value;
        if (sscanf(line, "%*s %d %d %lf", &node1, &node2, &value) != 3) {
            sprintf(errorMsg, "ERROR: Invalid line: %s", line);
            error_state = 1;
            log_run_end(logFile, start_time, count_components, error_state, errorMsg);
            return 1;
        } else {
            count_components++;
        }

        Node* n1 = find_or_insert_node(node1);
        Node* n2 = (node1 != node2) ? find_or_insert_node(node2) : n1;
        if (type == 'V') {
            append(&voltage_sources, node1, node2, value);
            add_connection(n1, 'V', voltage_sources.size - 1);
            if (node1 != node2) add_connection(n2, 'V', voltage_sources.size - 1);
        } else if (type == 'I') {
            append(&current_sources, node1, node2, value);
            add_connection(n1, 'I', current_sources.size - 1);
            if (node1 != node2) add_connection(n2, 'I', current_sources.size - 1);
        } else if (type == 'R') {
            append(&resistors, node1, node2, value);
            add_connection(n1, 'R', resistors.size - 1);
            if (node1 != node2) add_connection(n2, 'R', resistors.size - 1);
        } else if (type == 'L') {
            append(&inductors, node1, node2, value);
            add_connection(n1, 'L', inductors.size - 1);
            if (node1 != node2) add_connection(n2, 'L', inductors.size - 1);
        } else if (type == 'C') {
            append(&capacitors, node1, node2, value);
            add_connection(n1, 'C', capacitors.size - 1);
            if (node1 != node2) add_connection(n2, 'C', capacitors.size - 1);
        }
    }
    fclose(fin);

    //  step1: calculating the amount of different components
    int nVoltS = voltage_sources.size;
    int nCurrS = current_sources.size;
    int nR = resistors.size;
    int nC = capacitors.size;
    int nL = inductors.size;
    // int short_circuit_source=0;     // no consideration for source both sides connected to ground in this project!

    // step2: calculating the amount of short
    int nShort = 0;
    // case1 -- multiple VoltSources in parallel (prior than case2)
    int* pair_count = (int*)calloc(HASH_SIZE, sizeof(int));
    for (int i = 0; i < nVoltS; i++) {
        int n1 = voltage_sources.elements[i].node1;
        int n2 = voltage_sources.elements[i].node2;
        int min_n = n1 < n2 ? n1 : n2;
        int max_n = n1 > n2 ? n1 : n2;
        unsigned int h = hash(min_n * HASH_SIZE * HASH_SIZE + max_n);
        pair_count[h]++;
        if (pair_count[h] == 2) nShort++;   // only add nShort when pair_count==2, no counting after it (even 3 or more pairs)
    }
    // case2 -- directly connect two nodes of VoltSource together
    for (int i = 0; i < nVoltS; i++) {
        int n1 = voltage_sources.elements[i].node1;
        int n2 = voltage_sources.elements[i].node2;
        int min_n = n1 < n2 ? n1 : n2;
        int max_n = n1 > n2 ? n1 : n2;
        unsigned int h = hash(min_n * HASH_SIZE * HASH_SIZE + max_n);
        if (pair_count[h] == 1 && n1 == n2) nShort++;
    }
    free(pair_count);

    // step3: calculating the amount of open
    int nOpen = 0;
    for (int i = 0; i < nVoltS; i++) {
        int n1 = voltage_sources.elements[i].node1;
        int n2 = voltage_sources.elements[i].node2;
        if (n1 != n2) {
            Node* node1 = find_or_insert_node(n1);
            Connection* c = node1->conns;
            int degree1 = 0;
            for (; c; c = c->next) degree1++;
            if (degree1 == 1) nOpen++;

            Node* node2 = find_or_insert_node(n2);
            c = node2->conns;
            int degree2 = 0;
            for (; c; c = c->next) degree2++;
            if (degree2 == 1) nOpen++;
        }
    }
    for (int i = 0; i < nCurrS; i++) {
        int n1 = current_sources.elements[i].node1;
        int n2 = current_sources.elements[i].node2;
        if (n1 != n2) {
            Node* node1 = find_or_insert_node(n1);
            Connection* c = node1->conns;
            int degree1 = 0;
            for (; c; c = c->next) degree1++;
            if (degree1 == 1) nOpen++;

            Node* node2 = find_or_insert_node(n2);
            c = node2->conns;
            int degree2 = 0;
            for (; c; c = c->next) degree2++;
            if (degree2 == 1) nOpen++;
        }
    }

    // create the graph
    for (int i = 0; i < nVoltS; i++) {
        int n1 = voltage_sources.elements[i].node1;
        int n2 = voltage_sources.elements[i].node2;
        if (n1 != n2) add_edge(n1, n2);
        // if (n1 == 0 && n2 == 0) short_circuit_source++;
    }
    for (int i = 0; i < nCurrS; i++) {
        int n1 = current_sources.elements[i].node1;
        int n2 = current_sources.elements[i].node2;
        if (n1 != n2) add_edge(n1, n2);
        // if (n1 == 0 && n2 == 0) short_circuit_source++;
    }
    for (int i = 0; i < nR; i++) {
        int n1 = resistors.elements[i].node1;
        int n2 = resistors.elements[i].node2;
        if (n1 != n2) add_edge(n1, n2);
    }
    for (int i = 0; i < nL; i++) {
        int n1 = inductors.elements[i].node1;
        int n2 = inductors.elements[i].node2;
        if (n1 != n2) add_edge(n1, n2);
    }
    for (int i = 0; i < nC; i++) {
        int n1 = capacitors.elements[i].node1;
        int n2 = capacitors.elements[i].node2;
        if (n1 != n2) add_edge(n1, n2);
    }

    // step4: calculating the amount of float
    bool* visited = (bool*)calloc(num_nodes, sizeof(bool));
    bool* has_power = (bool*)calloc(num_nodes, sizeof(bool));
    bool* has_ground = (bool*)calloc(num_nodes, sizeof(bool));
    int* component = (int*)malloc(num_nodes * sizeof(int));
    int comp_id = 0;
    for (int v = 0; v < num_nodes; v++) {
        if (!visited[v]) {
            dfs_component(v, visited, comp_id, component, has_power, has_ground);
            comp_id++;
        }
    }
    free(visited);
    int nFloat = 0;
    ElementArray* passive_arrays[] = {&resistors, &capacitors, &inductors};
    int passive_sizes[] = {nR, nC, nL};
    for (int a = 0; a < 3; a++) {
        for (int i = 0; i < passive_sizes[a]; i++) {
            int n1 = passive_arrays[a]->elements[i].node1;
            int n2 = passive_arrays[a]->elements[i].node2;
            if (n1 >= num_nodes || n2 >= num_nodes) continue;
            int comp_n1 = component[n1];
            int comp_n2 = component[n2];
            if (comp_n1 == comp_n2 && !has_power[comp_n1] && !has_ground[comp_n1]) {
                nFloat++;
            }
        }
    }

    // step5: calculating the amount of circuits
    bool* visit = (bool*)calloc(num_nodes, sizeof(bool));
    int nCircuit = 0;  // do not consider cases that both sides of the source are connected to ground(0). [If considered, substitute 0 with short_circuit_source]
    for (int v = 1; v < num_nodes; v++) { // starting from 1st node (ground not in consideration)
        if (!visit[v]) {
            bool power = false;
            dfs(v, visit, &voltage_sources, &current_sources, &power);
            if (power) nCircuit++;
        }
    }
    free(visit);

    // output into csv file
    FILE* fout = fopen("NetlistReport.csv", "w");
    if (fout == NULL) {
        strcpy(errorMsg, "Error: Can't create file NetlistReport.csv");
        error_state = 1;
        log_run_end(logFile, start_time, count_components, error_state, errorMsg);
        return 1;
    }
    fprintf(fout, "nVoltS, %d\n", nVoltS);
    fprintf(fout, "nCurrS, %d\n", nCurrS);
    fprintf(fout, "nR, %d\n", nR);
    fprintf(fout, "nC, %d\n", nC);
    fprintf(fout, "nL, %d\n", nL);
    fprintf(fout, "nShort, %d\n", nShort);
    fprintf(fout, "nOpen, %d\n", nOpen);
    fprintf(fout, "nFloat, %d\n", nFloat);
    fprintf(fout, "nCircuit, %d\n", nCircuit);
    fclose(fout);


    // free the memory 
    free(voltage_sources.elements);
    free(current_sources.elements);
    free(resistors.elements);
    free(inductors.elements);
    free(capacitors.elements);
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* n = hash_table[i];
        while (n) {
            Connection* c = n->conns;
            while (c) {
                Connection* next = c->next;
                free(c);
                c = next;
            }
            Node* next = n->next;
            free(n);
            n = next;
        }
    }
    for (int i = 0; i < num_nodes; i++) {
        AdjNode* a = adj[i].head;
        while (a) {
            AdjNode* next = a->next;
            free(a);
            a = next;
        }
    }
    free(adj);


    // record the end of software
    log_run_end(logFile, start_time, count_components, error_state, errorMsg);
    fclose(logFile);

    return 0;
}