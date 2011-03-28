#ifndef _WORLD
#define _WORLD
double bounds(double number, double low, double high);
void fill(int low, int high, int type);
void perlin(int low, int high, int type);
int build_world();
void perlin_clouds(double level, double scale, double deg);
#endif
