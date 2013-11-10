#version 330

layout(location = 0) in vec4 vert_position;

out vec2 uv;

void main()
{
	gl_Position = vec4(vert_position.xy, 0.0, 1.0);

	uv = vert_position.zw;
}