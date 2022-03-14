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

static Framebuffer createFramebuffer(uint32_t width, uint32_t height)
{
	uint32_t fbo = 0;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	uint32_t depthBuffer = 0;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Framebuffer framebuffer;
	framebuffer.fbo = fbo;
	framebuffer.width = width;
	framebuffer.height = height;
	framebuffer.depthBuffer = depthBuffer;
	return framebuffer;
}

static void deleteFramebuffer(Framebuffer* framebuffer)
{
	assert(framebuffer);

	glDeleteRenderbuffers(1, &framebuffer->depthBuffer);

	glDeleteFramebuffers(1, &framebuffer->fbo);
}