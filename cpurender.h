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
	for(uint32_t y = sy; y < ey;y++) {
		uint8_t *row = fb->data + fb->stride * y;
		for(uint32_t x = sx; x < ex;x++) {
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
	for(uint32_t y = sy; y < ey;y++) {
		uint8_t *row = fb->data + fb->stride * y;
		for(uint32_t x = sx; x < ex;x++) {
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
	uint8_t *row = fb->data + fb->stride * y;
	for(uint32_t x = sx;x < ex; x++) {
		row[x * 4 + 0] = colour[0];
		row[x * 4 + 1] = colour[1];
		row[x * 4 + 2] = colour[2];
		row[x * 4 + 3] = colour[3];
	}
}

void
render_windowbase(struct fb_dumb* fb, struct Window* win)
{
	uint8_t* colour = malloc(4); /// C is reallocating colours again and again I guess.
	colour[0] = 0xff; // B
	colour[1] = 0xff; // G
	colour[2] = 0xff; // R
	colour[3] = 0xff; // X
	uint8_t* blackcolour = malloc(4);
	blackcolour[0] = 0x22;
	blackcolour[1] = 0x22;
	blackcolour[2] = 0x22;
	blackcolour[3] = 0x00;
	uint8_t* redcolour = malloc(4);
	redcolour[0] = 0x00;
	redcolour[1] = 0x11;
	redcolour[2] = 0xbb;
	redcolour[3] = 0x00;
	uint8_t* yellowcolour = malloc(4);
	yellowcolour[0] = 0x00;
	yellowcolour[1] = 0xbb;
	yellowcolour[2] = 0xbb;
	yellowcolour[3] = 0x00;
	uint8_t* greencolour = malloc(4);
	greencolour[0] = 0x00;
	greencolour[1] = 0xbb;
	greencolour[2] = 0x11;
	greencolour[3] = 0x00;

	const int COL_TH = 2; // COLUMN_THICKNESS
	const int BUTTON_TH = 5;
	for(i = 0;i<COL_TH;i+=1) {
		render_line(fb, win->sx, win->ex, win->sy-i,blackcolour);
		render_line(fb, win->sx, win->ex, win->sy-i-COL_TH-BUTTON_TH, blackcolour);
	}

	for(j = 0;j<BUTTON_TH;j+=1) {	
		render_line(fb, win->sx, win->sx + 5, win->sy-i-j, blackcolour);
		render_line(fb, win->sx + COL_TH, win->sx + 25, win->sy-i-j, redcolour); // exit
		render_line(fb, win->sx + 25, win->sx + 30, win->sy-i-j, blackcolour); // black
		render_line(fb, win->sx + 30, win->sx + 50, win->sy-i-j, greencolour); // fullscreen
		render_line(fb, win->sx + 50, win->ex - 10, win->sy-i-j, blackcolour); // black
		render_line(fb, win->ex - 10, win->ex - 10 + BUTTON_TH, win->sy-i-j, yellowcolour); // panic button
		render_line(fb, win->ex - 10 + BUTTON_TH, win->ex, win->sy-i-j, blackcolour); // black
	}


	render_box(fb, win->sx - 5, win->sy - COL_TH*2 - BUTTON_TH + 1, win->sx, win->ey,blackcolour);
	//render_box(fb, win->ex, win->sy - COL_TH*2 - BUTTON_TH + 1, win->ex + 4, win->ey,blackcolour);
	render_box(fb, win->sx, win->sy, win->ex, win->ey,colour);

	free(colour);
	free(blackcolour);
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
}
