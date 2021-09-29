#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 projView;

void main(void){
	gl_Position = projView * vec4(aPos, 1.0);
}
