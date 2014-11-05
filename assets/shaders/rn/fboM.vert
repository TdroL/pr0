#version 330

layout(location = 0) in vec4 vertPosition;

uniform mat4 M;
out vec2 uv;

void main()
{
	gl_Position = M * vec4(vertPosition.xy, 0.0, 1.0);

	uv = vertPosition.zw;
}