
#include <math.h>

int isInsideCircle(uint32_t x, uint32_t y, uint32_t cx, uint32_t cy, 
		float range)
{
	uint32_t dx = cx - x;
	uint32_t dy = cy - y;
	return sqrt(dx*dx + dy*dy) < range;
}

