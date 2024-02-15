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
}; 

struct ProgramStruct {
	struct DrmSystem* drmSystem;
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
		}
	}
	return drmesptr;
}

static void
page_flip_handler(int drm_fd, unsigned sequence, unsigned tv_sec,
		unsigned tv_usec, void* data)
{
	(void)sequence;
	(void)tv_sec;
	(void)tv_usec;

	// render here
	
	struct ProgramStruct* programStruct = data;
	struct DrmSystem* drmSystem = programStruct->drmSystem;
	struct Surface* surface = programStruct->surface;

	struct fb_dumb* fb = drmSystem->fb_back;
	
	render_dumbbuffer(fb);
	render_surface(fb,surface);

	
	if(drmModePageFlip(drm_fd, drmSystem->crtc_id, fb->id, 
				DRM_MODE_PAGE_FLIP_EVENT, data) < 0)
	{
		perror("drmModePageFlip");
	}

	drmSystem->fb_back = drmSystem->fb_front;
	drmSystem->fb_front = fb;
	//DrmSystemEnableFlip(drm_fd,drmSys);
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

	struct ProgramStruct* programStruct = malloc(sizeof(struct ProgramStruct));
	
	programStruct->drmSystem = drmSystem;
	struct Surface* surface = createSurface();
	programStruct->surface = surface;
	DrmSystemEnableFlip(drm_fd, drmSystem, programStruct);


	struct ControlUnit controlUnit = 
		cuCreateControlUnit(drmesptr->keyboardpath,"");
	controlUnit.data = surface;
	controlUnit.SuperSpace = surfaceAddWindow_quick;
	controlUnit.SuperK = surfaceMoveMainWindowLeft_quick;
	while(1) {
		// Control
		int cresult = cuEventRead(&controlUnit);	
		
		drmVSyncFlip(drm_fd,page_flip_handler);
		//drmModeSetCrtc(drm_fd, crtc_id, fb->id, 0, 0, &conn->connector_id, 1, &conn->modes[0]);
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
