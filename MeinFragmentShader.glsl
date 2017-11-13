#version 330

layout(location = 0) out vec4 out_color;

uniform vec3 object_color;

void main()
{
	// TODO: write pixel out color
	out_color = vec4(object_color,1);
}