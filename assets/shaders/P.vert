#version 330

layout(location = 0) in vec3 vert_position;

out vec3 position;

void main()
{
	gl_Position = vec4(vert_position, 1.0);

	position = vert_position;
}
