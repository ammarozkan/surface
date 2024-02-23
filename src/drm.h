#include <xf86drm.h>
#include <xf86drmMode.h>
#include <errno.h>
#include <poll.h>

struct fb_dumb {
	uint32_t id,		// DRM object ID
		 width,		
		 height,
		 stride,
		 handle;	// driver-specific handle
	uint64_t size;		// size of mapping

	uint8_t *data;		// memory mapped data we can write to
};

struct DrmSystem {
	uint32_t crtc_id;
	drmModeRes* resources;
	drmModeConnector* connector;

};

static const char*
conn_str(uint32_t conn_type)
{
	switch(conn_type)
	{
		case DRM_MODE_CONNECTOR_Unknown:	return "Unknown";
		case DRM_MODE_CONNECTOR_VGA:		return "VGA";
		case DRM_MODE_CONNECTOR_DVII:		return "DVI-I";
		case DRM_MODE_CONNECTOR_DVID:		return "DVI-D";
		case DRM_MODE_CONNECTOR_DVIA:		return "DVI-A";
		case DRM_MODE_CONNECTOR_Composite:	return "Composite";
		case DRM_MODE_CONNECTOR_SVIDEO:		return "SVIDEO";
		case DRM_MODE_CONNECTOR_LVDS:		return "LVDS";
		case DRM_MODE_CONNECTOR_Component:	return "Component";
		case DRM_MODE_CONNECTOR_9PinDIN:	return "9PinDIN";
		case DRM_MODE_CONNECTOR_DisplayPort:	return "DP";
		case DRM_MODE_CONNECTOR_HDMIA:		return "HDMI-A";
		case DRM_MODE_CONNECTOR_HDMIB:		return "HDMI-B";
		case DRM_MODE_CONNECTOR_TV:		return "TV";
		case DRM_MODE_CONNECTOR_eDP:		return "eDP";
		case DRM_MODE_CONNECTOR_VIRTUAL:	return "Virtual";
		case DRM_MODE_CONNECTOR_DSI:		return "DSI";
		default:				return "NotDefined";
	}
}


static uint32_t
find_crtc(int drm_fd, drmModeRes* res, drmModeConnector* conn,
		uint32_t* taken_crtcs)
{
	for(i = 0;i<conn->count_encoders;i+=1) {
		drmModeEncoder *enc = 
			drmModeGetEncoder(drm_fd,conn->encoders[i]);
		if (!enc)
			continue;
		for(j = 0;j<res->count_crtcs;j+=1) {
			uint32_t bit = 1 << j;
			// not compatible.
			if ((enc->possible_crtcs & bit) == 0)
				continue;
			// already taken.
			if (*taken_crtcs & bit)
				continue;
			
			drmModeFreeEncoder(enc);
			*taken_crtcs |= bit;
			return res->crtcs[i];
		}
		drmModeFreeEncoder(enc);
	}
	return 0;
}

drmModeConnector*
getCompatibleConnector(int drm_fd, drmModeRes* resource)
{
	drmModeConnector* result = NULL;
	for(i = 0;i<resource->count_connectors;i++) {
		drmModeConnector* conn =
			drmModeGetConnector(drm_fd, resource->connectors[i]);
		if(!conn)
			continue;
		printf("Mr %u is a %s type connector!\n",
				conn->connector_id,
				conn_str(conn->connector_type));
		for(j = 0;j<conn->count_modes;j+=1) {
			drmModeModeInfo info = conn->modes[j];
			printf("Mode %u h/v = %hu/%hu with %u clock.\n",
					j, info.hdisplay, info.vdisplay,
					info.clock);
		}
		if (j>0) {
			result = conn;
			break; continue;
		}
		drmModeFreeConnector(conn);
	}
	return result;
}


struct DrmSystem*
initDRM(int drm_fd)
{
	struct DrmSystem* drmSystem = malloc(sizeof(struct DrmSystem));
	drmModeRes *resources = drmModeGetResources(drm_fd);
	if (!resources) {
		perror("Resources... They doesn't exist...\n\
				Im sorry as hell for you, kid.\n");
		return NULL;
	}

	printf("They're %i connector and %i encoder in the source!\n",
			resources->count_connectors,resources->count_encoders);
	printf("Width is %u to %u and Height is %u to %u.\n",
			resources->min_width,resources->max_width,
			resources->min_height,resources->max_height);
	drmModeConnector *conn = getCompatibleConnector(drm_fd, resources);

	uint32_t taken_crtcs = 0;
	uint32_t crtc_id = find_crtc(drm_fd, resources, conn, &taken_crtcs);
	//drmModeSetCrtc(drm_fd, crtc_id, fb_front->id, 0, 0,
	//		&conn->connector_id, 1, &conn->modes[0]);
	drmSystem->crtc_id = crtc_id;
	drmSystem->resources = resources;
	drmSystem->connector = conn;
	return drmSystem;
}

int
DrmSystemEnableFlip(int drm_fd, struct DrmSystem* drmSystem,
		uint32_t fb_id,void* data)
{
	int ret = drmModePageFlip(drm_fd, drmSystem->crtc_id, fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, data);
	if(ret < 0) perror("drmModePageFlip error on DrmSystemEnableFlip");
	return ret;
}
int
drmVSyncFlip(int drm_fd, 
		void (*page_flip_handler)(int, unsigned, unsigned, unsigned, void*))
{
	/*	
	struct pollfd pollfd = {
		.fd = drm_fd,
		.events = POLLIN,
	};
	int ret = poll(&pollfd, 1, 0);
	if(ret < 0 && errno != "EAGAIN") {
		perror("poll");
		return -1;
	}
	if (pollfd.revents & POLLIN) {
		drmEventContext context = {
			.version = DRM_EVENT_CONTEXT_VERSION,
			.page_flip_handler = page_flip_handler,
		};
		if(drmHandleEvent(drm_fd,&context) < 0) {
			perror("drmHandleEvent");
			return -1;
		}
	}
	*/
	drmEventContext context = {
		.version = DRM_EVENT_CONTEXT_VERSION,
		.page_flip_handler = page_flip_handler,
	};
	drmHandleEvent(drm_fd, &context);
	
}
