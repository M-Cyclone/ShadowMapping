#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <random>
#include <chrono>
#include <cmath>
#include <memory>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>


#include "Timer.h"
#include "Shader.h"
#include "Object.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "DirectionalLight.h"
#include "DefaultConsts.h"


struct Scene
{
	std::unordered_map<std::string, Object> objects;

	void addObj(const std::string& name, Object obj)
	{
		if (objects.find(name) == objects.end())
		{
			objects[name] = std::move(obj);
		}
	}

	void render(const Shader& shader)
	{
		for (const auto& [name, obj] : objects)
		{
			obj.bind();
			shader.setValue("model", obj.getModel());
			glDrawElements(GL_TRIANGLES, obj.count, GL_UNSIGNED_INT, nullptr);
		}
	}

	void clear()
	{
		for (auto& [name, obj] : objects)
		{
			deleteObject(&obj);
		}
	}
};


int main(int argc, char** argv)
{
	assert(glfwInit());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(kScreenWidth, kScreenHeight, kWndName, nullptr, nullptr);
	assert(window);
	glfwMakeContextCurrent(window);

	assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
	glViewport(0, 0, kScreenWidth, kScreenHeight);


	glEnable(GL_DEPTH_TEST);


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig font;
	font.SizePixels = 20.0f;
	io.Fonts->AddFontDefault(&font);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


	Scene scene;
	scene.addObj("plane", createPlane());
	scene.objects["plane"].scale = { 10.0f, 10.0f, 10.0f };


	std::default_random_engine e;
	std::uniform_real_distribution u(0.0f, 1.0f);
	std::uniform_real_distribution s(0.3f, 0.5f);
	const uint32_t kBoxCount = 30;
	for (uint32_t i = 0; i < kBoxCount; ++i)
	{
		float x = u(e);
		float y = u(e);
		float z = u(e);
		std::string name = "box_" + std::to_string(i);
		scene.addObj(name, createBox());
		scene.objects[name].worldPos = { x * 8.0f - 4.0f, y * 3.0, z * 8.0f - 4.0f };
		scene.objects[name].scale = { s(e), s(e), s(e) };
	}


	auto camera = createCamera({ 9.0f, 6.0f, -9.0f }, { 0.0f, 0.0f, 0.0f });

	auto light = createDLight({ 7.5f, 7.5f, 7.5f }, { -2.0f, 0.0f, -2.0f }, 10.0f, 10.0f, 0.1f, 30.0f);


	auto shaderDepthMap = createShader("shader/depth_map.vert", "shader/depth_map.frag");
	auto shaderPCSS = createShader("shader/pcss.vert", "shader/pcss.frag");


	auto framebuffer = createFramebuffer(kDepthMapWidth, kDepthMapHeight);
	uint32_t depthMap = 0;
	{
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, kDepthMapWidth, kDepthMapHeight, 0, GL_RG, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	Timer timer;
	timer.last = std::chrono::steady_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


		const float dt = timer.mark();
		

		static bool shadowType = true; // true for pcss, false for vssm


		{
			static float cameraRadias = 15.0f;
			static float height = 6.0f;
			static float cameraTheta = 0.0f;
			cameraTheta += 15.0f * dt;

			const float theta = glm::radians(cameraTheta);
			camera.pos = { cameraRadias * std::cos(theta), height, cameraRadias * std::sin(theta) };
			camera.Update({ 0.0f, 0.0f, 0.0f });
		}


		auto lightSpaceTransMat = light.getProj() * light.getView();


		framebuffer.begin();
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shaderDepthMap.bind();
			shaderDepthMap.setValue("lightSpaceTrans", lightSpaceTransMat);

			scene.render(shaderDepthMap);
		}
		framebuffer.end();

		if(shadowType)
		{
			// pcss
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shaderPCSS.bind();

			shaderPCSS.setValue("cameraPos", camera.pos);

			shaderPCSS.setValue("light.pos", light.pos);
			shaderPCSS.setValue("light.lightWidth", light.lightWidth);

			shaderPCSS.setValue("proj", camera.getProj());
			shaderPCSS.setValue("view", camera.getView());
			shaderPCSS.setValue("lightSpaceTrans", lightSpaceTransMat);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			shaderPCSS.setValue("DepthMap", 0);

			scene.render(shaderPCSS);
		}
		else
		{

		}


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Component");
		{
			if (ImGui::TreeNode("Camera"))
			{
				ImGui::SliderFloat("Fov", &camera.fov, 0.0f, 90.0f);
				
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Light"))
			{
				ImGui::SliderFloat3("Light position", &light.pos.x, -5.0f, 5.0f);
				ImGui::SliderFloat("Light width", &light.lightWidth, 2.0f, 250.0f);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Shadow Algorithm"))
			{
				if (ImGui::Button("Change Algorithm"))
				{
					shadowType = !shadowType;
				}
				ImGui::Text(shadowType ? "Now: PCSS" : "Now: VSSM");

				ImGui::TreePop();
			}
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &depthMap);

	deleteShader(&shaderDepthMap);
	deleteShader(&shaderPCSS);
	scene.clear();


	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}