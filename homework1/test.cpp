#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


int main() 

{   FILE *netlist = fopen("netlistDump.sp", "r");
    if (!netlist) {
        perror("Failed to open config file");
        return 1;
    }

    int nVoltS = 0, nCurrS = 0, nR = 0, nC = 0, nL = 0, 
        nShort = 0, nLoop = 0, bOpen = 0;

    while (!feof(netlist)) {
        char key[20], value[20];
        int node1, node2;

        if (fscanf(netlist, "%s %d %d %s", key, &node1, &node2, value) == 4) {
            if (key[0] == 'V')  nVoltS++;
            else if (key[0] == 'I') nCurrS++;
            else if (key[0] == 'R') nR++;
            else if (key[0] == 'C') nC++;
            else if (key[0] == 'L') nL++;
        }

        

    }
    fclose(netlist);

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