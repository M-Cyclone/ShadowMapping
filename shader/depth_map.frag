#version 330 core

out vec4 FragColor;

void main()
{
	float depth = gl_FragCoord.z;
	float depth_2 = depth * depth;
	FragColor = vec4(depth, depth_2, 1.0, 1.0);
}