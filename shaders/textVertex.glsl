attribute vec2 model_pos;
attribute vec2 texture_coord_attrb;

uniform vec2 screen_size;

varying mediump vec2 texture_coord;

void main()
{
	texture_coord = texture_coord_attrb;
	gl_Position = vec4(2.0*(model_pos / screen_size) - vec2(1.0, 1.0), 
		0.0, 1.0);

} 
