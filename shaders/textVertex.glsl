attribute vec2 model_pos;
attribute vec2 texture_coord_attrb;

uniform vec2 screen_size;

varying mediump vec2 texture_coord;

void main()
{
	texture_coord = texture_coord_attrb;
	vec2 result = vec2(model_pos.x / screen_size.x, model_pos.y / screen_size.y);
	gl_Position = vec4(2.0*result - vec2(1.0, 1.0), 
		0.0, 1.0);

} 
