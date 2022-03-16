#pragma once
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};


struct Object
{
	uint32_t vao = 0;
	uint32_t vbo = 0;
	uint32_t ibo = 0;

	glm::vec3 worldPos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

	uint32_t count = 0;

	Object() = default;
	Object(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: count(static_cast<uint32_t>(indices.size()))
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
	}

	~Object()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
		glDeleteVertexArrays(1, &vao);
	}

	void bind() const
	{
		glBindVertexArray(vao);
	}

	glm::mat4 getModel() const
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, worldPos);
		model = glm::scale(model, scale);
		return model;
	}
};

static std::unique_ptr<Object> createPlane()
{
	static std::vector<Vertex> vertices =
	{
		{ glm::vec3{ -1.0f, 0.0f, -1.0f },  glm::vec3{ 0.0f, 1.0f, 0.0f },  glm::vec2{  0.0f,  0.0f } },
		{ glm::vec3{ -1.0f, 0.0f, +1.0f },  glm::vec3{ 0.0f, 1.0f, 0.0f },  glm::vec2{  0.0f, +1.0f } },
		{ glm::vec3{ +1.0f, 0.0f, +1.0f },  glm::vec3{ 0.0f, 1.0f, 0.0f },  glm::vec2{ +1.0f, +1.0f } },
		{ glm::vec3{ +1.0f, 0.0f, -1.0f },  glm::vec3{ 0.0f, 1.0f, 0.0f },  glm::vec2{ +1.0f,  0.0f } },
	};

	static std::vector<uint32_t> indices =
	{
		0, 1, 2,
		0, 2, 3
	};

	return std::make_unique<Object>(vertices, indices);
}

static std::unique_ptr<Object> createBox()
{
	static std::vector<Vertex> vertices =
	{
		//position					 //normal					//texture coords				
		//back plane
		{ glm::vec3{ +1.0f, +1.0f, +1.0f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 1.0f, 1.0f } }, // 00: top right		
		{ glm::vec3{ +1.0f, -1.0f, +1.0f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 1.0f, 0.0f } }, // 01: bottom right
		{ glm::vec3{ -1.0f, -1.0f, +1.0f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 0.0f, 0.0f } }, // 02: bottom left
		{ glm::vec3{ -1.0f, +1.0f, +1.0f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 0.0f, 1.0f } }, // 03: top left
		//top plane
		{ glm::vec3{ +1.0f, +1.0f, -1.0f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 1.0f, 0.0f } }, // 04: bottom right
		{ glm::vec3{ +1.0f, +1.0f, +1.0f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 1.0f, 1.0f } }, // 05: top right
		{ glm::vec3{ -1.0f, +1.0f, -1.0f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 0.0f, 0.0f } }, // 06: bottom left
		{ glm::vec3{ -1.0f, +1.0f, +1.0f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 0.0f, 1.0f } }, // 07: top left
		//bottom plane
		{ glm::vec3{ +1.0f, -1.0f, -1.0f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 1.0f, 0.0f } }, // 08: bottom right
		{ glm::vec3{ +1.0f, -1.0f, +1.0f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 1.0f, 1.0f } }, // 09: top right
		{ glm::vec3{ -1.0f, -1.0f, -1.0f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 0.0f, 0.0f } }, // 10: bottom left
		{ glm::vec3{ -1.0f, -1.0f, +1.0f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 0.0f, 1.0f } }, // 11: top left
		//left plane
		{ glm::vec3{ -1.0f, -1.0f, -1.0f },  glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f, 0.0f } }, // 12: bottom right
		{ glm::vec3{ -1.0f, -1.0f, +1.0f },  glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f, 0.0f } }, // 13: bottom left
		{ glm::vec3{ -1.0f, +1.0f, -1.0f },  glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f, 1.0f } }, // 14: top right
		{ glm::vec3{ -1.0f, +1.0f, +1.0f },  glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f, 1.0f } }, // 15: top left
		//right plane
		{ glm::vec3{ +1.0f, -1.0f, -1.0f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f, 0.0f } }, // 16: bottom right
		{ glm::vec3{ +1.0f, -1.0f, +1.0f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f, 0.0f } }, // 17: bottom left
		{ glm::vec3{ +1.0f, +1.0f, -1.0f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f, 1.0f } }, // 18: top right
		{ glm::vec3{ +1.0f, +1.0f, +1.0f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f, 1.0f } }, // 19: top left
		//front plane
		{ glm::vec3{ +1.0f, +1.0f, -1.0f },  glm::vec3{ 0.0f,  0.0f, -1.0f },  glm::vec2{ 1.0f, 1.0f } }, // 20: top right
		{ glm::vec3{ +1.0f, -1.0f, -1.0f },  glm::vec3{ 0.0f,  0.0f, -1.0f },  glm::vec2{ 1.0f, 0.0f } }, // 21: bottom right
		{ glm::vec3{ -1.0f, -1.0f, -1.0f },  glm::vec3{ 0.0f,  0.0f, -1.0f },  glm::vec2{ 0.0f, 0.0f } }, // 22: bottom left
		{ glm::vec3{ -1.0f, +1.0f, -1.0f },  glm::vec3{ 0.0f,  0.0f, -1.0f },  glm::vec2{ 0.0f, 1.0f } }  // 23: top left
	};

	static std::vector<uint32_t> indices =
	{
		//back plane
		0,  1,  2, // first triangle
		2,  3,  0, // second triangle 
		//top plane
		4,  5,  7,
		4,  6,  7,
		//bottom plane
		8,  9,  11,
		8,  10, 11,
		//left plane
		12, 13, 15,
		12, 14, 15,
		//right plane
		16, 17, 19,
		16, 18, 19,
		//front plane
		20, 21, 22,
		22, 23, 20
	};

	return std::make_unique<Object>(vertices, indices);
}