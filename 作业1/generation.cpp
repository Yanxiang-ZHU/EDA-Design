#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_NODES 500000

int parent[MAX_NODES];

// Union-Find
int find(int x) {
    if (parent[x] != x) parent[x] = find(parent[x]);
    return parent[x];
}

void unite(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);
    if (rootX != rootY) parent[rootX] = rootY;
}

typedef struct {
    int *nR_arr;
    int *nL_arr;
    int *nC_arr;
    int *nShort_arr;
    int *nVoltS_arr;
    int *nCurrS_arr;
} SplitParams;

typedef struct {
    int ringNode;
    int replace;
} Replace;

typedef struct {
    int nVoltS, nCurrS, nR, nC, nL, nShort, nLoop, bOpen;
} ConfigParams;

// read the file
ConfigParams read_config(const char* config_file) {
    ConfigParams params = {0};
    FILE* config = fopen(config_file, "r");
    if (!config) {
        perror("Failed to open config file");
        return params;
    }

    char key[20];
    int value;
    while (fscanf(config, "%s %d", key, &value) == 2) {
        if (strcmp(key, "nVoltS") == 0) params.nVoltS = value;
        else if (strcmp(key, "nCurrS") == 0) params.nCurrS = value;
        else if (strcmp(key, "nR") == 0) params.nR = value;
        else if (strcmp(key, "nC") == 0) params.nC = value;
        else if (strcmp(key, "nL") == 0) params.nL = value;
        else if (strcmp(key, "nShort") == 0) params.nShort = value;
        else if (strcmp(key, "nLoop") == 0) params.nLoop = value;
        else if (strcmp(key, "bOpen") == 0) params.bOpen = value;
    }
    fclose(config);
    return params;
}

void initialize_parents() {
    for (int i = 0; i < MAX_NODES; i++) {
        parent[i] = i;
    }
}

void write_component(FILE* output, char type, int count, int n1, int n2, int* R, int* C, int* L) {
    switch (type) {
        case 'R': fprintf(output, "R%d %d %d 1k\n", *R, n1, n2); (*R)++; break;
        case 'C': fprintf(output, "C%d %d %d 0.001u\n", *C, n1, n2); (*C)++; break;
        case 'L': fprintf(output, "L%d %d %d 10mH\n", *L, n1, n2); (*L)++; break;
    }
}

void generate_rcl_loop(FILE* output, int nodes[], int* nodeCount, int nR, int nC, int nL, int* R, int* C, int* L) {
    int totalRCL = nR + nC + nL;
    int rCount = 0, cCount = 0, lCount = 0;

    // generate node
    nodes[0] = 0;
    for (int i = 1; i < totalRCL; i++) {
        nodes[i] = (*nodeCount)++;
    }

    // generate R, C, L components
    for (int i = 0; i < totalRCL; i++) {
        int n1 = nodes[i];
        int n2 = nodes[(i+1)%totalRCL];
        char type;

        if (rCount < nR) {
            type = 'R'; rCount++;
        } else if (cCount < nC) {
            type = 'C'; cCount++;
        } else {
            type = 'L'; lCount++;
        }

        write_component(output, type, (type == 'R') ? rCount : (type == 'C') ? cCount : lCount, n1, n2,  R, C, L);
        unite(n1, n2);
    }
}


// add Volt Source
void add_voltage_sources(FILE* output, int nVoltS, int nodes[], int totalRCL, int* VoltS, int* nShort, Replace *replace, int* short_num) {
    // use usedNodes[] to avoid parallel voltage source
    int usedNodes[MAX_NODES] = {0};
    int ground = 0;

    for (int i = 1; i <= nVoltS; i++) {
        int attempts = 0;
        int ringNode;
        
        do {
            ringNode = nodes[rand() % totalRCL];
            attempts++;
            if (attempts > 100 * totalRCL) {
                fprintf(stderr, "Error: Cannot find unique node for V%d\n", i);
                return;
            }
        } while (usedNodes[ringNode] == 1 || ringNode == 0);
        usedNodes[ringNode] = 1;
        fprintf(output, "V%d %d %d 10V\n", *VoltS, ground, ringNode);
        (*VoltS)++;
        if (*nShort) {
            replace[(*short_num)].ringNode = ringNode;
            replace[(*short_num)].replace = ground;
            (*short_num)++;
            (*nShort)--;
        } 
        unite(ground, ringNode);
    }
}

// add Current Source
void add_current_sources(FILE* output, int nCurrS, int nodes[], int totalRCL, int* CurrS, int* nShort, Replace *replace, int* short_num) {
    for (int i = 1; i <= nCurrS; i++) {
        int n1, n2;
        int attempts = 0;
        // avoid series current source
        do {
            n1 = nodes[rand() % totalRCL];
            n2 = nodes[rand() % totalRCL];
            attempts++;
            if (attempts > 100 * totalRCL) {
                fprintf(stderr, "Error: Cannot find unique node for I%d\n", i);
                return;
            }
        } while (n1 == n2);
        fprintf(output, "I%d %d %d 5mA\n", *CurrS, n1, n2);
        (*CurrS)++;
        if (*nShort) {
            replace[(*short_num)].ringNode = n2;
            replace[(*short_num)].replace = n1;
            (*short_num)++;
            (*nShort)--;
        }
        unite(n1, n2);
    }
}


int* random_split(int total, int parts) {
    int *result = (int *) calloc(parts, sizeof(int));
    for(int i = 0; i < total; i++){
        result[rand() % parts]++;
    }
    return result;
}


int check_conditions(int nR, int nL, int nC, int nShort, int nVoltS, int nCurrS) {
    return (nVoltS + nCurrS >= nShort) && (nR + nL + nC >= 2) && (nR + nL + nC >= nCurrS) && (nR + nL + nC - 1 >= nVoltS);
}

SplitParams classify_components(ConfigParams params, int loop_num) {
    SplitParams split = {0};
    srand(time(NULL));
    int total_power = params.nVoltS + params.nCurrS;
    if (total_power < params.nShort) {
        fprintf(stderr, "Error: Insufficient power sources (%d < %d)\n", total_power, params.nShort);
        return split;
    }
    if ((params.nR + params.nL + params.nC) < 2 * loop_num) {
        fprintf(stderr, "Error: Insufficient components (%d < %d)\n", params.nR + params.nL + params.nC, 2 * loop_num);
        return split;
    }
    
    split.nR_arr = (int *)malloc(loop_num * sizeof(int));
    split.nL_arr = (int *)malloc(loop_num * sizeof(int));
    split.nC_arr = (int *)malloc(loop_num * sizeof(int));
    split.nShort_arr = (int *)malloc(loop_num * sizeof(int));
    split.nVoltS_arr = (int *)malloc(loop_num * sizeof(int));
    split.nCurrS_arr = (int *)malloc(loop_num * sizeof(int));
    
    for (int attempt = 0; attempt < 100000; attempt++) {
        int *R = random_split(params.nR, loop_num);
        int *L = random_split(params.nL, loop_num);
        int *C = random_split(params.nC, loop_num);
        int *Vs = random_split(params.nVoltS, loop_num);
        int *Is = random_split(params.nCurrS, loop_num);
        int *Shorts = random_split(params.nShort, loop_num);

        int valid = 1;
        for (int i = 0; i < loop_num; i++) {
            if (!check_conditions(R[i], L[i], C[i], Shorts[i], Vs[i], Is[i])) {
                valid = 0;
                break;
            }
        }

        if (valid) {
            memcpy(split.nR_arr, R, loop_num * sizeof(int));
            memcpy(split.nL_arr, L, loop_num * sizeof(int));
            memcpy(split.nC_arr, C, loop_num * sizeof(int));
            memcpy(split.nVoltS_arr, Vs, loop_num * sizeof(int));
            memcpy(split.nCurrS_arr, Is, loop_num * sizeof(int));
            memcpy(split.nShort_arr, Shorts, loop_num * sizeof(int));
            
            free(R); free(L); free(C);
            free(Vs); free(Is); free(Shorts);
            return split;
        }
        
        free(R); free(L); free(C);
        free(Vs); free(Is); free(Shorts);
    }
    
    fprintf(stderr, "Error: Failed to find valid distribution after many attempts\n");
    return split;
}

void process_netlist(const char *input_file, const char *output_file, Replace replace[], int replace_count) {
    FILE *fin = fopen(input_file, "r");
    FILE *fout = fopen(output_file, "w");
    if (!fin || !fout) {
        fprintf(stderr, "Error: Unable to open file.\n");
        return;
    }
    
    char component[10];
    int node1, node2;
    char value[20];
    
    while (fscanf(fin, "%s %d %d %s", component, &node1, &node2, value) == 4) {
        for (int i = 0; i < replace_count; i++) {
            if (node1 == replace[i].ringNode) {
                node1 = replace[i].replace;
            }
            if (node2 == replace[i].ringNode) {
                node2 = replace[i].replace;
            }
        }
        int ret = fprintf(fout, "%s %d %d %s\n", component, node1, node2, value);
        if (ret < 0) {
            printf("Write error!\n");
        }
    }
    
    fclose(fin);
    fclose(fout);
}

void generate_netlist(const char* config_file, const char* output_file) {
    // read the config
    ConfigParams params = read_config(config_file);
    // we assume that the circuit must have a R/C/L
    if (params.nR + params.nC + params.nL < 1) {
        printf("Error: Not enough R/C/L components\n");
        return;
    }

    FILE* output = fopen(output_file, "w");
    if (!output) {
        perror("Failed to create output file");
        return;
    }

    srand(time(NULL));
    initialize_parents();
    int nodes[MAX_NODES], nodeCount = 1;

    int VoltS = 0;
    int CurrS = 0;
    int R = 0, C = 0, L = 0;

    int loop_num = params.nLoop - params.bOpen;     // we only need a seperated source to one open circuit
    int open_choose = 0;    // open_choose = 1: choose VoltS for the open circuit; open_choose = 2: chose CurrS for the open circuit
    if (params.bOpen) {
        if (params.nVoltS > 0) {
            params.nVoltS--;
            open_choose = 1;
        } else {
            params.nCurrS--;
            open_choose = 2;
        }
    }

    // seperate data to loop_num groups
    SplitParams split = classify_components(params, loop_num);
    Replace replace[params.nShort];
    int short_num = 0;

    for (int loop=0; loop<loop_num; loop++) {
        // generate the core circuit
        generate_rcl_loop(output, nodes, &nodeCount, split.nR_arr[loop], split.nC_arr[loop], split.nL_arr[loop], &R, &C, &L);
        add_voltage_sources(output, split.nVoltS_arr[loop], nodes, split.nR_arr[loop] + split.nC_arr[loop] + split.nL_arr[loop], &VoltS, &split.nShort_arr[loop], replace, &short_num);
        add_current_sources(output, split.nCurrS_arr[loop], nodes, split.nR_arr[loop] + split.nC_arr[loop] + split.nL_arr[loop], &CurrS, &split.nShort_arr[loop], replace, &short_num);
        // printf("successful for %d loops!!\n", loop);
    }

    if (params.bOpen) {
        if (open_choose == 1) {
            fprintf(output, "V%d %d %d 10V\n", params.nVoltS, 0, nodeCount);
        } else if (open_choose == 2) {
            fprintf(output, "V%d %d %d 10V\n", params.nCurrS, 0, nodeCount);
        }
        // printf("successful for open case!!\n");
    }
    fclose(output);

    process_netlist("netlistDump_withoutShort", "netlistDump.sp", replace, params.nShort);
}

int main() {
    generate_netlist("netlistConfig", "netlistDump_withoutShort");
    printf("Success!\n");
    return 0;
}