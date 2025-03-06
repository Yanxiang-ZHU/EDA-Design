#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NODES 100

// 并查集结构
int parent[MAX_NODES];

// 并查集查找（带路径压缩）
int find(int x) {
    if (parent[x] != x) parent[x] = find(parent[x]);
    return parent[x];
}

// 并查集合并
void unite(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);
    if (rootX != rootY) parent[rootX] = rootY;
}

void generate_netlist(const char *config_file, const char *output_file) {
    // 打开配置文件
    FILE *config = fopen(config_file, "r");
    if (!config) {
        perror("Failed to open config file");
        return;
    }

    // 初始化器件数量
    int nVoltS = 0, nCurrS = 0, nR = 0, nC = 0, nL = 0, nShort = 0, nLoop = 0, bOpen = 0;

    // 读取配置文件
    while (!feof(config)) {
        char key[20];
        int value;
        if (fscanf(config, "%s %d", key, &value) == 2) {
            if (strcmp(key, "nVoltS") == 0) nVoltS = value;
            else if (strcmp(key, "nCurrS") == 0) nCurrS = value;
            else if (strcmp(key, "nR") == 0) nR = value;
            else if (strcmp(key, "nC") == 0) nC = value;
            else if (strcmp(key, "nL") == 0) nL = value;
            else if (strcmp(key, "nShort") == 0) nShort = value;
            else if (strcmp(key, "nLoop") == 0) nLoop = value;
            else if (strcmp(key, "bOpen") == 0) bOpen = value;
        }
    }
    fclose(config);

    // 打开输出文件
    FILE *output = fopen(output_file, "w");
    if (!output) {
        perror("Failed to open output file");
        return;
    }

    // 初始化随机数种子
    srand(time(NULL));

    // 初始化并查集
    for (int i = 0; i < MAX_NODES; i++) parent[i] = i;

    int nodes[MAX_NODES], nodeCount = 0;
    int totalRCL = nR + nC + nL;

    // 检查R、C、L总数是否足够形成环路
    if (totalRCL < 2) {
        printf("Error: Not enough R, C, L to form a loop.\n");
        fclose(output);
        return;
    }

    // **第一步：生成R、C、L环路**
    // 创建环路节点
    for (int i = 0; i < totalRCL; i++) {
        nodes[i] = nodeCount++;
    }

    // 连接成环路
    int rCount = 0, cCount = 0, lCount = 0;
    for (int i = 0; i < totalRCL; i++) {
        int n1 = nodes[i];
        int n2 = nodes[(i + 1) % totalRCL]; // 闭合环路
        char type = 'R'; // 默认电阻

        // 按顺序分配R、C、L
        if (rCount < nR) {
            type = 'R';
            rCount++;
        } else if (cCount < nC) {
            type = 'C';
            cCount++;
        } else if (lCount < nL) {
            type = 'L';
            lCount++;
        }

        // 输出器件到网表
        if (type == 'R') {
            fprintf(output, "R%d %d %d 1k\n", rCount, n1, n2);
        } else if (type == 'C') {
            fprintf(output, "C%d %d %d 0.001u\n", cCount, n1, n2);
        } else if (type == 'L') {
            fprintf(output, "L%d %d %d 10mH\n", lCount, n1, n2);
        }

        // 合并节点到并查集
        unite(n1, n2);
    }

    // **第二步：接入电压源**
    for (int i = 1; i <= nVoltS; i++) {
        int ringNode = nodes[rand() % totalRCL]; // 随机选择环路节点
        int ground = 0; // 地节点
        fprintf(output, "V%d %d %d 10V\n", i, ground, ringNode);
        unite(ground, ringNode);
    }

    // **第三步：接入电流源**
    for (int i = 1; i <= nCurrS; i++) {
        int n1 = nodes[rand() % totalRCL];
        int n2 = nodes[rand() % totalRCL];
        while (n1 == n2) { // 确保不是同一节点
            n2 = nodes[rand() % totalRCL];
        }
        fprintf(output, "I%d %d %d 5mA\n", i, n1, n2);
        unite(n1, n2);
    }

    fclose(output);
}

int main() {
    generate_netlist("netlistConfig", "netlistDump.sp");
    printf("✅ 复杂拓扑 SPICE 网表已生成！\n");
    return 0;
}