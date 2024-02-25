attribute vec2 model_pos;

vec2 emptyvariable = vec2(1.0,1.0);
uniform vec2 screen_size;
uniform vec2 cursorPos;

mediump float cursor_size = 10.0;

void main()
{
	vec2 n_model = model_pos;
	if(n_model.x == 1.0 && n_model.y == 1.0) n_model = vec2(0.8,0.8);

	vec2 result = cursorPos + n_model*10.0;
	result = vec2(result.x/screen_size.x, 
			result.y/screen_size.y);
	result = vec2(2.0*result.x - 1.0, 2.0*result.y - 1.0); // opengl screen
	result.y = -result.y;
	gl_Position = vec4(result, 0.0, 1.0);
}
