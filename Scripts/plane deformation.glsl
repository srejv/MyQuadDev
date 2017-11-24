void main()
{
	vec2 p = ((gl_FragCoord.xy / iResolution.xy) * 2.0) - 1.0;

    float d = sqrt( p.x*p.x + p.y*p.y );
    float a = atan( p.y, p.x );

    float u = cos(a+iTime)/d;
    float v = sin(a)/d;
    

	FragColor = texture(iChannel0, vec2(u, v)); // vec4(p.x, p.y, 0.0f, 1.0f);
};#define PI 3.14159265359

void main()
{
	vec2 p = ((gl_FragCoord.xy / iResolution.xy) * 2.0) - 1.0;

    float d = sqrt( p.x*p.x + p.y*p.y );
    float a = atan( p.y, p.x );

    // Idk what r should be
    float r = d * 0.5;

    //float u = cos(a+iTime)/d;
    //float v = sin(a)/d;
    
    //float u = p.x*cos(2.0*r) - p.y*sin(2.0*r);
    //float v = p.y*cos(2.0*r) + p.x*sin(2.0*r);;

    //float u = 0.5*a/PI;
    //float v = sin(7.0*r);

    //float u = 0.3/(r+0.5*p.x);
    //float v = 3.0*a/PI;

    //float u = r*cos(a+r);;
    //float v = r*sin(a+r);;

    //float u = 0.02*p.y + 0.03*cos(a*3.0)/r;;
    //float v = 0.02*p.x + 0.03*sin(a*3.0)/r;

    //float u = 1.0/(r + 0.5 + 0.5 * sin(5.0*a));;
    //float v = a*3.0/PI;
    
    //float u = 0.1*p.x/(0.11+r*0.5);
    //float v = 0.1*p.y/(0.11+r*0.5);

    float u = p.x/abs(p.y);
    float v = 1.0/abs(p.y);

    if(p.y < 0.0) {
        FragColor = texture(iChannel0, vec2(u, v));
    } else {
        FragColor = vec4(0.2,0.4 + sin(iTime) * 0.05,0.8,1.0);
    }
};