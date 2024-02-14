#ifndef SURFACE_H
#define SURFACE_H

#define SURF_ITERATE(typed,array,as) \
	for(typed* as = array;as!=NULL;as=as->next)

#define FIND_LAST_ELEMENT(as, array) \
	for(as = array;as->next!=NULL;as=as->next)

#define FIND_ELEMENT_BY_ID(as, array, id) \
	for(as = array;as!=NULL||as->id==id;as=as->next)

typedef uint8_t SurfaceIdType; 	// I wanna make it modifiable cuz
			    	// o' optimization. U can put
			    	// here uint8_t if ure not gonna
			   	// open that much windows.
			    	//
			    	// or if ure a bit mad, u can put
			    	// here uint64_t for a multi mega
			    	// super server computer!

struct QuickCursor {
	uint32_t x, y;
};

struct WindowElement {
	uint32_t x,y,wx,wy;
	void (*render)(struct fb_dumb*);
	bool (*onClick)();
	struct WindowElement* next;
};

struct Window {
	struct Window* prev;
	SurfaceIdType id;
	uint32_t sx, sy, ex, ey;
	uint32_t elementCount;
	struct WindowElement* elements;
	struct Window* next;
};

struct Surface {
	struct Window* focus;	// The Last Window from wins or NULL as "Desktop"
	struct Window* wins;
};

SurfaceIdType
surfaceGetId()
{
	static SurfaceIdType staticIdResult = 0;
	staticIdResult+=1;
	return staticIdResult-1; // or staticIdResult++ if that would be cpp
}

struct Surface*
createSurface()
{
	struct Surface* result = malloc(sizeof(struct Surface));
	result->wins = NULL;
	return result;
}

struct Window*
createWindow(uint32_t sx, uint32_t sy, 
	     uint32_t ex, uint32_t ey)
{
	struct Window* result = malloc(sizeof(struct Window));
	result->prev = NULL;
	result->id = surfaceGetId();
	result->sx = sx;
	result->sy = sy;
	result->ex = ex;
	result->ey = ey;
	result->elementCount = 0;
	result->elements = NULL;
	result->next = NULL;
	return result;
}

void
surfaceAddWindow(struct Surface* surface,
			uint32_t sx, uint32_t sy, 
	     		uint32_t ex, uint32_t ey)
{
	struct Window** lastnull;
	for(lastnull = &surface->wins ; *lastnull!=NULL ; lastnull=&(*lastnull)->next);
					/// OHOHOHHHHMMMMMMM I loved that.
	*lastnull = createWindow(sx,sy,ex,ey);
}

void
surfacePutWindowTop(struct Window* window)
{
	struct Window* last;
	FIND_LAST_ELEMENT(last,window);
	if(last == window) return;

	struct Window* prev = window->prev;
	struct Window* next = window->next;
	prev->next = next;              /// While I enjoy reading it, what if previous one doesnt exist? Well... It will continue to not existing I guess... Is it wrong? I dont have any idea.
	next->prev = prev; // Connecting other ones together. And
			   // extracting our window.
	
	last->next = window;
	window->prev = last;
	window->next = NULL; // connecting our window with the last
			     // window.
	// pure enjoyment.
}

#endif
