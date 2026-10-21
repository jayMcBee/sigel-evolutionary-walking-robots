#version 120

// Each of the 16 Poisson disk taps is a hardware 2x2 comparison, as the
// shadow map filters linearly. The terrain texture is applied as
// GL_DECAL does it, and the fog as GL_LINEAR does it.

uniform sampler2D terrainTexture;
uniform sampler2DShadow shadowMap;
uniform bool terrainTextured;
uniform float shadowFilterRadius;
varying vec4 shadowCoordinate;
const int poissonTapCount = 16;
const vec2 poissonDisk[ poissonTapCount ] = vec2[ poissonTapCount ](
  vec2( -0.94201624, -0.39906216 ), vec2(  0.94558609, -0.76890725 ),
  vec2( -0.09418410, -0.92938870 ), vec2(  0.34495938,  0.29387760 ),
  vec2( -0.91588581,  0.45771432 ), vec2( -0.81544232, -0.87912464 ),
  vec2( -0.38277543,  0.27676845 ), vec2(  0.97484398,  0.75648379 ),
  vec2(  0.44323325, -0.97511554 ), vec2(  0.53742981, -0.47373420 ),
  vec2( -0.26496911, -0.41893023 ), vec2(  0.79197514,  0.19090188 ),
  vec2( -0.24188840,  0.99706507 ), vec2( -0.81409955,  0.91437590 ),
  vec2(  0.19984126,  0.78641367 ), vec2(  0.14383161, -0.14100790 ) );
void main()
{
  vec3 coordinate = shadowCoordinate.xyz / shadowCoordinate.w;
  float sunlight = 0.0;
  for (int i = 0; i < poissonTapCount; i++)
    sunlight += shadow2D( shadowMap, vec3( coordinate.xy + poissonDisk[ i ] * shadowFilterRadius,
                                           coordinate.z ) ).r;
  sunlight /= float( poissonTapCount );
  vec4 color = vec4( min( gl_Color.rgb + gl_SecondaryColor.rgb * sunlight, 1.0 ), gl_Color.a );
  if (terrainTextured)
  {
    vec4 texel = texture2D( terrainTexture, gl_TexCoord[ 0 ].st );
    color.rgb = mix( color.rgb, texel.rgb, texel.a );
  }
  float clearness = clamp( (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0 );
  color.rgb = mix( gl_Fog.color.rgb, color.rgb, clearness );
  gl_FragColor = color;
}
