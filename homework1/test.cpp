#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;



class CircuitGraph {
    private:
        unordered_map<int, vector<int>> adjList; // 邻接表
        unordered_set<int> visited; // 访问标记
    
    public:
        void addConnection(int node1, int node2) {
            adjList[node1].push_back(node2);
            adjList[node2].push_back(node1); // 无向图
        }
    
        void dfs(int node, vector<int>& subCircuit) {
            visited.insert(node);
            subCircuit.push_back(node);
            for (int neighbor : adjList[node]) {
                if (visited.find(neighbor) == visited.end()) {
                    dfs(neighbor, subCircuit);
                }
            }
        }
    
        vector<vector<int>> findIndependentCircuits() {
            vector<vector<int>> circuits;
            for (const auto& pair : adjList) {
                int node = pair.first;
                if (visited.find(node) == visited.end()) {
                    vector<int> subCircuit;
                    dfs(node, subCircuit);
                    circuits.push_back(subCircuit);
                }
            }
            return circuits;
        }
    };
    

int main() 

{   FILE *netlist = fopen("netlistDump.sp", "r");
    if (!netlist) {
        perror("Failed to open config file");
        return 1;
    }

    CircuitGraph circuit;

    int nVoltS = 0, nCurrS = 0, nR = 0, nC = 0, nL = 0, 
        nShort = 0, nLoop = 0, bOpen = 0;

    while (!feof(netlist)) {
        char key[20], value[20];
        int node1, node2;

        if (fscanf(netlist, "%s %d %d %s", key, &node1, &node2, value) == 4) {
            if (key[0] == 'V') {
                nVoltS++;
                if (node1 == node2) {
                    nShort++;
                }
            }
            else if (key[0] == 'I') {
                nCurrS++;
                if (node1 == node2){
                    nShort++;
                }
            }
            else if (key[0] == 'R') nR++;
            else if (key[0] == 'C') nC++;
            else if (key[0] == 'L') nL++;
        }

        if (node1) {
            if (node2) 
                circuit.addConnection(node1, node2);
            else
                circuit.addConnection(node1, node1);
        }
        else {
            if (node2) 
                circuit.addConnection(node2, node2);
        }
    }
    fclose(netlist);

    vector<vector<int>> independentCircuits = circuit.findIndependentCircuits();
    for (const auto& circuit : independentCircuits) nLoop++;

    cout << "nVoltS" << '\t' << nVoltS << endl;
    cout << "nCurrS" << '\t' << nCurrS << endl;
    cout << "nR" << '\t' << nR << endl;
    cout << "nC" << '\t' << nC << endl;
    cout << "nL" << '\t' << nL << endl;

    cout << "nShort" << '\t' << nShort << endl;
    cout << "nLoop" << '\t' << nLoop << endl;
    cout << "bOpen" << '\t' << bOpen << endl;
    

    return 0;
}