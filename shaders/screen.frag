#version 430
out vec4 FragColor;

uniform sampler2D u_texture;

void main()
{             
    //FragColor = vec4(texture(u_texture, TexCoords).rgb, 1.0); 
    FragColor = vec4(vec3(1,0,1), 1); 
}