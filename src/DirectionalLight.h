#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct DirectionalLight
{
	glm::vec3 pos;
	glm::vec3 targetPos;

	float halfWidth = 0.0f;
	float halfHeight = 0.0f;
	float zNear = 0.1f;
	float zFar = 100.0f;

	float lightWidth = 50.0f;

	DirectionalLight(glm::vec3 pos, glm::vec3 targetPos, float halfWidth, float halfHeight, float zNear, float zFar)
		: pos(pos)
		, targetPos(targetPos)
		, halfWidth(halfWidth)
		, halfHeight(halfHeight)
		, zNear(zNear)
		, zFar(zFar)
	{}

	glm::mat4 getProj() const
	{
		return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar);
	}

	glm::mat4 getView() const
	{
		return glm::lookAt(pos, targetPos, { 0.0f, 1.0f, 0.0f });
	}
};