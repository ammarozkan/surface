varying mediump vec2 pos;

void main()
{
	if(distance(pos, vec2(9.0, 8.0)) < 3.0) 
		gl_FragColor = vec4(0.2, 0.2, 0.2, 0.6);
	else gl_FragColor = vec4(1.0, 1.0, 1.0, 0.6);
}
