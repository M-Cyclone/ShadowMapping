#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct ComputeShader
{
	uint32_t program = 0;

	void bind() const
	{
		glUseProgram(program);
	}
};

ComputeShader createComputeShader(const std::string& shaderPath)
{
	std::ifstream file(shaderPath, std::ios::in);
	assert(file.is_open());

	std::stringstream shaderStream;
	shaderStream << file.rdbuf();
	file.close();

	std::string codeStr = shaderStream.str();
	const char* code = codeStr.c_str();


	int success;
	char infoLog[512];


	uint32_t shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &code, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "Compute Shader Error: " << infoLog << std::endl;
		assert(false);
	}

	uint32_t newProgram = glCreateProgram();
	glAttachShader(newProgram, shader);
	glLinkProgram(newProgram);
	glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(newProgram, 512, nullptr, infoLog);
		std::cerr << "Shader Linking Error: " << infoLog << std::endl;
		assert(false);
	}

	ComputeShader cs;
	cs.program = newProgram;

	return cs;
}

static void deleteShader(ComputeShader* shader)
{
	assert(shader);
	glDeleteProgram(shader->program);
}