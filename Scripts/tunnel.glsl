void main() {
  // normalized coordinates (-1 to 1 vertically)
  vec2 p = (-iResolution.xy + 2.0*gl_FragCoord)/iResolution.y;

  // angle of each pixel to the center of the screen
  float a = atan(p.y,p.x);

  #if 0
    // cylindrical tunnel
    float r = length(p);
  #else
    // squareish tunnel
    float r = pow( pow(p.x*p.x,4.0) + pow(p.y*p.y,4.0), 1.0/8.0 );
  #endif

  // index texture by (animated inverse) radious and angle
  vec2 uv = vec2( 0.3/r + 0.2*iTime, a/3.1415927 );

  #ifdef NAIVE_IMPLEMENTATION
    // naive fecth color
    vec3 col =  texture( iChannel0, uv ).xyz;
  #else
    // fetch color with correct texture gradients, to prevent discontinutity
    vec2 uv2 = vec2( uv.x, atan(p.y,abs(p.x))/3.1415927 );
    vec3 col = textureGrad( iChannel0, uv, dFdx(uv2), dFdy(uv2) ).xyz;
  #endif

  // darken at the center
  col = col*r;

  gl_FragColor = vec4( col, 1.0 );
}
