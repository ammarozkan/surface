attribute vec2 model_pos;

uniform vec2 emptyone;
uniform vec2 screen_size;

void main()
{
	vec2 result = model_pos;
	result.y = result.y * 15.0/screen_size.y;
	result = 2.0*result - vec2(1.0,1.0);
	result.y *= -1.0;
	gl_Position = vec4(result, 0.0, 1.0);
}
