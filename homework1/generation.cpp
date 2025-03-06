#include <stdio.h>
#include <stdlib.h>

void generate_netlist(const char *config_file, const char *output_file) {
    FILE *config = fopen(config_file, "r");
    if (!config) {
        perror("Failed to open config file");
        return;
    }
    
    int nVoltS, nCurrS, nR, nC, nL, nShort, nLoop;
    int bOpen;
    fscanf(config, "nVoltS %d", &nVoltS);
    fscanf(config, "nCurrS %d", &nCurrS);
    fscanf(config, "nR %d", &nR);
    fscanf(config, "nC %d", &nC);
    fscanf(config, "nL %d", &nL);
    fscanf(config, "nShort %d", &nShort);
    fscanf(config, "nLoop %d", &nLoop);
    fscanf(config, "bOpen %d", &bOpen);
    fclose(config);
    
    FILE *output = fopen(output_file, "w");
    if (!output) {
        perror("Failed to open output file");
        return;
    }
    
    int node = 1; // 从节点1开始
    int firstNode = node;  // 记录起始节点

    // 生成电压源（V1），连接1号节点和地（0）
    fprintf(output, "V1 %d 0 10V\n", node);

    // 依次添加电阻，使电路逐步扩展
    for (int i = 1; i <= nR; i++) {
        fprintf(output, "R%d %d %d 1k\n", i, node, node + 1);
        node++;
    }

    // 依次添加电容，连接到当前节点
    for (int i = 1; i <= nC; i++) {
        fprintf(output, "C%d %d 0 0.001u\n", i, node);
    }

    // 依次添加电感，并连接回起始节点形成环路
    for (int i = 1; i <= nL; i++) {
        fprintf(output, "L%d %d %d 10mH\n", i, node, firstNode);
    }

    fclose(output);
}

int main() {
    generate_netlist("netlistConfig", "netlistDump.sp");
    printf("SPICE netlist generated successfully.\n");
    return 0;
}
