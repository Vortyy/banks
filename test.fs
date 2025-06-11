#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

const float warp = 0.75; // simulate curvature of CRT monitor
const float scan = 0.75; // simulate darkness between scanlines

uniform float time;

void main() // A CRT shader inspired by (https://www.shadertoy.com/view/WsVSzV) Adapted to RAYLIB 
{
  float freq = 450.0/2.0;
  // squared distance from center
  vec2 uv = fragTexCoord;
  vec2 dc = abs(0.5-uv);
  dc *= dc;
  
  // warp the fragment coordinates
  uv.x -= 0.5; uv.x *= 1.0+(dc.y*(0.3*warp)); uv.x += 0.5;
  uv.y -= 0.5; uv.y *= 1.0+(dc.x*(0.4*warp)); uv.y += 0.5;

  // sample inside boundaries, otherwise set to black
  if (uv.y > 1.0 || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0)
      finalColor = vec4(0.0,0.0,0.0,1.0);
  else
  {
    float globalPos = (fragTexCoord.y) * freq;
    float wavePos = cos((fract(globalPos))*3.14);
    
    vec4 texelColor = texture(texture0, uv);
    // sample the texture
    finalColor = mix(texelColor, vec4(0.1, 0.1, 0.1, 1.0), wavePos);
  }
}
