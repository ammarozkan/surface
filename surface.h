#ifndef SURFACE_H
#define SURFACE_H

#include <string.h>

// Window Style Constants
#define WINDOW_ROOF_THICKNESS 16
#define WINDOW_ROOF_BUTTON_THICKNESS 8
#define WINDOW_GENERIC_ROOF_BUTTON_WIDTH 20
#define WINDOW_SPACE_FROM_START 5
#define WINDOW_SPACE_BETWEEN_ROOF_BUTTONS 5
#define WINDOW_PANIC_BUTTON_WIDTH 8


// Other Generated Constants
#define WINDOW_UNTIL_BUTTON_THICKNESS (WINDOW_ROOF_THICKNESS-WINDOW_ROOF_BUTTON_THICKNESS)/2

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

	uint8_t bckr_colour[4];

	struct WindowElement* elements;
	struct Window* next;

	struct ezySurfaceClient* client;
};

struct DesktopContextMenu {
#define SURFACE_CONTEXT_ACTIVE (1 << 1)
	uint8_t info;
	uint32_t posx, posy;
};

struct Grab {
#define SURFACE_GRAB_UNDEF 0
#define SURFACE_GRAB_WINDOW 1
	uint32_t type;
	void* ptr;
	uint32_t x, y;
};

struct Surface {
	struct QuickCursor cursor;
	struct Window* focus;	
	// The Last Window from wins or NULL as "Desktop"
	
	struct Window* wins;
	struct Window* lastWindow;

	struct DesktopContextMenu dcm;

	struct Grab grab;
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
	result->cursor.x = result->cursor.y = 60;
	return result;
}

struct Window*
createWindow(uint32_t sx, uint32_t sy, 
	     uint32_t ex, uint32_t ey)
{
	uint8_t white[4] = {0xff,0xff,0xff,0xff};
	
	printf("WINDOWCREATION!\n");
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
	result->client = NULL;
	memcpy(result->bckr_colour, white, 4);
	return result;
}


void
surfaceAddSpecificWindow(struct Surface* surface,
		struct Window* window)
{
	if(surface->lastWindow == NULL) {
		printf("First!\n");
		surface->wins = window;
		surface->lastWindow = window;
		surface->lastWindow->next = surface->lastWindow->prev = NULL;
	} else {
		surface->lastWindow->next = window;
		window->prev = surface->lastWindow;
		surface->lastWindow = window;
	}
}

void
surfaceAddWindow(struct Surface* surface,
			uint32_t sx, uint32_t sy, 
	     		uint32_t ex, uint32_t ey)
{
	surfaceAddSpecificWindow(surface, createWindow(sx,sy,ex,ey));
}


void 
surfaceAddWindow_quick(struct Surface* surface)
{
	static uint32_t lastPos = 0;
	
	lastPos += 30; 
	if(lastPos > 1000) lastPos = 30;

	surfaceAddWindow(surface, lastPos, lastPos, 
			400+lastPos, 300+lastPos);
	
	surface->focus = surface->lastWindow;
}

void 
surfaceMoveMainWindowLeft_quick(struct Surface* surface)
{
	surface->focus->sx += 10;
	surface->focus->ex += 10;
}

void
surfacePutWindowTop(struct Surface* surface, struct Window* window)
{
	if(surface->lastWindow == window) return;

	struct Window* prev = window->prev;
	struct Window* next = window->next;
	
	if (prev != NULL) prev->next = next;
	else surface->wins = next;
	next->prev = prev; // Connecting other ones together. And
			   // extracting our window.
	surface->lastWindow->next = window;
	window->prev = surface->lastWindow;
	window->next = NULL; // connecting our window with the last
			     // window.
	surface->lastWindow = window; // Boom, now its the last one.
	// pure enjoyment.
}


/*
 *
 *  Controls
 *
 */

void
surfaceCloseWindow(struct Surface* surface, struct Window* window)
{
	// surfaceDestroyWindowElements()
	if(window->prev != NULL) window->prev->next = window->next;
	else surface->wins = window->next;

	if(window->next != NULL) {
		window->next->prev = window->prev;
	} else {
		surface->lastWindow = window->prev;
		surface->focus = surface->lastWindow;
	}
	free(window);
}

int
constantWindowCloseButtonControl(uint32_t x, uint32_t y, struct Window* window)
{
	int ycontrol = window->sy - WINDOW_UNTIL_BUTTON_THICKNESS - WINDOW_ROOF_BUTTON_THICKNESS < y && y < window->sy - WINDOW_UNTIL_BUTTON_THICKNESS;

	int xcontrol = window->sx + WINDOW_SPACE_FROM_START < x && x < window->sx + WINDOW_GENERIC_ROOF_BUTTON_WIDTH;

	return xcontrol && ycontrol;
}

void
clickWindowInside(struct Surface* surface, 
		struct Window* window, unsigned short code)
{
	// window send event.
}

void
clickWindow(struct Surface* surface, struct Window* window, 
		unsigned short code)
{

	surface->dcm.info &= ~SURFACE_CONTEXT_ACTIVE;
	surfacePutWindowTop(surface,window);
	surface->focus = window;
	
	if(constantWindowCloseButtonControl(surface->cursor.x, 
			surface->cursor.y, window)) {
		surfaceCloseWindow(surface,window);
	} else if(window->sx < surface->cursor.x && 
			surface->cursor.x < window->ex &&
			window->sy < surface->cursor.y &&
			surface->cursor.y < window->ey){
		// Inside
		clickWindowInside(surface, window, code);
	}
	else {
		// Border
		surface->grab.type = SURFACE_GRAB_WINDOW;
		surface->grab.ptr = window;
		surface->grab.x = surface->cursor.x;
		surface->grab.y = surface->cursor.y;
	}
}

void
grabEnd(struct Surface* surface, unsigned short code)
{
	surface->grab.type = SURFACE_GRAB_UNDEF;
	surface->grab.ptr = NULL;
}

void
clickSurface(struct Surface* surface, unsigned short code,
		unsigned int value)
{
	printf("Click event. %u\n",value);
	if(value == 0) {
		grabEnd(surface,code);
		return;
	}

	printf("Surface Click!\n");

	uint32_t x = surface->cursor.x, y = surface->cursor.y;
	
	SURF_ITERATE(struct Window, surface->wins, win)
	{
		if(win->sx < x && x < win->ex &&
			win->sy - WINDOW_ROOF_THICKNESS < y 
			&& y < win->ey)
		{
			clickWindow(surface, elmWindow, code);
			return;
		}
	}

	
	if(code == 273) {
		surface->dcm.posx = x; surface->dcm.posy = y;
		surface->dcm.info |= SURFACE_CONTEXT_ACTIVE;
	} else surface->dcm.info &= ~SURFACE_CONTEXT_ACTIVE;
	printf("Desktop Click Everybody!\n");
	return;
}

// These basic functions will be useful in drag & drop operations.
// inlines could be removed in final version. for now, inline is good.
static inline void 
surfaceMoveCursorX(struct Surface* surface, uint32_t x)
{
	surface->cursor.x = x;
	if(surface->grab.type == SURFACE_GRAB_WINDOW)
	{
		int move = surface->cursor.x - surface->grab.x;
		struct Window* grabbed = surface->grab.ptr;
		grabbed->sx += move;
		grabbed->ex += move;
	}
	surface->grab.x = x;
}

static inline void 
surfaceMoveCursorY(struct Surface* surface, uint32_t y)
{
	surface->cursor.y = y;
	if(surface->grab.type == SURFACE_GRAB_WINDOW)
	{
		int move = surface->cursor.y - surface->grab.y;
		struct Window* grabbed = surface->grab.ptr;
		grabbed->sy += move;
		grabbed->ey += move;
	}
	surface->grab.y = y;
}

static inline void
surfaceMoveCursor(struct Surface* surface, uint32_t x, uint32_t y)
{
	surface->cursor.x = x;
	surface->cursor.y = y;
}


/*
 *
 *  Server Request Handlers
 *
 */

void
surfaceWorkOnRequest(struct Surface* surface, struct Window* win)
{
	struct ProgramRequestBase* base 
		= ezySurfaceReceiveRequestBase(win->client);
	if (base==NULL) return;
	
	switch (base->requestType)
	{
	case PROGRAM_REQUEST_KILL:
		printf("Kill Request\n");
		struct ProgramRequest_kill* kill_req
			= ezySurfaceReceiveKillRequest(win->client);
		if(kill_req!=NULL) {
			printf("Window Kill\n");
			surfaceCloseWindow(surface,win);
			free(kill_req);
		}
		break;
	default:
		printf("An request can't understanded.\n");
	}
	free(base);
}

void
surfaceLookUpRequests(struct Surface* surface) 
{

	SURF_ITERATE(struct Window, surface->wins, win)
	{
		surfaceWorkOnRequest(surface, win);
	}
}

void
surfaceLookUpClients(struct Surface* surface,int server_fd)
{
	struct ezySurfaceClient* surfaceClient = 
		ezySurfaceLookUpClients(server_fd);
	
	if(surfaceClient == NULL) return;
	
	struct ProgramFirstRequest* req = 
		ezySurfaceFirstRequestReceive(surfaceClient);
	
	if(req==NULL) return;
	
	struct Window* window = createWindow(30, 30,
			30 + req->windowsize_x,
			30 + req->windowsize_y);
	
	window->client = surfaceClient;
	memcpy(window->bckr_colour, req->bckr_colour, 4);
	
	surfaceAddSpecificWindow(surface, window);
}

#endif
