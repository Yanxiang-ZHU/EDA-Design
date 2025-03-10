#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 100000
#define MAX_LINE 100000

int parent[MAX_NODES];
int rank[MAX_NODES];

void init_union_find(int n) {
    for (int i = 0; i < n; i++) {
        parent[i] = i;
        rank[i] = 0;
    }
}

int find(int x) {
    if (parent[x] != x)
        parent[x] = find(parent[x]);
    return parent[x];
}

void union_sets(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);
    if (rootX != rootY) {
        if (rank[rootX] > rank[rootY]) {
            parent[rootY] = rootX;
        } else if (rank[rootX] < rank[rootY]) {
            parent[rootX] = rootY;
        } else {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
    }
}

void parse_netlist(const char *filename, int *nVoltS, int *nCurrS, int *nR, int *nC, int *nL, int *nShort, int *nLoop, int *bOpen) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open netlist file");
        exit(1);
    }
    
    char line[MAX_LINE];
    int last_index = 0;
    *nVoltS = *nCurrS = *nR = *nC = *nL = *nShort = *nLoop = *bOpen = 0;
    
    init_union_find(MAX_NODES);
    int used_nodes[MAX_NODES] = {0};
    
    while (fgets(line, sizeof(line), file)) {
        char type;
        int id, node1, node2;
        if (sscanf(line, "%c%d %d %d", &type, &id, &node1, &node2) < 3) continue;
        
        if (id > last_index) last_index = id;
        
        switch (type) {
            case 'V': (*nVoltS)++; break;
            case 'I': (*nCurrS)++; break;
            case 'R': (*nR)++; break;
            case 'C': (*nC)++; break;
            case 'L': (*nL)++; break;
        }
        
        if ((type == 'V' || type == 'I') && node1 == node2) {
            (*nShort)++;
        }
        
        if (node1 != 0 && node2 != 0) {
            union_sets(node1, node2);
        }
        used_nodes[node1] = 1;
        used_nodes[node2] = 1;
    }
    fclose(file);
    
    int unique_roots[MAX_NODES] = {0};
    for (int i = 1; i < MAX_NODES; i++) {
        if (used_nodes[i]) {
            unique_roots[find(i)] = 1;
        }
    }
    for (int i = 1; i < MAX_NODES; i++) {
        if (unique_roots[i]) {
            (*nLoop)++;
        }
    }
    (*nLoop)--;
    
    FILE *file2 = fopen(filename, "r");
    while (fgets(line, sizeof(line), file2)) {
        char type;
        int id, node1, node2;
        if (sscanf(line, "%c%d %d %d", &type, &id, &node1, &node2) < 3) continue;
        if ((type == 'V' || type == 'I') && node1 == 0 && used_nodes[node2] == 1) {
            int is_isolated = 1;
            FILE *file3 = fopen(filename, "r");
            while (fgets(line, sizeof(line), file3)) {
                int id2, n1, n2;
                if (sscanf(line, "%*c%d %d %d", &id2, &n1, &n2) < 3) continue;
                if (id2 != id && (n1 == node2 || n2 == node2)) {
                    is_isolated = 0;
                    break;
                }
            }
            fclose(file3);
            if (is_isolated) {
                *bOpen = 1;
                break;
            }
        }
    }
    fclose(file2);
}

void compare_with_config(const char *config_file, int nVoltS, int nCurrS, int nR, int nC, int nL, int nShort, int nLoop, int bOpen) {
    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("Failed to open config file");
        exit(1);
    }
    
    int expected[7];
    int expected_bOpen;
    fscanf(file, "nVoltS %d\nnCurrS %d\nnR %d\nnC %d\nnL %d\nnShort %d\nnLoop %d\nbOpen %d", 
           &expected[0], &expected[1], &expected[2], &expected[3], &expected[4], &expected[5], &expected[6], &expected_bOpen);
    fclose(file);
    
    if (nVoltS == expected[0] && nCurrS == expected[1] && nR == expected[2] && nC == expected[3] && 
        nL == expected[4] && nShort == expected[5] && nLoop == expected[6] && bOpen == expected_bOpen) {
        printf("success!!\n");
    } else {
        printf("Mismatch detected!\n");
        printf("Expected: nVoltS=%d, nCurrS=%d, nR=%d, nC=%d, nL=%d, nShort=%d, nLoop=%d, bOpen=%d\n",
               expected[0], expected[1], expected[2], expected[3], expected[4], expected[5], expected[6], expected_bOpen);
        printf("Generated: nVoltS=%d, nCurrS=%d, nR=%d, nC=%d, nL=%d, nShort=%d, nLoop=%d, bOpen=%d\n",
               nVoltS, nCurrS, nR, nC, nL, nShort, nLoop, bOpen);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <netlistDump.sp> <netlistConfig>\n", argv[0]);
        return 1;
    }
    int nVoltS, nCurrS, nR, nC, nL, nShort, nLoop, bOpen;
    parse_netlist(argv[1], &nVoltS, &nCurrS, &nR, &nC, &nL, &nShort, &nLoop, &bOpen);
    compare_with_config(argv[2], nVoltS, nCurrS, nR, nC, nL, nShort, nLoop, bOpen);
    return 0;
}