
#include "p2QuadTree.h"

bool Contains(const SDL_Rect& a, const SDL_Rect& b)
{
	return (a.x <= b.x &&
			a.x + a.w >= b.x + b.w &&
			a.y <= b.y &&
			a.y + a.h >= b.y + b.h);
}

bool Intersects(const SDL_Rect& a, const SDL_Rect& b)
{
	return SDL_HasIntersection(&a, &b) == SDL_TRUE;
	/*return (a.x < b.x + b.w &&
			a.x + a.w > b.x &&
			a.y < b.y + b.h &&
			a.h + a.y > b.y);*/
}