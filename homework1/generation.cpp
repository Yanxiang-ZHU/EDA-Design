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
    int node1;  // 修正点：统一使用node1/node2
    int node2;  // 原错误使用的node_peer改为node2
    char value[20];
} Component;

// 全局状态管理
Component components[MAX_COMPONENTS];
int comp_count = 0;
int parent[MAX_NODES];  // 并查集用于节点合并
int node_usage[MAX_NODES] = {0};
int current_max_node = GROUND_NODE;

/* 并查集操作 */
void dsu_init() {
    for(int i=0; i<MAX_NODES; i++) parent[i] = i;
}

int find(int x) {
    return parent[x] = (parent[x] == x ? x : find(parent[x]));
}

void merge_nodes(int a, int b) {
    parent[find(b)] = find(a);
}

/* 拓扑生成核心逻辑 */
int generate_connected_node(int must_exist) {
    if(must_exist || current_max_node == GROUND_NODE) {
        // 强制连接现有节点
        int attempts = 0;
        do {
            int node = rand() % (current_max_node + 1);
            if(node_usage[node] > 0) return node;
        } while(++attempts < 100);
    }
    return ++current_max_node; // 生成新节点
}

void create_component(char type, int count) {
    for(int i=0; i<count; i++) {
        Component* c = &components[comp_count++];
        int base_node, peer_node;

        // 电源类元件需要跨接已有节点
        if(type == 'V' || type == 'I') {
            base_node = generate_connected_node(1);
            do {
                peer_node = generate_connected_node(1);
            } while(peer_node == base_node);
        } 
        // 无源元件至少一端连接现有节点
        else {
            base_node = generate_connected_node(1);
            peer_node = generate_connected_node(rand()%3 == 0); // 30%概率生成新节点
        }

        // 记录节点使用情况
        node_usage[base_node]++;
        node_usage[peer_node]++;

        // 生成元件属性
        sprintf(c->type, "%c", type);
        sprintf(c->name, "%c%d", type, comp_count);
        c->node1 = base_node;
        c->node2 = peer_node;  // 修正点：使用正确的node2成员

        // 生成随机参数
        if(type == 'R') sprintf(c->value, "%d%c", rand()%1000+1, " kM"[rand()%3]);
        else if(type == 'C') sprintf(c->value, "%.3f%c", (rand()%1000)/1000.0f, "pnu"[rand()%3]);
        else if(type == 'L') sprintf(c->value, "%d%c", rand()%1000+1, "nmu"[rand()%3]);
        else sprintf(c->value, "%d", rand()%100+1);
    }
}

/* 特殊功能实现 */
void apply_shorts(int n) {
    for(int i=0; i<n; i++) {
        int nodeA = rand() % current_max_node;
        int nodeB;
        do {
            nodeB = rand() % current_max_node;
        } while(nodeB == nodeA);
        
        merge_nodes(nodeA, nodeB);
    }

    // 应用节点合并到所有元件
    for(int i=0; i<comp_count; i++) {
        components[i].node1 = find(components[i].node1);
        components[i].node2 = find(components[i].node2);  // 修正点
    }
}

void create_loops(int n) {
    for(int i=0; i<n; i++) {
        int central = generate_connected_node(1);
        int endpoints[3];
        
        // 创建三角环
        for(int j=0; j<3; j++) {
            endpoints[j] = generate_connected_node(0);
            create_component('R', 1);
            components[comp_count-1].node1 = central;
            components[comp_count-1].node2 = endpoints[j];  // 修正点
        }
        
        // 闭合环路
        create_component('R', 1);
        components[comp_count-1].node1 = endpoints[0];
        components[comp_count-1].node2 = endpoints[1];  // 修正点
    }
}

void apply_open_condition() {
    if(comp_count == 0) return;
    
    // 选择最后一个元件断开连接
    Component* c = &components[comp_count-1];
    if(rand()%2) {
        c->node1 = current_max_node + 1; // 悬空新节点
    } else {
        c->node2 = current_max_node + 1;  // 修正点
    }
    current_max_node++;
}

/* 主流程 */
int main() {
    srand(time(NULL));
    dsu_init();
    Config cfg;
    memset(&cfg, 0, sizeof(Config));

    // 读取配置文件
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

    // 生成基础元件
    create_component('V', cfg.nVoltS);
    create_component('I', cfg.nCurrS);
    create_component('R', cfg.nR);
    create_component('C', cfg.nC);
    create_component('L', cfg.nL);

    // 应用拓扑规则
    apply_shorts(cfg.nShort);
    create_loops(cfg.nLoop);
    if(cfg.bOpen) apply_open_condition();

    // 输出SPICE网表
    fp = fopen("netlistDump.sp", "w");
    for(int i=0; i<comp_count; i++) {
        fprintf(fp, "%s %s %d %d %s\n",
            components[i].type,
            components[i].name,
            components[i].node1,
            components[i].node2,  // 修正点
            components[i].value);
    }
    fclose(fp);

    return 0;
}