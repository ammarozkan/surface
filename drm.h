#include <xf86drm.h>
#include <xf86drmMode.h>

struct fb_dumb {
	uint32_t id,		// DRM object ID
		 width,		
		 height,
		 stride,
		 handle;	// driver-specific handle
	uint64_t size;		// size of mapping

	uint8_t *data;		// memory mapped data we can write to
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

static inline struct fb_dumb*
createDumbFrameBuffer(int drm_fd,drmModeConnector* conn)
{
	uint32_t width = conn->modes[0].hdisplay,
		 height = conn->modes[0].vdisplay;
	struct drm_mode_create_dumb create = {
		.width = width,
		.height = height,
		.bpp = 32,
	};

	int ioctlret;

	drmIoctl(drm_fd,DRM_IOCTL_MODE_CREATE_DUMB,&create);

	struct fb_dumb* fb = malloc(sizeof(struct fb_dumb));
	fb->handle = create.handle;
	fb->stride = create.pitch;
	fb->size = create.size;
	fb->width = width;
	fb->height = height;

	uint32_t handles[4] = { fb->handle },
		 strides[4] = { fb->stride },
		 offsets[4] = { 0 };
	drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_XRGB8888,
			handles, strides, offsets, &fb->id, 0);

	struct drm_mode_map_dumb map = { .handle = fb->handle };
	ioctlret = drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
	if (ioctlret < 0) perror("DRM_IOCTL_MODE_MAP_DUMB");
	
	fb->data = mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED,
			drm_fd, map.offset);
	return fb;
}

