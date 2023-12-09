#ifndef _VISIBILITY
#define _VISIBILITY
float frustum[6][4];

int CubeInFrustum(float x, float y, float z, float size);
void ExtractFrustum();
int PointInFrustum(float x, float y, float z);
void trimout();
int you_can_see(int x, int y, int z);
void buildDisplayList();
#endif
