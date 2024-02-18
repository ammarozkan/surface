#include "surface.h"


static inline uint8_t*
getColor(uint32_t x, uint32_t y)
{
	uint8_t* result = malloc(4);
	uint8_t xt = (x%255), yt = (y%255);
	uint8_t distance = (500 - x) * (500 - y);
	result[0] = distance; // B
	result[1] = distance; // G
	result[2] = distance; // R
	result[3] = 0x00; // X
	memset(result,0x33,4);
	return result;
}

void
render_dumbbuffer(struct fb_dumb *fb)
{
	for(uint32_t y = 0;y < fb->height;y++) {
		uint8_t *row = fb->data + fb->stride * y;
		for(uint32_t x = 0;x < fb->width;x++) {
			uint8_t* colour = getColor(x,y);
			row[x * 4 + 0] = colour[0];
			row[x * 4 + 1] = colour[1];
			row[x * 4 + 2] = colour[2];
			row[x * 4 + 3] = colour[3];
			free(colour);
		}
	}
	//memset(fb->data, 0x55, fb->size);
}

void
reverse_box(struct fb_dumb* fb, 
		uint32_t sx, uint32_t sy, 
		uint32_t ex, uint32_t ey)
{
	for(uint32_t y = sy; y < ey && y < fb->height;y++) {
		uint8_t *row = fb->data + fb->stride * y;
		for(uint32_t x = sx; x < ex && x < fb->width;x++) {
			row[x * 4 + 0] = 255 - row[x * 4 + 0];
			row[x * 4 + 1] = 255 - row[x * 4 + 1];
			row[x * 4 + 2] = 255 - row[x * 4 + 2];
			row[x * 4 + 3] = 255 - row[x * 4 + 3];
		}
	}
}

void
render_box(struct fb_dumb* fb, 
		uint32_t sx, uint32_t sy, 
		uint32_t ex, uint32_t ey,
		uint8_t* colour)
{
	for(uint32_t y = sy; y < ey && y < fb->height;y++) {
		uint8_t *row = fb->data + fb->stride * y;
		for(uint32_t x = sx; x < ex && x < fb->width;x++) {
			row[x * 4 + 0] = colour[0];
			row[x * 4 + 1] = colour[1];
			row[x * 4 + 2] = colour[2];
			row[x * 4 + 3] = colour[3];
		}
	}
}

void 
render_line(struct fb_dumb* fb,
		uint32_t sx, uint32_t ex, uint32_t y,
		uint8_t* colour)
{
	if (y > fb->height) return;
	uint8_t *row = fb->data + fb->stride * y;
	for(uint32_t x = sx;x < ex && x < fb->width; x++) {
		row[x * 4 + 0] = colour[0];
		row[x * 4 + 1] = colour[1];
		row[x * 4 + 2] = colour[2];
		row[x * 4 + 3] = colour[3];
	}
}

void
render_windowbase(struct fb_dumb* fb, struct Window* win)
{
	uint8_t colour[4] = {0xff,0xff,0xff,0xff};
	uint8_t bordercolour[4] = {0x22,0x22,0x22,0x00};
	uint8_t exitcolour[4] = {0x00,0x11,0xbb,0x00};
	uint8_t paniccolour[4] = {0x00,0xbb,0xbb,0x00};
	uint8_t fullscreencolour[4] = {0x00,0xbb,0x11,0x00};

	const int COL_TH = 2; // COLUMN_THICKNESS
	const int BUTTON_TH = 10;
	for(i = 0;i<WINDOW_UNTIL_BUTTON_THICKNESS;i+=1) {
		render_line(fb, win->sx, win->ex, win->sy - i, bordercolour);
	}

	for(j = 0;j<WINDOW_ROOF_BUTTON_THICKNESS;j+=1) {
		// Start Space
		uint32_t linestart = win->sx, 
			 lineend = win->sx + WINDOW_SPACE_FROM_START;
		render_line(fb, linestart, lineend, 
				win->sy - i - j,bordercolour);

		// Close Button		
		linestart = lineend; 
		lineend += WINDOW_GENERIC_ROOF_BUTTON_WIDTH;

		render_line(fb, linestart, lineend, 
				win->sy - i - j,exitcolour);

		// Space
		linestart = lineend;
		lineend += WINDOW_SPACE_BETWEEN_ROOF_BUTTONS;
		render_line(fb, linestart, lineend, 
				win->sy - i - j,bordercolour);

		// Fullscreen Button
		linestart = lineend;
		lineend += WINDOW_GENERIC_ROOF_BUTTON_WIDTH;
		render_line(fb, linestart, lineend, 
				win->sy - i - j,fullscreencolour);
		
		// Space Until Panic Button
		linestart = lineend;
		lineend = win->ex - WINDOW_SPACE_FROM_START 
				  - WINDOW_PANIC_BUTTON_WIDTH;
		render_line(fb, linestart, lineend, 
				win->sy - i - j,bordercolour);

		// Panic Button
		lineend = win->ex - WINDOW_SPACE_FROM_START;
		linestart = lineend - WINDOW_PANIC_BUTTON_WIDTH;
		render_line(fb, linestart, lineend, 
				win->sy - i - j,paniccolour);

		// Last Empty Space	
		render_line(fb, lineend, win->ex, 
				win->sy - i - j,bordercolour);
	}

	for(i = 0;i<WINDOW_UNTIL_BUTTON_THICKNESS;i+=1) {
		render_line(fb, win->sx, win->ex, win->sy - i - WINDOW_UNTIL_BUTTON_THICKNESS - WINDOW_ROOF_BUTTON_THICKNESS, bordercolour);
	}


	//render_box(fb, win->sx - 5, win->sy - COL_TH*2 - BUTTON_TH + 1, win->sx, win->ey,blackcolour);
	//render_box(fb, win->ex, win->sy - COL_TH*2 - BUTTON_TH + 1, win->ex + 4, win->ey,blackcolour);
	render_box(fb, win->sx, win->sy, win->ex, win->ey,win->bckr_colour);

}

void render_quickcursor(struct fb_dumb* fb, struct QuickCursor* qc)
{
	reverse_box(fb, qc->x - 2, qc->y - 2,
			qc->x + 2, qc->y + 2);
}

void
render_window(struct fb_dumb* fb, struct Window* win)
{
	render_windowbase(fb,win);
}

void
render_surface(struct fb_dumb* fb,struct Surface* surf)
{
	SURF_ITERATE(struct Window, surf->wins, win)
	{
		render_window(fb,win);
	}
	uint8_t contextcolour[4] = {0x22, 0x22, 0x22, 0x22};
	uint8_t whitecolour[4] = {0xff,0xff,0xff,0xff};
	if(surf->dcm.info & SURFACE_CONTEXT_ACTIVE) 
		render_box(fb,surf->dcm.posx,surf->dcm.posy,
				surf->dcm.posx + 20, surf->dcm.posy + 50,contextcolour);
	render_quickcursor(fb, &surf->cursor);
}
