#version 430
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex;

void main()
{
	gl_Position = vec4(vec3(a_pos, 0), 1);
}
