#version 430

in vec2 uv;
out vec4 FragColor;
uniform sampler2D u_texture;

void main()
{             
#if 0
    FragColor = vec4(texture(u_texture, uv).rgb, 1.0); 
#else
    FragColor = vec4(1, 0, 1, 1); 
#endif
}