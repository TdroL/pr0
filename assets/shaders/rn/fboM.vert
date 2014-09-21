#version 330

layout(location = 0) in vec4 vert_position;

uniform mat4 M;
out vec2 uv;

void main()
{
	gl_Position = M * vec4(vert_position.xy, 0.0, 1.0);

	uv = vert_position.zw;
}