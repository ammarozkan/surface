varying mediump vec2 texture_coord;

uniform sampler2D text;
uniform mediump vec4 textColor;

void main()
{
	highp vec4 sampled = vec4(1.0, 1.0, 1.0, texture2D(text, texture_coord).r);
	gl_FragColor = textColor * sampled;
}
