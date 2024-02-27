attribute vec2 model_pos;

uniform vec2 emptyone;
uniform vec2 screen_size;

varying mediump vec2 pos;

void main()
{
	vec2 result = model_pos;
	result.y = result.y * 15.0/screen_size.y;
	pos = result*screen_size;
	result = 2.0*result - vec2(1.0,1.0);
	result.y *= -1.0;
	gl_Position = vec4(result, 0.0, 1.0);
}
