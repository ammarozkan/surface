attribute vec2 model_pos;

uniform vec2 cursorPos;
uniform vec2 screen_size;

mediump float cursor_size = 10.0;

void main()
{
	vec2 result = cursorPos + model_pos*10.0;
	result = vec2(result.x/screen_size.x, 
			result.y/screen_size.y);
	gl_Position = vec4(result, 0.0, 1.0);
}
