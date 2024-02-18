#include <drm_fourcc.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>


unsigned int i = 0; // global counter. I dont want to
		    // reallocate that thing again and 
		    // again.
unsigned int j = 0; // u know the drill

#include "ezySurface/ezySurfaceServer.h"

#include "drm.h"
#include "surface.h"
#include "cpurender.h"

#include "controlunit.h"


#include <xf86drm.h>
#include <xf86drmMode.h>

#define BETWEEN(a,x,b) (x < b && a < x) // a < x < b

struct drme_specs
{
	char* gpupath;
	char* keyboardpath;
	char* touchscreenpath; 	// my virtual machine works with my mouse like 
			    	// this but its not accurate in a
			    	// real machine i guess.
}; 

struct ProgramStruct {
	struct DrmSystem* drmSystem;
	void* fb;
	struct Surface* surface;
};

struct drme_specs*
argWork(int argc, char* argv[])
{
	struct drme_specs* drmesptr = malloc(sizeof(struct drme_specs));
	drmesptr->gpupath = "/dev/dri/card0";
	for(i = 0;i<argc;i++) {
		if(strcmp(argv[i],"--setgpu") == 0) {
			i+=1; drmesptr->gpupath = argv[i];
			printf("Setting GPU Path to '%s'.\n",argv[i]);
		}else if(strcmp(argv[i],"--setkeyboard") == 0) {
			i+=1; drmesptr->keyboardpath = argv[i];
			printf("Setting Keyboard Path to '%s'.\n",argv[i]);
		}else if(strcmp(argv[i],"--settouchscreen") == 0) {
			i+=1; drmesptr->touchscreenpath = argv[i];
			printf("Setting Touchscreen Path to '%s'.\n",argv[i]);
		}
	}
	return drmesptr;
}

static void
cpu_page_flip_handler(int drm_fd, unsigned sequence, unsigned tv_sec,
		unsigned tv_usec, void* data)
{
	(void)sequence;
	(void)tv_sec;
	(void)tv_usec;

	// render here
	
	static uint32_t clearCounter = 9;

	struct ProgramStruct* programStruct = data;
	struct DrmSystem* drmSystem = programStruct->drmSystem;
	struct Surface* surface = programStruct->surface;

	struct CPUFrameBuffer* fb_cpu = programStruct->fb;
	struct fb_dumb* fb = fb_cpu->fb_back;
	
	
	clearCounter -= 1;
	if(clearCounter == 0) {
		render_dumbbuffer(fb);
		clearCounter = 9;
	}
	render_surface(fb,surface);

	
	if(drmModePageFlip(drm_fd, drmSystem->crtc_id, fb->id, 
				DRM_MODE_PAGE_FLIP_EVENT, data) < 0)
	{
		perror("drmModePageFlip");
	}

	fb_cpu->fb_back = fb_cpu->fb_front;
	fb_cpu->fb_front = fb;
	//DrmSystemEnableFlip(drm_fd,drmSys);
}

uint32_t getChangedRange(uint32_t value, uint32_t old, uint32_t new)
{
	return ((float)new)*((float)value)
			/((float)old);
}

void TouchscreenPositionX(unsigned int value,struct ProgramStruct* data)
{
	uint32_t width = ((struct CPUFrameBuffer*)data->fb)->fb_front->width;
	uint32_t newx = getChangedRange(value, 32000, width);
	surfaceMoveCursorX(data->surface, newx);
}

void TouchscreenPositionY(unsigned int value, struct ProgramStruct* data)
{
	uint32_t height = ((struct CPUFrameBuffer*)data->fb)->fb_front->height;
	uint32_t newy = getChangedRange(value, 32000, height);
	surfaceMoveCursorY(data->surface, newy);
}

void TouchscreenClick(struct ProgramStruct* data,unsigned short code,unsigned int value)
{
	clickSurface(data->surface,code,value);
}

void SuperSpace(struct ProgramStruct* data)
{
	surfaceAddWindow_quick(data->surface);
}

void SuperK(struct ProgramStruct* data)
{
	surfaceMoveMainWindowLeft_quick(data->surface);
}

int
main(int argc, char* argv[])
{
	printf("drme working out.\n");
	
	struct drme_specs* drmesptr = argWork(argc,argv);
	printf("Specs:\n");
	printf("GPU:%s\n",drmesptr->gpupath);
	printf("\n\n");
	
	int drm_fd = open(drmesptr->gpupath, O_RDWR | O_NONBLOCK);
	if (drm_fd < 0) {
		perror("Wait... Where is\nWhere is the GPU???");
		return 1;
	}

	struct DrmSystem* drmSystem = initDRM(drm_fd);
       	printf("DRM Init.\n");	
	printf("CPU Render Init.\n");
	struct ProgramStruct* programStruct = 
		malloc(sizeof(struct ProgramStruct));
	printf("Program Sruct allocated.\n");
	
	programStruct->fb = initCPUDumbs(drm_fd, drmSystem);
	
	printf("Start Render.\n");
	programStruct->drmSystem = drmSystem;
	render_dumbbuffer(((struct CPUFrameBuffer*)programStruct->fb)->fb_back);
	render_dumbbuffer(((struct CPUFrameBuffer*)programStruct->fb)->fb_front);
	
	printf("Surface Main Program Creation.\n");
	struct Surface* surface = createSurface();
	programStruct->surface = surface;
	DrmSystemEnableFlip(drm_fd, drmSystem, ((struct CPUFrameBuffer*)programStruct->fb)->fb_front->id, programStruct);


	printf("Control Unit\n");
	struct ControlUnit controlUnit = 
		cuCreateControlUnit(drmesptr->keyboardpath,drmesptr->touchscreenpath);
	controlUnit.data = programStruct;
	controlUnit.SuperSpace = SuperSpace;
	controlUnit.SuperK = SuperK;
	controlUnit.TouchscreenPositionX = TouchscreenPositionX;
	controlUnit.TouchscreenPositionY = TouchscreenPositionY;
	controlUnit.TouchscreenClick = TouchscreenClick;

	printf("UNIX Server.\n");
	int unixserver = ezySurfaceCreateUnixServer("/surfacedesktop/regulardesktop-0");
	perror("E");
	while(1) {
		// Control
		int cresult = cuEventRead(&controlUnit);
		
		drmVSyncFlip(drm_fd,cpu_page_flip_handler);

		surfaceLookUpClients(surface,unixserver);
		surfaceLookUpRequests(surface);
	}
	
closeprogram:
	drmModeFreeResources(drmSystem->resources);
	close(drm_fd);

	printf("Succesfully exiting from program.\n");
	return 0;

drm_cantopen:
	return 1;
drm_cantgetresources:
	close(drm_fd);
	return 1;
}
