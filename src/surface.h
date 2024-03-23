#ifndef SURFACE_H
#define SURFACE_H

#include <string.h>
#include "mathematics.h"
#include "usurf/usurfTypes.h"

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

struct MenuBar;

struct ContextMenu {
	char* preview;
	uint8_t segmentCount;
	struct ContextMenu* segments;

	int (*function)(void*);
};

struct FunctionalContextMenu {
	char* preview;
	uint8_t segmentCount;
	
	void* data;
	int (*function)(void* data);
};

struct QuickCursor {
	uint32_t x, y;
};

struct Cursor {
	uint32_t x, y;
	void* currentContextMenu;
};

struct WindowElement {
	uint32_t x,y,w,h;
	uint32_t id;
	void* data;
};

struct Window {
	struct Window* prev;
	
	SurfaceIdType id;

#define SURFACE_WINDOW_STRICT 1	// using services to build the program
#define SURFACE_WINDOW_IMAGER 2	// Requesting simple gl functions from server
#define SURFACE_WINDOW_FREE 4	// Getting a context (OpenGL, Vulkan)
	uint8_t WindowType;
	uint32_t sx, sy, ex, ey;

	struct MenuBar* menubar;

	int clientsocket;
	
	struct Window* next;
	// ... 
	// type specific things.
};

struct StrictWindow {
	struct Window generics; // For memory placement.

	uint32_t buttonCount;
	struct WindowElement** buttons;

	uint32_t textCount;
	struct WindowElement** texts;
	uint8_t bckr_colour[4];
};

/* // Future Thing.
struct FreeWindow {
	struct Window generics; // MemPlacement
	// EGL context. make it sharable, send it.
};
*/

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

struct MenuBar {
	uint8_t segmentCount;
	struct ContextMenu* segments;
};

struct Surface {
	struct QuickCursor cursor;
	struct Window* focus;	
	// The Last Window from wins or NULL as "Desktop"
	
	struct Window* wins;
	struct Window* lastWindow;

	struct MenuBar* menubar;

	struct DesktopContextMenu dcm;

	struct Grab grab;

	// we could have a reqest list here. 'queue' like thing.
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
	result->next = NULL;
	result->clientsocket = NULL;

	return result;
}

struct StrictWindow*
createStrictWindow(uint32_t sx, uint32_t sy, 
	     uint32_t ex, uint32_t ey)
{
	uint8_t white[4] = {0xff,0xff,0xff,0xff};
	
	printf("STRICTWINDOWCREATION!\n");
	struct Window* result = malloc(sizeof(struct StrictWindow));
	result->prev = NULL;
	result->id = surfaceGetId();
	result->sx = sx;
	result->sy = sy;
	result->ex = ex;
	result->ey = ey;
	result->next = NULL;
	result->clientsocket = NULL;

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

void surfaceWindowEntry(struct Surface* surface, 
		struct usurfClientEntry* entry)
{
	surfaceAddWindow(surface, 50, 50, 50+entry->width, 50+entry->height);
}

void surfaceAddButton(struct StrictWindow* window, uint32_t id,
		uint8_t panel,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, char* text)
{
	struct WindowElement** newbuttons = 
		malloc(sizeof(void*)*(window->buttonCount + 1));
	memcpy(newbuttons, window->buttons, sizeof(void*)*window->buttonCount);

	struct WindowElement* button = malloc(sizeof(struct WindowElement));
	button->x = x;
	button->y = y;
	button->w = w;
	button->h = h;

	button->data = malloc(strlen(text)+1);
	memcpy(button->data, text, strlen(text)+1);

	button->id = id;
	free(window->buttons);
	window->buttons = newbuttons;
}

void surfaceAddText(struct StrictWindow* window,
		uint8_t panel,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, char* text)
{
	struct WindowElement** newtexts = 
		malloc(sizeof(void*)*(window->buttonCount + 1));
	memcpy(newtexts, window->texts, sizeof(void*)*window->textCount);

	struct WindowElement* elm = malloc(sizeof(struct WindowElement));
	elm->x = x;
	elm->y = y;
	elm->w = w;
	elm->h = h;

	elm->data = malloc(strlen(text)+1);
	memcpy(elm->data, text, strlen(text)+1);

	elm->id = 0;
	free(window->texts);
	window->texts = newtexts;
}

void
surfaceFreeWindowElement(struct WindowElement* we)
{
	free(we->data);
	free(we);
}

void
surfaceFreeWindowElements(struct WindowElement** we, uint32_t count)
{
	for(uint32_t i = 0 ; i < count ; i += 1) {
		surfaceFreeWindowElement(we[i]);
	}
	free(we);
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
	if(window == surface->lastWindow) 
		surface->menubar = NULL;

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
	uint32_t start = 10, space = 5, height = 10, radius = 6;
	x = x - window->sx;
	y = window->sy - y;

	return isInsideCircle(x, y, start + radius, height, radius);
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

	surface->menubar = window->menubar;
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

	printf("Surface Click 0x%x!\n", code);

	uint32_t x = surface->cursor.x, y = surface->cursor.y;
	
	SURF_ITERATE(struct Window, surface->wins, win)
	{
		if(win->sx < x && x < win->ex &&
			win->sy - WINDOW_ROOF_THICKNESS < y 
			&& y < win->ey)
		{
			clickWindow(surface, win, code);
			return;
		}
	}

	
	if(code == 273) {
		surface->dcm.posx = x; surface->dcm.posy = y;
		surface->dcm.info |= SURFACE_CONTEXT_ACTIVE;
	} else surface->dcm.info &= ~SURFACE_CONTEXT_ACTIVE;
	printf("Desktop Click Everybody!\n");
	surface->menubar = NULL;
	return;
}

// These basic functions will be useful in drag & drop operations.
// inlines could be removed in final version. for now, inline is good.
static inline void
surfaceMoveDragX(struct Surface* surface, int move)
{
	if(surface->grab.type == SURFACE_GRAB_WINDOW)
	{
		struct Window* grabbed = surface->grab.ptr;
		grabbed->sx += move;
		grabbed->ex += move;
	}
}

static inline void
surfaceMoveDragY(struct Surface* surface, int move)
{
	if(surface->grab.type == SURFACE_GRAB_WINDOW)
	{
		struct Window* grabbed = surface->grab.ptr;
		grabbed->sy += move;
		grabbed->ey += move;
	}
}

static inline void 
surfaceSetCursorX(struct Surface* surface, uint32_t x)
{
	surface->cursor.x = x;
	surfaceMoveDragX(surface, surface->cursor.x - surface->grab.x);
	surface->grab.x = x;
}

static inline void 
surfaceSetCursorY(struct Surface* surface, uint32_t y)
{
	surface->cursor.y = y;
	surfaceMoveDragY(surface, surface->cursor.y - surface->grab.y);
	surface->grab.y = y;
}

static inline void
surfaceSetCursor(struct Surface* surface, uint32_t x, uint32_t y)
{
	surface->cursor.x = x;
	surface->cursor.y = y;
}

static inline void
surfaceMoveCursorX(struct Surface* surface, int32_t value)
{
	surface->cursor.x += value;
	surfaceMoveDragX(surface, value);
	//printf("Cursor:%u,%u\n", surface->cursor.x, surface->cursor.y);
}

static inline void
surfaceMoveCursorY(struct Surface* surface, int32_t value)
{
	surface->cursor.y += value;
	surfaceMoveDragY(surface, value);
}

/*
 *
 *  Server Request Handlers
 *
 */

void
surfaceWorkOnRequest(struct Surface* surface, struct Window* win)
{
	struct usurfServerInterface interface;
	interface.window = win;
	interface.addText = surfaceAddText;
	interface.addButton = surfaceAddButton;
	usurfServerRecvMsg(win->clientsocket, &interface);
}

void
surfaceLookupRequests(struct Surface* surface) 
{

	SURF_ITERATE(struct Window, surface->wins, win)
	{
		surfaceWorkOnRequest(surface, win);
	}
}

void
surfaceLookupClients(struct Surface* surface,int server_fd)
{
	int clientsocket = usurfServerLookupClients(server_fd);

	if(clientsocket == -1 || clientsocket == -2) return;
	
	struct usurfClientEntry* req = 
		usurfServerGetEntry(clientsocket);
	printf("windowEntry:0x%x\n",req);
	
	if(req==NULL) {
		printf("window entry was empty\n");
		return;
	} else if(req->type != USURF_SURFACEAPI) {
		printf("no surface api? apicode:0%x\n",req->type);
		return;
	} else {
		printf("window entry!\n");
	}
	
	struct Window* window = createStrictWindow(150, 150,
			150 + req->width,
			150 + req->height);
	
	window->clientsocket = clientsocket;
	
	surfaceAddSpecificWindow(surface, window);
	printf("Adding new window.\n");

}

#endif
