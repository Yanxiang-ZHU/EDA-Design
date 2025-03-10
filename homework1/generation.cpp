#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_NODES 10000

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
    int nR, nL, nC, nShort, nVoltS, nCurrS;
} Combination;

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

void write_component(FILE* output, char type, int count, int n1, int n2) {
    switch (type) {
        case 'R': fprintf(output, "R%d %d %d 1k\n", count, n1, n2); break;
        case 'C': fprintf(output, "C%d %d %d 0.001u\n", count, n1, n2); break;
        case 'L': fprintf(output, "L%d %d %d 10mH\n", count, n1, n2); break;
    }
}

void generate_rcl_loop(FILE* output, int nodes[], int* nodeCount, int nR, int nC, int nL) {
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

        write_component(output, type, (type == 'R') ? rCount : (type == 'C') ? cCount : lCount, n1, n2);
        unite(n1, n2);
    }
}

// add Volt Source
void add_voltage_sources(FILE* output, int nVoltS, int nodes[], int totalRCL) {
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
        } while (usedNodes[ringNode] == 1);
        usedNodes[ringNode] = 1;
        fprintf(output, "V%d %d %d 10V\n", i, ground, ringNode);
        unite(ground, ringNode);
    }
}

// add Current Source
void add_current_sources(FILE* output, int nCurrS, int nodes[], int totalRCL) {
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
        fprintf(output, "I%d %d %d 5mA\n", i, n1, n2);
        unite(n1, n2);
    }
}


int check_conditions(int nR, int nL, int nC, int nShort, int nVoltS, int nCurrS) {
    return (nVoltS + nCurrS >= nShort) && (nR + nL + nC >= 2);
}

// void add_combination(Combination **combinations, int *size, int nR, int nL, int nC, int nShort, int nVoltS, int nCurrS) {
//     *combinations = (Combination *) realloc(*combinations, (*size + 1) * sizeof(Combination));
//     if (*combinations == NULL) {
//         printf("Memory allocation failed\n");
//         exit(-1);
//     }
//     (*combinations)[*size].nR = nR;
//     (*combinations)[*size].nL = nL;
//     (*combinations)[*size].nC = nC;
//     (*combinations)[*size].nShort = nShort;
//     (*combinations)[*size].nVoltS = nVoltS;
//     (*combinations)[*size].nCurrS = nCurrS;
//     (*size)++;
// }

// void classify_parameters(ConfigParams params, Combination **combinations, int *size, int loop_num) {
//     *size = 0;
//     *combinations = NULL;

//     if (check_conditions(params.nR, params.nL, params.nC, params.nShort, params.nVoltS, params.nCurrS)) {
//         add_combination(combinations, size, params.nR, params.nL, params.nC, params.nShort, params.nVoltS, params.nCurrS);
//     } else {
//         printf("No valid combination found based on given conditions.\n");
//     }

//     if (*size < loop_num) {
//         printf("Could not find enough (%d) valid combinations.\n", loop_num);
//     }
// }



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
    Combination *combinations = NULL;
    int size = 0;
    classify_parameters(params, &combinations, &size, loop_num);

    for (int loop=0; loop<loop_num; loop++) {
        // generate the core circuit
        generate_rcl_loop(output, nodes, &nodeCount, combinations[loop].nR, combinations[loop].nC, combinations[loop].nL);
        add_voltage_sources(output, combinations[loop].nVoltS, nodes, combinations[loop].nR + combinations[loop].nC + combinations[loop].nL);
        add_current_sources(output, combinations[loop].nCurrS, nodes, combinations[loop].nR + combinations[loop].nC + combinations[loop].nL);
        printf("successful for %d loops!!\n", loop);
    }

    if (params.bOpen) {
        if (open_choose == 1) {
            fprintf(output, "V%d %d %d 10V\n", params.nVoltS + 1, 0, 0);
        } else if (open_choose == 2) {
            fprintf(output, "V%d %d %d 10V\n", params.nCurrS + 1, 0, 0);
        }
        printf("successful for open case!!\n");
    }

    fclose(output);
}

int main() {
    generate_netlist("netlistConfig", "netlistDump.sp");
    printf("Success!\n");
    return 0;
}