attribute vec2 model_pos;

//varying mediump vec4 varyingColor;

void main()
{
	/*
	if(model_pos == vec2(1.0,0.0)) varyingColor = vec4(0.7, 0.1, 0.3, 1.0);
	else if(model_pos == vec2(0.0, 1.0)) varyingColor = vec4(0.1, 0.6, 0.4, 1.0);
	else varyingColor = vec4(0.1, 0.3, 0.8, 1.0);
	*/

	vec2 realpos = model_pos*2.0 - vec2(1.0, 1.0);
	gl_Position = vec4(realpos, 0.0, 1.0);
}
