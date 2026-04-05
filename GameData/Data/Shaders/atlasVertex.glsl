#version 430 core
layout(location = 0) in vec2 aPos; 
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord; 
uniform mat4 u_MVP; 

// either -1 or 1;
uniform vec2 u_flip;

// xy = top left coords of sprite in atlas
// zw = scale of the sprite
// mapped from 0 to 1
uniform vec4 u_AtlasUV;

void main() 
{ 
    gl_Position = u_MVP * vec4(aPos.x, -aPos.y, 0.0, 1.0); 

    vec2 flippedUV = mix(aTexCoord, 1.0 - aTexCoord, (u_flip * 0.5) + 0.5);
    TexCoord = u_AtlasUV.xy + (flippedUV * u_AtlasUV.zw);
}