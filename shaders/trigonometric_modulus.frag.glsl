#version 400
uniform float time;

in vec3 interpolatedColor;
in vec2 fraguv;
out vec4 fragColor;

#define PI 3.141592653


void main(void) {
    vec2 uv = fraguv;
    float repetitions = 3.0;
    // Do linear transformation along both axes.
    vec2 xy = ((uv-0.5)*2*PI*repetitions)-PI/2;

    float x =xy.x;
    float y= xy.y;


    // This formula is loosely based on GrafEq example "eyes".
    // The original formula was mod(sin(x), cos(y)) == mod(sin(y), cos(x)).
    float c1 =mod(abs(1-sin(x)), abs(cos(y)));
    float c2 =mod(abs(1-sin(y)), abs(cos(x)));
    float mainTone = c1-c2;

    // A smooth contour line that sweeps over values with equal mainTone.
    float sweepTone = 1/(mod(mainTone+time/4.0, 2.0)/ 0.04);

    fragColor = vec4(mainTone, sweepTone,-mainTone, 1.0);

}


