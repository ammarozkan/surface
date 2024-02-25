
/*
uniform float roof_thickness;
uniform float roof_button_thickness;
uniform float roof_button_width;
uniform float space_from_start;
uniform float space_between_buttons;
uniform float panic_button_width;
*/

varying mediump vec2 win_pos;
varying mediump vec2 w_size;

mediump float roof_thickness = 16.0;

mediump vec4 getButtons(mediump vec2 pos)
{
	if(5.0 < pos.x && pos.x < 25.0) 
		return vec4(0.8, 0.3, 0.3, 1.0);
	else if(30.0 < pos.x && pos.x < 50.0)
		return vec4(0.4, 0.7, 0.3, 1.0);
	else if(w_size.x - 13.0 < pos.x && pos.x < w_size.x - 5.0)
		return vec4(0.8, 0.7, 0.3, 1.0);
	return vec4(0.3, 0.3, 0.3, 1.0);
}

mediump vec4 getRoof(mediump vec2 pos)
{
	if(pos.y > 4.0 && pos.y < 12.0)
	{
		return getButtons(pos);
	}	
	return vec4(0.3, 0.3, 0.3, 1.0);
}

bool
roundLegal(mediump float radius)
{
	if(win_pos.y < radius) {
		if(win_pos.x < radius)
			if(distance(win_pos, vec2(radius, radius)) > radius) return false;
		if(win_pos.x > w_size.x - radius)
			if(distance(win_pos, vec2(w_size.x - radius, radius)) > radius) return false;
	} 

	// Uncomment this part if wanna round the down.
	/*
	else if(win_pos.y > w_size.y - radius) {	
		if(win_pos.x < radius)
			if(distance(win_pos, vec2(radius, w_size.y - radius)) > radius) return false;
		if(win_pos.x > w_size.x - radius)
			if(distance(win_pos, vec2(w_size.x - radius, w_size.y - radius)) > radius) return false;
	} 
	*/
	return true;
}

void main()
{
	if(!roundLegal(5.0)) discard;
	
	if(win_pos.y < roof_thickness) {
		gl_FragColor = getRoof(win_pos);
	} else {
		gl_FragColor = vec4(0.8, 0.8, 0.8, 1.0);
	}
}
