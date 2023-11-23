#version 430

in vec2 uv;
out vec4 FragColor;
uniform sampler2D u_texture;

void main()
{
  FragColor = vec4(texture(u_texture, uv).rgb, 1.0); 
}