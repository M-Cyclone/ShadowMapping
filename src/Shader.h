#pragma once
#include <string>
#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Shader
{
	uint32_t program = 0;

	template <class T>
	void setValue(const std::string& name, T value) const
	{
		assert(false);
	}

	void bind() const
	{
		glUseProgram(program);
	}
};


static Shader createShader(const std::string& vertFile, const std::string& fragFile)
{
	std::ifstream fVert(vertFile, std::ios::in);
	assert(fVert.is_open());

	std::stringstream vShaderStream;
	vShaderStream << fVert.rdbuf();
	fVert.close();

	std::string vertexCode = vShaderStream.str();
	const char* vertCode = vertexCode.c_str();


	std::ifstream fFrag(fragFile);
	assert(fFrag.is_open());

	std::stringstream fShaderStream;
	fShaderStream << fFrag.rdbuf();
	fFrag.close();

	std::string fragmentCode = fShaderStream.str();
	const char* fragCode = fragmentCode.c_str();


	int success;
	char infoLog[512];

	uint32_t shaderVert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shaderVert, 1, &vertCode, nullptr);
	glCompileShader(shaderVert);
	glGetShaderiv(shaderVert, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderVert, 512, nullptr, infoLog);
		std::cerr << "Vertex Shader Error: " << infoLog << std::endl;
		assert(false);
	}

	uint32_t shaderFrag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shaderFrag, 1, &fragCode, nullptr);
	glCompileShader(shaderFrag);
	glGetShaderiv(shaderFrag, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderFrag, 512, nullptr, infoLog);
		std::cerr << "Fragment Shader Error: " << infoLog << std::endl;
		assert(false);
	}


	uint32_t renderId = glCreateProgram();
	glAttachShader(renderId, shaderVert);
	glAttachShader(renderId, shaderFrag);
	glLinkProgram(renderId);
	glGetProgramiv(renderId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(renderId, 512, nullptr, infoLog);
		std::cerr << "Shader Linking Error: " << infoLog << std::endl;
		assert(false);
	}

	glDeleteShader(shaderVert);
	glDeleteShader(shaderFrag);


	Shader shader;
	shader.program = renderId;
	return shader;
}

static void deleteShader(Shader* shader)
{
	assert(shader);
	glDeleteProgram(shader->program);
}


template<> void Shader::setValue(const std::string& name, int val) const
{
	glUniform1i(glGetUniformLocation(program, name.c_str()), val);
}
template<> void Shader::setValue(const std::string& name, uint32_t val) const
{
	glUniform1i(glGetUniformLocation(program, name.c_str()), val);
}
template<> void Shader::setValue(const std::string& name, glm::ivec2 val) const
{
	glUniform2i(glGetUniformLocation(program, name.c_str()), val.x, val.y);
}
template<> void Shader::setValue(const std::string& name, glm::ivec3 val) const
{
	glUniform3i(glGetUniformLocation(program, name.c_str()), val.x, val.y, val.z);
}
template<> void Shader::setValue(const std::string& name, glm::ivec4 val) const
{
	glUniform4i(glGetUniformLocation(program, name.c_str()), val.x, val.y, val.z, val.w);
}

template<> void Shader::setValue(const std::string& name, float val) const
{
	glUniform1f(glGetUniformLocation(program, name.c_str()), val);
}
template<> void Shader::setValue(const std::string& name, glm::vec2 val) const
{
	glUniform2f(glGetUniformLocation(program, name.c_str()), val.x, val.y);
}
template<> void Shader::setValue(const std::string& name, glm::vec3 val) const
{
	glUniform3f(glGetUniformLocation(program, name.c_str()), val.x, val.y, val.z);
}
template<> void Shader::setValue(const std::string& name, glm::vec4 val) const
{
	glUniform4f(glGetUniformLocation(program, name.c_str()), val.x, val.y, val.z, val.w);
}

template<> void Shader::setValue(const std::string& name, glm::mat3 val) const
{
	glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}
template<> void Shader::setValue(const std::string& name, glm::mat4 val) const
{
	glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}