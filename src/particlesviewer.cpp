#include "viewer.h"
#include "drawbuffer.h"
#include "renderapi.h"

#include <time.h>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include "globaldata.cpp"
#include <vector>
#include <iostream>

struct MyParticlesViewer : Viewer {

	//ADDED PARTICLE & WELL STRUCTS
	struct Well {

		glm::vec2 position;
		const float padding = 10.0f;

		Well(glm::vec2 initPosition) {
			position = initPosition;
		}
	};

	struct Particle {

		glm::vec2 position;
		float velocity;
		glm::vec2 direction;
		const float padding = 5.0f;

		Particle(glm::vec2 initPosition) {
			position = initPosition;
			velocity = 1.0f;
			direction = {0, 0};
		}

		void updatePosition(std::vector<Well*> wellList) {

			for(Well* well : wellList) {
				glm::vec2 vecDir = well->position - position;
				//If force is too big divide by something
				float force = 1/(std::sqrt(vecDir.x * vecDir.x + vecDir.y * vecDir.y))/8;
				direction = direction + vecDir * force;
			}
			position = position + (direction * velocity);
		}
	};

	const int numParticles = 10;
	const int numWells = 4;
	//

	glm::vec3 jointPosition;
	glm::vec3 cubePosition;
	float boneAngle;

	glm::vec2 mousePos;

	bool leftMouseButtonPressed;
	bool altKeyPressed;

	VertexShaderAdditionalData additionalShaderData;

	std::vector<Well*> wellList;
	std::vector<Particle*> particleList;

	MyParticlesViewer() : Viewer(viewerName, 1280, 720) {}

	void init() override {
		cubePosition = glm::vec3(1.f, 0.25f, -1.f);
		jointPosition = glm::vec3(-1.f, 2.f, -1.f);
		boneAngle = 0.f;
		mousePos = { 0.f, 0.f };
		leftMouseButtonPressed = false;

		altKeyPressed = false;

		additionalShaderData.Pos = { 0.,0.,0. };
		//INIT PARTICLES & WELLS
		for (int i = 0; i < numParticles; i++) {
			int randomW = rand() % viewportWidth;
			int randomH = rand() % viewportHeight;
			Particle* p = new Particle(glm::vec2(randomW, randomH));
			particleList.push_back(p);
		}

		for (int i = 0; i < numWells; i++) {
			int randomW = rand() % viewportWidth;
			int randomH = rand() % viewportHeight;
			Well *w = new Well(glm::vec2(randomW, randomH));
			wellList.push_back(w);
		}
	}


	void update(double elapsedTime) override {
		boneAngle = (float)elapsedTime;

		leftMouseButtonPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		altKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		mousePos = { float(mouseX), viewportHeight - float(mouseY) };

		pCustomShaderData = &additionalShaderData;
		CustomShaderDataSize = sizeof(VertexShaderAdditionalData);

		//UPDATE PARTICLES
		for (Particle* particle : particleList) {
			particle->updatePosition(wellList);
		}
	}

	void render3D_custom(const RenderApi3D& api) const override {
		//Here goes your drawcalls affected by the custom vertex shader
		//api.horizontalPlane({ 0, 2, 0 }, { 4, 4 }, 200, glm::vec4(0.0f, 0.2f, 1.f, 1.f));
	}

	void render3D(const RenderApi3D& api) const override {
		/*api.horizontalPlane({ 0, 0, 0 }, { 10, 10 }, 1, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));

		api.grid(10.f, 10, glm::vec4(0.5f, 0.5f, 0.5f, 1.f), nullptr);

		api.axisXYZ(nullptr);

		constexpr float cubeSize = 0.5f;
		glm::mat4 cubeModelMatrix = glm::translate(glm::identity<glm::mat4>(), cubePosition);
		api.solidCube(cubeSize, white, &cubeModelMatrix);

		{
			glm::vec3 vertices[] = {
				{0.5f * cubeSize, 0.5f * cubeSize, 0.5f * cubeSize},
				{0.f, cubeSize, 0.f},
				{0.5f * cubeSize, 0.5f * cubeSize, -0.5f * cubeSize},
				{0.f, cubeSize, 0.f},
				{-0.5f * cubeSize, 0.5f * cubeSize, 0.5f * cubeSize},
				{0.f, cubeSize, 0.f},
				{-0.5f * cubeSize, 0.5f * cubeSize, -0.5f * cubeSize},
				{0.f, cubeSize, 0.f},
			};
			api.lines(vertices, COUNTOF(vertices), white, &cubeModelMatrix);
		}

		{
			glm::quat q = glm::angleAxis(boneAngle, glm::vec3(0.f, 1.f, 0.f));
			glm::vec3 childRelPos = { 1.f, 1.f, 0.f };
			api.bone(childRelPos, white, q, glm::vec3(0.f, 0.f, 0.f));
			glm::vec3 childAbsPos = q * childRelPos;
			api.solidSphere(childAbsPos, 0.05f, 10, 10, white);
		}

		api.solidSphere(glm::vec3(-1.f, 0.5f, 1.f), 0.5f, 100, 100, white);*/
	}

	void render2D(const RenderApi2D& api) const override {

		constexpr float padding = 50.f;

		// Circle/ square following mouse position
		// Shows how to use inputs
		if (altKeyPressed) {
			if (leftMouseButtonPressed) {
				api.circleFill(mousePos, padding, 10, white);
			}
			else {
				api.circleContour(mousePos, padding, 10, white);
			}

		}
		else {
			const glm::vec2 min = mousePos + glm::vec2(padding, padding);
			const glm::vec2 max = mousePos + glm::vec2(-padding, -padding);
			if (leftMouseButtonPressed) {
				api.quadFill(min, max, white);
			}
			else {
				api.quadContour(min, max, white);
			}
		}

		// draw debug arrow
		{
			const glm::vec2 from = { viewportWidth * 0.5f, padding };
			const glm::vec2 to = { viewportWidth * 0.5f, 2.f * padding };
			constexpr float thickness = padding * 0.25f;
			constexpr float hatRatio = 0.3f;
			api.arrow(from, to, thickness, hatRatio, white);
		}

		//// draw lines
		//{
		//	glm::vec2 vertices[] = {
		//		{ padding, viewportHeight - padding },
		//		{ viewportWidth * 0.5f, viewportHeight - 2.f * padding },
		//		{ viewportWidth * 0.5f, viewportHeight - 2.f * padding },
		//		{ viewportWidth - padding, viewportHeight - padding },
		//	};
		//	api.lines(vertices, COUNTOF(vertices), white);
		//}

		//DRAW PARTICLES & WELLS
		for (Particle* particle : particleList) {
			api.circleContour(particle->position, particle->padding, 20, pink);
		}
		for (Well* well : wellList) {
			api.circleContour(well->position, well->padding, 20, white);
		}


	}

	void drawGUI() override {
		static bool showDemoWindow = false;

		ImGui::Begin("3D Sandbox");

		ImGui::Checkbox("Show demo window", &showDemoWindow);

		ImGui::ColorEdit4("Background color", (float*)&backgroundColor, ImGuiColorEditFlags_NoInputs);

		ImGui::SliderFloat("Point size", &pointSize, 0.1f, 10.f);
		ImGui::SliderFloat("Line Width", &lineWidth, 0.1f, 10.f);
		ImGui::Separator();
		ImGui::SliderFloat3("Light dir", (float(&)[3])lightDir, -1.f, 1.f);
		ImGui::SliderFloat("Light Strength", &lightStrength, 0.f, 2.f);
		ImGui::SliderFloat("Ligh Ambient", &lightAmbient, 0.f, 0.5f);
		ImGui::SliderFloat("Ligh Specular", &specular, 0.f, 1.f);
		ImGui::SliderFloat("Ligh Specular Pow", &specularPow, 1.f, 200.f);
		ImGui::Separator();
		ImGui::SliderFloat3("CustomShader_Pos", &additionalShaderData.Pos.x, -10.f, 10.f);
		ImGui::Separator();
		float fovDegrees = glm::degrees(camera.fov);
		if (ImGui::SliderFloat("Camera field of fiew (degrees)", &fovDegrees, 15, 180)) {
			camera.fov = glm::radians(fovDegrees);
		}

		ImGui::SliderFloat3("Cube Position", (float(&)[3])cubePosition, -1.f, 1.f);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();

		if (showDemoWindow) {
			// Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
			ImGui::ShowDemoWindow(&showDemoWindow);
		}
	}
};