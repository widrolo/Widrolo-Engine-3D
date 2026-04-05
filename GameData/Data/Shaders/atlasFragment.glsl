#version 430 core

uniform sampler2D u_Texture; 
uniform vec4 u_Color; 
uniform bool u_Filtering;

in vec2 TexCoord; 
out vec4 FragColor; 

void main() 
{ 
    vec4 texColor;
    if (u_Filtering)
    {
        texColor = texture(u_Texture, TexCoord); 
    }
    else
    {
        ivec2 texelCoord = ivec2(TexCoord * textureSize(u_Texture, 0));
        texColor = texelFetch(u_Texture, texelCoord, 0);
    }
    FragColor = texColor * u_Color; 
}