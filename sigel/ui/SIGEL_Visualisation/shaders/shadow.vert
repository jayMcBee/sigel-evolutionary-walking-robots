#version 120

// Lighting per vertex, as GL_LIGHT0 and GL_LIGHT1 give it. The sun goes
// out in the secondary colour, so the fragment shader shadows only it.

uniform mat4 shadowMatrix;
uniform vec3 sunDirection;
uniform float sunIntensity;
varying vec4 shadowCoordinate;
vec3 cameraLight( int index, vec3 normal, vec3 eyePosition )
{
  vec3 towardsLight = normalize( gl_LightSource[ index ].position.xyz - eyePosition );
  return max( dot( normal, towardsLight ), 0.0 ) * gl_LightSource[ index ].diffuse.rgb;
}
void main()
{
  vec4 eyePosition = gl_ModelViewMatrix * gl_Vertex;
  vec3 normal = normalize( gl_NormalMatrix * gl_Normal );
  vec3 light = gl_LightModel.ambient.rgb
             + cameraLight( 0, normal, eyePosition.xyz )
             + cameraLight( 1, normal, eyePosition.xyz );
  gl_FrontColor = vec4( light * gl_Color.rgb, gl_Color.a );
  vec3 sun = max( dot( normal, sunDirection ), 0.0 ) * sunIntensity * gl_Color.rgb;
  gl_FrontSecondaryColor = vec4( sun, 1.0 );
  gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
  shadowCoordinate = shadowMatrix * eyePosition;
  gl_FogFragCoord = abs( eyePosition.z );
  gl_Position = ftransform();
}
