#ifndef USURF_USURFTYPES_H
#define USURF_USURFTYPES_H

#include <stdint.h>
#include <stddef.h>

// size is 0-255 -> 0-100
struct usurfClientEntry {
	uint32_t zero1, zero2; // skipping wayland
#define USURF_SURFACEAPI 0x01
	uint8_t type;
	uint32_t width, height;
#define USURF_PANEL_HORIZONTAL 0x01
#define USURF_PANEL_VERTICAL 0x02
	/* 
	uint8_t panelType;
	uint8_t phaseCount;
	uint8_t phase_panelCount[4]; uint8_t phase_size[4];
	uint8_t panelSizes[4][4]; // [phaseid][panelid]
	*/
};

struct usurfServerInterface {
	struct Window* window;
	void (*addText)(struct Window* window, uint8_t panel, 
			uint32_t x, uint32_t y, char* text);

	void (*addButton)(struct Window* window, uint32_t id,
			uint8_t panel, 
			uint32_t x, uint32_t y, 
			uint32_t w, uint32_t h, char* text);
};

struct usurfHeader {
#define USURF_READY 0x01 // ready to show!
#define USURF_ADDTEXT 0x10
#define USURF_ADDBUTTON 0x20
	uint32_t type;
	size_t size;
};

struct usurfAddText {
	struct usurfHeader header;
	uint8_t panel;
	uint32_t x; uint32_t y;
	/* text is after this thing */
};

struct usurfAddButton {
	struct usurfHeader header;
	uint8_t panel;
	uint32_t x; uint32_t y;
	uint32_t width; uint32_t height;
	uint32_t buttonid; // custom id is important. when a click happens,
			   // that id will be came.
	/* button text */
};

#endif /* USURF_USURFTYPES_H */
