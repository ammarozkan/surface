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

#define SURFACE_GPURENDER
//#define SURFACE_CPURENDER

#include "ezySurface/ezySurfaceServer.h"

#include "drm.h"
#include "surface.h"

#if defined(SURFACE_GPURENDER)
#include "egl.h"
#include "glesrender.h"
#elif defined(SURFACE_CPURENDER)
#include "cpurender.h"
#endif

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
	struct Surface* surface;
	void* fb;

	struct EGL* egl;
	struct GBM* gbm;
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

#if defined(SURFACE_CPURENDER)
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
#elif defined(SURFACE_GPURENDER)
static void
gpu_page_flip_handler(int drm_fd, unsigned sequence, unsigned tv_sec,
		unsigned tv_usec, void* data)
{
	(void)sequence;
	(void)tv_sec;
	(void)tv_usec;

	struct ProgramStruct* programStruct = data;
	struct DrmSystem* drmSystem = programStruct->drmSystem;
	//struct drm_fb* fb = programStruct->fb;
	// render here
	
	render_surface(programStruct->surface);
	eglSwapBuffers(programStruct->egl->display,
		       programStruct->egl->surface);

	//struct drm_fb* newfb = get_drm_fb_from_gbm(drm_fd,
	//		programStruct->gbm);
	
	struct gbm_bo* bo = ((struct drm_fb*)programStruct->fb)->bo;
	struct gbm_bo* next_bo = 
		gbm_surface_lock_front_buffer(programStruct->gbm->surface);
	struct drm_fb* fb = get_drm_fb_from_bo(drm_fd,next_bo);
	
	if(drmModePageFlip(drm_fd, drmSystem->crtc_id, fb->id,
				DRM_MODE_PAGE_FLIP_EVENT, data) < 0)
	{
		perror("drmModePageFlip");
	}

	gbm_surface_release_buffer(programStruct->gbm->surface,
			bo);

	//gbm_surface_release_buffer(programStruct->gbm->surface,
	//		fb->bo);
	
	((struct drm_fb*)programStruct->fb)->bo = next_bo;
	//fb->bo = newfb->bo;
}
#endif

uint32_t getChangedRange(uint32_t value, uint32_t old, uint32_t new)
{
	return ((float)new)*((float)value)
			/((float)old);
}
#if defined(SURFACE_CPURENDER)
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
#elif defined(SURFACE_GPURENDER)
void TouchscreenPositionX(unsigned int value,struct ProgramStruct* data)
{
	uint32_t width = data->gbm->hdisplay;
	uint32_t newx = getChangedRange(value, 32000, width);
	surfaceMoveCursorX(data->surface, newx);
}

void TouchscreenPositionY(unsigned int value, struct ProgramStruct* data)
{
	uint32_t height = data->gbm->vdisplay;
	uint32_t newy = getChangedRange(value, 32000, height);
	surfaceMoveCursorY(data->surface, newy);
}
#endif
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
	
	struct ProgramStruct* programStruct = 
		malloc(sizeof(struct ProgramStruct));
	printf("Program Sruct allocated.\n");
	
	printf("Surface Main Program Creation.\n");
	struct Surface* surface = createSurface();
	programStruct->surface = surface;
	
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
	
	programStruct->drmSystem = drmSystem;
	
	#if defined(SURFACE_GPURENDER)
	printf("Init GBM\n");
	programStruct->gbm = init_gbm_from_connector(drm_fd,
			drmSystem->connector);
	printf("Init GL.\n");
	programStruct->egl = init_gl(programStruct->gbm);

	initBuffers();
	initPrograms();
	
	printf("PROGRAMSCREENSIZE:%u/%u\n",
			programStruct->gbm->hdisplay,
			programStruct->gbm->vdisplay);	
	initProgramScreenSize(programStruct->gbm->hdisplay,
			programStruct->gbm->vdisplay);

	render_surface(programStruct->surface);	
	
	eglSwapBuffers(programStruct->egl->display,
			programStruct->egl->surface);
	printf("Init frame buffer\n");
	programStruct->fb = get_drm_fb_from_gbm(drm_fd,
		       	programStruct->gbm);


	printf("Start Render.\n");
	DrmSystemEnableFlip(drm_fd, drmSystem, 
			((struct drm_fb*)programStruct->fb)->id,
			programStruct); // GPU RENDER
	#elif defined(SURFACE_CPURENDER)
	programStruct->fb = initCPUDumbs(drm_fd, drmSystem);
	
	// Rendering bckr on start looks better.	
	render_dumbbuffer(((struct CPUFrameBuffer*)programStruct->fb)
			->fb_back);
	render_dumbbuffer(((struct CPUFrameBuffer*)programStruct->fb)
			->fb_front);
	
	DrmSystemEnableFlip(drm_fd, drmSystem, 
			getActiveCPUFBId(programStruct->fb), 
			programStruct); // CPU RENDER
	#endif
	
	


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

		#if defined(SURFACE_GPURENDER)	
		drmVSyncFlip(drm_fd,gpu_page_flip_handler); // GPU
		#elif defined(SURFACE_CPURENDER)
		drmVSyncFlip(drm_fd,cpu_page_flip_handler); // CPU
		#endif

		surfaceLookUpClients(surface,unixserver);
		surfaceLookUpRequests(surface);
	}
	
closeprogram:
	destroyBuffers();
	destroyPrograms();

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
