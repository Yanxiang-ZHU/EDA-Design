#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NODES 1000
#define MAX_COMPONENTS 1000
#define GROUND_NODE 0

typedef struct {
    int nVoltS, nCurrS, nR, nC, nL, nShort, nLoop, bOpen;
} Config;

typedef struct {
    char type[3];
    char name[10];
    int node1;  
    int node2;
    char value[20];
} Component;

Component components[MAX_COMPONENTS];
int comp_count = 0;
int parent[MAX_NODES];
int node_usage[MAX_NODES] = {0};
int current_max_node = GROUND_NODE;

void dsu_init() {
    for(int i=0; i<MAX_NODES; i++) parent[i] = i;
}

int find(int x) {
    return parent[x] = (parent[x] == x ? x : find(parent[x]));
}

void merge_nodes(int a, int b) {
    parent[find(b)] = find(a);
}

int generate_connected_node(int must_exist) {
    if(must_exist || current_max_node == GROUND_NODE) {
        int attempts = 0;
        do {
            int node = rand() % (current_max_node + 1);
            if(node_usage[node] > 0) return node;
        } while(++attempts < 100);
    }
    return ++current_max_node;
}

void create_component(char type, int count) {
    for(int i=0; i<count; i++) {
        Component* c = &components[comp_count++];
        int base_node, peer_node;

        if(type == 'V' || type == 'I') {
            base_node = generate_connected_node(1);
            do {
                peer_node = generate_connected_node(1);
            } while(peer_node == base_node);
        }
        else {
            base_node = generate_connected_node(1);
            peer_node = generate_connected_node(rand()%3 == 0);
        }

        node_usage[base_node]++;
        node_usage[peer_node]++;

        sprintf(c->name, "%c%d", type, comp_count);
        c->node1 = base_node;
        c->node2 = peer_node;

        if(type == 'R') sprintf(c->value, "%d%c", rand()%1000+1, " kM"[rand()%3]);
        else if(type == 'C') sprintf(c->value, "%.3f%c", (rand()%1000)/1000.0f, "pnu"[rand()%3]);
        else if(type == 'L') sprintf(c->value, "%d%c", rand()%1000+1, "nmu"[rand()%3]);
        else sprintf(c->value, "%d", rand()%100+1);
    }
}

void apply_shorts(int n) {
    for(int i=0; i<n; i++) {
        int nodeA = rand() % current_max_node;
        int nodeB;
        do {
            nodeB = rand() % current_max_node;
        } while(nodeB == nodeA);
        
        merge_nodes(nodeA, nodeB);
    }

    for(int i=0; i<comp_count; i++) {
        components[i].node1 = find(components[i].node1);
        components[i].node2 = find(components[i].node2);
    }
}

void create_loops(int n) {
    for(int i=0; i<n; i++) {
        int central = generate_connected_node(1);
        int endpoints[3];
        
        for(int j=0; j<3; j++) {
            endpoints[j] = generate_connected_node(0);
            create_component('R', 1);
            components[comp_count-1].node1 = central;
            components[comp_count-1].node2 = endpoints[j];
        }
        
        create_component('R', 1);
        components[comp_count-1].node1 = endpoints[0];
        components[comp_count-1].node2 = endpoints[1];
    }
}

void apply_open_condition() {
    if(comp_count == 0) return;
    
    Component* c = &components[comp_count-1];
    if(rand()%2) {
        c->node1 = current_max_node + 1;
    } else {
        c->node2 = current_max_node + 1;
    }
    current_max_node++;
}

int main() {
    srand(time(NULL));
    dsu_init();
    Config cfg;
    memset(&cfg, 0, sizeof(Config));

    FILE* fp = fopen("netlistConfig", "r");
    char key[20];
    while(fscanf(fp, "%s", key) != EOF) {
        if(!strcmp(key, "nVoltS")) fscanf(fp, "%d", &cfg.nVoltS);
        else if(!strcmp(key, "nCurrS")) fscanf(fp, "%d", &cfg.nCurrS);
        else if(!strcmp(key, "nR")) fscanf(fp, "%d", &cfg.nR);
        else if(!strcmp(key, "nC")) fscanf(fp, "%d", &cfg.nC);
        else if(!strcmp(key, "nL")) fscanf(fp, "%d", &cfg.nL);
        else if(!strcmp(key, "nShort")) fscanf(fp, "%d", &cfg.nShort);
        else if(!strcmp(key, "nLoop")) fscanf(fp, "%d", &cfg.nLoop);
        else if(!strcmp(key, "bOpen")) fscanf(fp, "%d", &cfg.bOpen);
    }
    fclose(fp);

    create_component('V', cfg.nVoltS);
    create_component('I', cfg.nCurrS);
    create_component('R', cfg.nR);
    create_component('C', cfg.nC);
    create_component('L', cfg.nL);

    apply_shorts(cfg.nShort);
    create_loops(cfg.nLoop);
    if(cfg.bOpen) apply_open_condition();

    fp = fopen("netlistDump.sp", "w");
    for(int i=0; i<comp_count; i++) {
        fprintf(fp, "%s %s %d %d %s\n",
            components[i].type,
            components[i].name,
            components[i].node1,
            components[i].node2,
            components[i].value);
    }
    fclose(fp);

    return 0;
}