#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NODES 10000
#define MAX_COMPONENTS 1000

typedef struct {
    int nVoltS, nCurrS, nR, nC, nL, nShort, nLoop, bOpen;
} Config;

typedef struct {
    char type[3];
    char name[10];
    int node1, node2;
    char value[20];
} Component;

int parent[MAX_NODES];
Component components[MAX_COMPONENTS];
int comp_count = 0;

void dsu_init() {
    for (int i = 0; i < MAX_NODES; i++) parent[i] = i;
}

// find PARENT node
int find(int x) {
    return parent[x] == x ? x : (parent[x] = find(parent[x]));
}

void union_nodes(int a, int b) {
    int ra = find(a), rb = find(b);
    if (ra != rb) parent[rb] = ra;
}

// read the config file
void parse_config(const char* filename, Config* cfg) {
    FILE* fp = fopen(filename, "r");
    char key[20];
    memset(cfg, 0, sizeof(Config));
    
    while (fscanf(fp, "%s", key) != EOF) {
        if      (!strcmp(key, "nVoltS")) fscanf(fp, "%d", &cfg->nVoltS);
        else if (!strcmp(key, "nCurrS")) fscanf(fp, "%d", &cfg->nCurrS);
        else if (!strcmp(key, "nR"))     fscanf(fp, "%d", &cfg->nR);
        else if (!strcmp(key, "nC"))     fscanf(fp, "%d", &cfg->nC);
        else if (!strcmp(key, "nL"))     fscanf(fp, "%d", &cfg->nL);
        else if (!strcmp(key, "nShort")) fscanf(fp, "%d", &cfg->nShort);
        else if (!strcmp(key, "nLoop"))  fscanf(fp, "%d", &cfg->nLoop);
        else if (!strcmp(key, "bOpen"))  fscanf(fp, "%d", &cfg->bOpen);
    }
    fclose(fp);
}

void gen_nodes(int* n1, int* n2) {
    do {
        *n1 = rand() % 100;
        *n2 = rand() % 100;
        *n1 = find(*n1);
        *n2 = find(*n2);
    } while (*n1 == *n2);
}

void gen_value(char* buf, char type) {
    switch(type) {
        case 'V': case 'I':
            sprintf(buf, "%d", rand()%100 + 1); break;
        case 'R': 
            sprintf(buf, "%d%c", rand()%1000 + 1, " kM"[rand()%3]); break;
        case 'C':
            sprintf(buf, "%.3f%c", (rand()%1000)/1000.0, "pnu"[rand()%3]); break;
        case 'L':
            sprintf(buf, "%d%c", rand()%1000 + 1, "nmu"[rand()%3]); break;
    }
}

void gen_component(char type, int count) {
    for (int i = 1; i <= count; i++) {
        Component* c = &components[comp_count++];
        sprintf(c->type, "%c", type);
        sprintf(c->name, "%c%d", type, i);
        gen_nodes(&c->node1, &c->node2);
        gen_value(c->value, type);
    }
}

void handle_open(Config cfg) {
    if (!cfg.bOpen) return;
    int n = rand()%100 + 100;
    Component* c = &components[comp_count++];
    strcpy(c->type, "R");
    sprintf(c->name, "R%d", cfg.nR + 1);
    c->node1 = find(rand()%100);
    c->node2 = find(n);
    gen_value(c->value, 'R');
}

int main() {
    srand(time(NULL));
    dsu_init();
    Config cfg;
    
    parse_config("netlistConfig", &cfg);
    
    for (int i = 0; i < cfg.nShort; i++) 
        union_nodes(rand()%100, rand()%100);

    gen_component('V', cfg.nVoltS);
    gen_component('I', cfg.nCurrS);
    gen_component('R', cfg.nR);
    gen_component('C', cfg.nC);
    gen_component('L', cfg.nL);
    
    handle_open(cfg);
    
    FILE* fp = fopen("netlistDump.sp", "w");
    for (int i = 0; i < comp_count; i++) {
        Component* c = &components[i];
        fprintf(fp, "%s %s %d %d %s\n", 
               c->type, c->name, c->node1, c->node2, c->value);
    }
    fclose(fp);
    
    return 0;
}