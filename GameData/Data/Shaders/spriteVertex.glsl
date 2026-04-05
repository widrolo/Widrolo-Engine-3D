#version 430 core
layout(location = 0) in vec2 aPos; 
layout(location = 1) in vec2 aTexCoord; 

out vec2 TexCoord; 
uniform mat4 u_MVP; 

// either -1 or 1;
uniform vec2 u_flip;

void main() 
{ 
    gl_Position = u_MVP * vec4(aPos.x, -aPos.y, 0.0, 1.0); 

    vec2 flip01 = (u_flip * 0.5) + 0.5;
    TexCoord = mix(aTexCoord, 1.0 - aTexCoord, flip01);
}