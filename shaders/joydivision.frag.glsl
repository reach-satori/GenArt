#version 400
#extension GL_ARB_explicit_uniform_location : enable

const float PI = 3.1415926535897932384626433832795;
const float N = 10.;


layout(location = 2) uniform float time;
in vec3 interpolatedColor;
in vec2 fraguv;
out vec4 fragColor;


float rand(float n){return fract(sin(n) * 35758.453123);}


float noise(float p){
	float fl = floor(p);
        float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), smoothstep(0., 1., fc));
}

float makeline(float center, float epsilon, vec2 pos) {
	return smoothstep(center - epsilon, center, pos.y) - smoothstep(center, center + epsilon, pos.y);
}


void main( void ) {
	vec2 uv = fraguv;
	uv.y = 1.-uv.y;
	float f = 0.;
	for (float i = 1.; i <= N; i++){

		float lengthwise = uv.x * 30 + time;
		float x = (0.5 - uv.x) * 2.;
		float amplitude = (0.16 - 0.005 * i) * pow(min(cos(PI * x / 2.), 1. - abs(x)), 4.);
		float yshift = 0.7 - 0.06 * i;
		float linewidth = 0.01 - 0.0001 * i;
		f += makeline( noise(lengthwise * i)*amplitude + yshift, linewidth, uv);
	}

	vec3 color = vec3(0., .8, .3) * f;
	fragColor = vec4(color, 1.);
}
