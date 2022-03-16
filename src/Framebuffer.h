#pragma once
#include <cassert>
#include <cstdint>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DefaultConsts.h"

struct Framebuffer
{
	uint32_t width = 0;
	uint32_t height = 0;

	uint32_t fbo = 0;

	uint32_t depthBuffer = 0;

	Framebuffer(uint32_t width, uint32_t height)
		: width(width)
		, height(height)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	~Framebuffer()
	{
		glDeleteRenderbuffers(1, &depthBuffer);
		glDeleteFramebuffers(1, &fbo);
	}

	void begin() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);
	}

	void end()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, kScreenWidth, kScreenHeight);
	}
};