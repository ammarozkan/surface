attribute vec2 position;
//vec2 screen = vec2(1280,800);

uniform vec2 w_s;
uniform vec2 w_e;
uniform vec2 screen_size;

vec2 w_pos = vec2(100, 100);
varying mediump vec2 w_size;
vec2 result;

varying mediump vec2 win_pos;

mediump float roof_thickness = 20.0;

void main()
{
	w_pos = w_s - vec2(0.0, roof_thickness);
	w_size = w_e - w_s + vec2(0.0, roof_thickness);
	w_size+=vec2(0.0,roof_thickness);
	vec2 normd = vec2(position.x*w_size.x + w_pos.x,
		position.y*w_size.y + w_pos.y);
	
	win_pos = normd - w_pos;

	normd = vec2(normd.x/screen_size.x, 
			normd.y/screen_size.y);
	result = vec2(2.0*normd.x - 1.0,
		1.0 - 2.0*normd.y);
	gl_Position = vec4(result, 0.0, 1.0);
}
