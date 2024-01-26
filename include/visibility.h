#ifndef _VISIBILITY
#define _VISIBILITY

int CubeInFrustum(GameContext *gc, float x, float y, float z, float size);
void ExtractFrustum(GameContext *gc, glm::mat4 view, glm::mat4 projection);
int PointInFrustum(GameContext *gc, float x, float y, float z);
void trimout(GameContext *gc);
int you_can_see(GameContext *gc, int x, int y, int z);
void buildDisplayList(GameContext *gc, glm::mat4 view, glm::mat4 projection);
#endif
