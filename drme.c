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


#include <xf86drm.h>
#include <xf86drmMode.h>

#define BETWEEN(a,x,b) (x < b && a < x) // a < x < b

struct drme_specs
{
	char* gpupath;
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
		}
	}
	return drmesptr;
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

	drmModeRes *resources = drmModeGetResources(drm_fd);
	if (!resources) {
		perror("Resources... They doesn't exist...\n\
				Im sorry as hell for you, kid.\n");
		return 1;
	}

	printf("They're %i connector and %i encoder in the source!\n",
			resources->count_connectors,resources->count_encoders);
	printf("Width is %u to %u and Height is %u to %u.\n",
			resources->min_width,resources->max_width,
			resources->min_height,resources->max_height);
	drmModeConnector *conn = getCompatibleConnector(drm_fd, resources);

	struct fb_dumb* fb = createDumbFrameBuffer(drm_fd, conn);
	
	uint32_t taken_crtcs = 0;
	uint32_t crtc_id = find_crtc(drm_fd, resources, conn, &taken_crtcs);
	render_dumbbuffer(fb);
	drmModeSetCrtc(drm_fd, crtc_id, fb->id, 0, 0,
			&conn->connector_id, 1, &conn->modes[0]);


	uint32_t thacounter = 1;
	struct Surface* surface = createSurface();
	struct QuickCursor cursor = {.x = 85, .y = 85};
	surfaceAddWindow(surface, 60, 60, 500, 300);
	uint32_t pos = 60;
	while(1) {
		thacounter+=1; thacounter = (thacounter%255);
		render_dumbbuffer(fb);
		cursor.x = thacounter;
		cursor.y = 255 - thacounter;
		render_surface(fb,surface);
		if(thacounter%200 == 0) {
			surfaceAddWindow(surface,pos,pos,pos+500,pos+300);
			pos+=30;
		}
		render_quickcursor(fb,&cursor);
		drmModeSetCrtc(drm_fd, crtc_id, fb->id, 0, 0,
				&conn->connector_id, 1, &conn->modes[0]);
	}
	
closeprogram:
	drmModeFreeResources(resources);
	close(drm_fd);

	printf("Succesfully exiting from program.\n");
	return 0;

drm_cantopen:
	return 1;
drm_cantgetresources:
	close(drm_fd);
	return 1;
}
