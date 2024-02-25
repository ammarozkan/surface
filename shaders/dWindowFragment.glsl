
/*
uniform float roof_thickness;
uniform float roof_button_thickness;
uniform float roof_button_width;
uniform float space_from_start;
uniform float space_between_buttons;
uniform float panic_button_width;
*/

varying mediump vec2 win_pos;

mediump vec4 getRoof(mediump vec2 pos)
{
	return vec4(0.3, 0.3, 0.3, 1.0);
}

mediump float roof_thickness = 16.0;

void main()
{
	if(win_pos.y < roof_thickness) {
		gl_FragColor = getRoof(win_pos);
	} else {
		gl_FragColor = vec4(0.8, 0.8, 0.8, 1.0);
	}
}
