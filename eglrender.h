#ifndef SURFACE_EGLRENDER_H
#define SURFACE_EGLRENDER_H

// this file used for initializing EGL
// for a display in DRM with 
// GBM_FORMAT_XRGB8888

#include <gbm.h>

// We're trying this define later.
// or removing it permanently
//#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct drm_fb {
	struct gbm_bo* bo;
	uint32_t id;
};

struct GBM {
	struct gbm_device* dev;
	struct gbm_surface* surface;
};

struct EGL {
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
};

int active_drm_fd;

struct GBM*
init_gbm(int drm_fd, uint32_t hdisplay, uint32_t vdisplay)
{
	active_drm_fd = drm_fd;
	struct GBM* result = malloc(sizeof(struct GBM));

	result->dev = gbm_create_device(drm_fd);

	result->surface = gbm_surface_create(result->dev,
			hdisplay, vdisplay,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	if(!result->surface) {
		perror("failed to create gbm surface");
	}
	return result;
}

struct GBM*
init_gbm_from_connector(int drm_fd, drmModeConnector* conn)
{
	uint32_t width = conn->modes[0].hdisplay,
		 height = conn->modes[0].vdisplay;
	return init_gbm(drm_fd, width, height);
}

/*
static void
drm_fb_destroy_callback(struct gbm_bo* bo, void* data)
{
	struct drm_fb* fb = data;
	struct gbm_device* gbm = gbm_bo_get_device(bo);

	if(fb->id)
		drmModeRmFB(drm.fd, fb->id);
}
*/

static void
drm_fb_destroy_callback(struct gbm_bo* bo, void* data)
{
	struct drm_fb* fb = data;
	struct gbm_device* gbm = gbm_bo_get_device(bo);

	if(fb->id)
		drmModeRmFB(active_drm_fd, fb->id);
	free(fb);
}

struct drm_fb*
create_fb_from_bo(int drm_fd, struct gbm_bo* bo)
{
	uint32_t width, height, stride, handle;
	int ret;
	struct drm_fb* fb = calloc(1, sizeof(struct drm_fb));

	printf("reaching bo to get values\n");
	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	stride = gbm_bo_get_stride(bo);
	handle = gbm_bo_get_handle(bo).u32;

	printf("drmmodeadd function call\n");
	ret = drmModeAddFB(drm_fd, width, height, 
			24, 32, stride, handle, &fb->id);
	// I dont exactly know what drmModeAddFB does but it looks like to
	// be creating a id for that fb.
	// and reserving it place for drm. is it?
	if(ret) {
		perror("failed to create fb");
		free(fb);
		return NULL;
	}
	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);
	return fb;
}

struct drm_fb*
get_drm_fb_from_bo(int drm_fd, struct gbm_bo* bo)
{
	struct drm_fb *fb = gbm_bo_get_user_data(bo);
	if(fb) return fb;
	return create_fb_from_bo(drm_fd, bo);	
}

struct drm_fb*
get_drm_fb_from_gbm(int drm_fd, struct GBM* gbm)
{
	struct gbm_bo* bo = gbm_surface_lock_front_buffer(gbm->surface);
	printf("Hmmm 0x%x\n",bo);
	if(bo == NULL)
	{
		perror("Wait. gbm_bo is empty. It is NULL");
		return NULL;
	}
	return get_drm_fb_from_bo(drm_fd, bo);
}

static struct EGL*
init_gl(struct GBM* gbm)
{
	struct EGL* egl_result = malloc(sizeof(struct EGL));
	EGLint major, minor, n;
	GLuint vertex_shader, fragment_shader;
	GLint ret;

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	// config for GBM_FORMAT_XRGB8888
	
	static const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display = NULL;
	get_platform_display =
		(void*)eglGetProcAddress("eglGetPlatformDisplayEXT");
	//assert(get_platform_display != NULL);
	if(get_platform_display == NULL) {
		printf("failed to get a display function EGLERR0x%x",eglGetError());
		perror(" ");
		return NULL;
	}

	egl_result->display = get_platform_display(EGL_PLATFORM_GBM_KHR, gbm->dev, NULL);

	if(!eglInitialize(egl_result->display, &major, &minor)) {
		printf("failed to initialize egl EGLERR0x%x",eglGetError());
		perror(" ");
		return NULL;
	}

	printf("Using display %p with EGL version %d.%d\n",
			egl_result->display, major, minor);

	printf("EGL Version \"%s\"\n", eglQueryString(egl_result->display, EGL_VERSION));
	printf("EGL Vendor \"%s\"\n", eglQueryString(egl_result->display, EGL_VENDOR));
	printf("EGL Extensions \"%s\"\n", eglQueryString(egl_result->display, EGL_EXTENSIONS));

	if(!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("failed to bind api EGL_OPENG_ES_API EGLERR0x%x",eglGetError());
		perror(" ");
		return NULL;
		
	}

	if(!eglChooseConfig(egl_result->display, config_attribs, &egl_result->config, 1, &n) || n!=1) {	
		printf("failed to choose config: %d EGLERR0x%x",n, eglGetError());
		perror(" ");
		return NULL;
	}

	egl_result->context = eglCreateContext(egl_result->display, egl_result->config,
			EGL_NO_CONTEXT, context_attribs);
	if(egl_result->context == NULL) {
		printf("failed to create context EGLERR0x%x ",eglGetError());
		perror(" ");
		return NULL;
	}

	egl_result->surface = eglCreateWindowSurface(egl_result->display, egl_result->config,
			gbm->surface, NULL);
	if(egl_result->surface == EGL_NO_SURFACE) {
		printf("failed to create egl surface EGLERROR0x%x",eglGetError());
		perror(" ");
		return NULL;
	}

	eglMakeCurrent(egl_result->display, egl_result->surface, 
			egl_result->surface, egl_result->context);
	
	printf("GL Extensions: \"%s\"\n", glGetString(GL_EXTENSIONS));

	return egl_result;
}



void render_surface(struct Surface* surface)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

#endif
