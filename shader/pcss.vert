#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

out vec3 vCameraPos;

out vec4 LightSpacePos;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform mat4 lightSpaceTrans;

uniform vec3 cameraPos;

void main()
{
	gl_Position = proj * view * model * vec4(aPos, 1.0);

	vPosition = vec3(model * vec4(aPos, 1.0));
	vNormal = aNormal;
	vTexCoord = aTexCoord;

	LightSpacePos = lightSpaceTrans * vec4(vPosition, 1.0);
}