#ifndef CONFIG_H
#define CONFIG_H

extern double dT_A, Vth, Tsim;
extern int tA;

void read_config(const char* filename);

#endif /* CONFIG_H */