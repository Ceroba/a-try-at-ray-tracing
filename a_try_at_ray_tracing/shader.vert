#version 450

layout(location = 0)in vec3 i_pos;
layout(location = 0)out vec2 uv;

void main(){
	gl_Position = vec4(i_pos, 1.0);
	uv = vec2(i_pos.x, -i_pos.y);
}