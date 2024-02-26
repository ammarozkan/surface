
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

mediump float roof_thickness = 20.0;

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

mediump vec4 getCircularButtons(mediump vec2 pos, mediump float height, mediump float radius, lowp float start, lowp float space)
{
	if(distance(pos, vec2(start+radius, height)) < radius)
		return vec4(0.8, 0.3, 0.3, 1.0);
	else if(distance(pos, vec2(start+radius, height)) < radius + 0.1)
		return (vec4(0.8, 0.3, 0.3, 1.0) + vec4(0.3, 0.3, 0.3, 1.0))/2.0;
	else if(distance(pos, vec2(start+space+3.0*radius, height)) < radius)
		return vec4(0.4, 0.7, 0.3, 1.0);
	else if(distance(pos, vec2(start+space+3.0*radius, height)) < radius + 0.1)
		return (vec4(0.4, 0.7, 0.3, 1.0) + vec4(0.3, 0.3, 0.3, 1.0))/2.0;
	return vec4(0.3, 0.3, 0.3, 0.7);
}

mediump vec4 getRoof(mediump vec2 pos)
{
	return getCircularButtons(pos, 10.0, 6.0, 10.0, 5.0);
}

bool
roundLegal(mediump float radius, mediump float porius) // radius, position_radius
{
	if(win_pos.y < porius) {
		if(win_pos.x < porius)
			if(distance(win_pos, vec2(porius, porius)) > radius) return false;
		if(win_pos.x > w_size.x - porius)
			if(distance(win_pos, vec2(w_size.x - porius, porius)) > radius) return false;
	} 

	// Uncomment this part if wanna round the down.
	
	else if(win_pos.y > w_size.y - porius) {	
		if(win_pos.x < porius)
			if(distance(win_pos, vec2(porius, w_size.y - porius)) > radius) return false;
		if(win_pos.x > w_size.x - porius)
			if(distance(win_pos, vec2(w_size.x - porius, w_size.y - porius)) > radius) return false;
	} 
	
	return true;
}

highp vec4 
getColor()
{
	if(win_pos.y < roof_thickness) {
		return getRoof(win_pos);
	} else {
		return vec4(0.3, 0.3, 0.3, 0.7);
	}
}

void main()
{
	if(!roundLegal(4.0, 5.0)) {
		if(!roundLegal(4.7, 5.0)) discard;
		else gl_FragColor = getColor() - vec4(0.0, 0.0, 0.0, 0.2);
	} else gl_FragColor = getColor();
	
}
