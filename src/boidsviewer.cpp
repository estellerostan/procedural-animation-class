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

#include<cstdlib>
#include <vector>
#include <iostream>

struct MyBoidsViewer : Viewer {

	glm::vec3 jointPosition;
	glm::vec3 cubePosition;
	float boneAngle;

	glm::vec2 mousePos;

	bool leftMouseButtonPressed;
	bool altKeyPressed;

	VertexShaderAdditionalData additionalShaderData;

	std::vector<glm::vec2> boidsPositions;

	MyBoidsViewer() : Viewer(viewerName, 1280, 720) {}

	void initBoidsPos() {
		// based on screen width/ height

		constexpr float padding = 50.f;

		// scatter them at each sides of the screen
		std::vector<glm::vec2> basePositions = {
			// top left
			{ padding, viewportHeight - padding },
			// top middle
			//{ viewportWidth * 0.5f, viewportHeight - padding },
			// top right
			{ viewportWidth - padding, viewportHeight - padding },
			// bottom left
			{ 0 + padding, 0 + padding },
			// bottom right
			{ viewportWidth - padding, 0 + padding },
		};

		int length = 100;
		for (size_t i = 0; i < length; i++)
		{
			int randomOffset = rand() % 1000 + 1;
			int randomOffsetY = rand() % 1000 + 1;
			std::cout << randomOffset << ":" << randomOffsetY << std::endl;

			glm::vec2 res = { basePositions[i % basePositions.size()].x + randomOffset,  basePositions[i % basePositions.size()].y + randomOffsetY };
			// std::cout << res.x << ":" << res.y << std::endl;

			boidsPositions.push_back(res);
		}
	}

	void init() override {
		cubePosition = glm::vec3(1.f, 0.25f, -1.f);
		jointPosition = glm::vec3(-1.f, 2.f, -1.f);
		boneAngle = 0.f;
		mousePos = { 0.f, 0.f };
		leftMouseButtonPressed = false;

		altKeyPressed = false;

		additionalShaderData.Pos = { 0.,0.,0. };

		// initialize random seed
		srand(time(NULL));

		initBoidsPos();
	}

	float distance(glm::vec2 boid1, glm::vec2 boid2) {
		return std::sqrt(
			(boid1.x - boid2.x) * (boid1.x - boid2.x) +
			(boid1.y - boid2.y) * (boid1.y - boid2.y)
			);
	}

	void flyTowardsCenter(glm::vec2& boid) {
		float centeringFactor = 0.005; // adjust velocity by this %

		int centerX = 0;
		int centerY = 0;
		int numNeighbors = 0;

		for (size_t j = 0; j < boidsPositions.size(); j++)
		{
			glm::vec2 otherBoid = boidsPositions[j];
			// TODO: variable
			int visualRange = 175;
			if (distance(boid, otherBoid) < visualRange) {
				centerX += otherBoid.x;
				centerY += otherBoid.y;
				numNeighbors += 1;
			}
		}

		if (numNeighbors) {
			centerX = centerX / numNeighbors;
			centerY = centerY / numNeighbors;

			// TODO: deltaTime from elapsedTime
			// TODO: make this a boid property to update the position based on the current velocity all at once with other rules
			boid.x += (centerX - boid.x) * centeringFactor;
			boid.y += (centerY - boid.y) * centeringFactor;
		}
	}

	void moveBoidsToNewPos() {
		for (size_t i = 0; i < boidsPositions.size(); i++)
		{
			flyTowardsCenter(boidsPositions[i]);
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

		moveBoidsToNewPos();
	}

	void render3D_custom(const RenderApi3D& api) const override {
	}

	void render3D(const RenderApi3D& api) const override {
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

		// draw lines
		{
			glm::vec2 vertices[] = {
				{ padding, viewportHeight - padding },
				{ viewportWidth * 0.5f, viewportHeight - 2.f * padding },
				{ viewportWidth * 0.5f, viewportHeight - 2.f * padding },
				{ viewportWidth - padding, viewportHeight - padding },
			};
			api.lines(vertices, COUNTOF(vertices), white);
		}

		//  draw_boids()
		for each (glm::vec2 boidPos in boidsPositions)
		{
			// TODO: change shape for triagle/ make it a parameter?
			// TODO: create a custom struct to store boid info??
			// arr[i] = value;
			//std::cout << boidPos.x << ":" << boidPos.y << std::endl;
			api.circleFill(boidPos, padding / 2, 5, white);
		}
	}

	void drawGUI() override {
		static bool showDemoWindow = false;

		ImGui::Begin("3D Sandbox");

		ImGui::Checkbox("Show demo window", &showDemoWindow);

		ImGui::ColorEdit4("Background color", (float*)&backgroundColor, ImGuiColorEditFlags_NoInputs);

		ImGui::SliderFloat("Point size", &pointSize, 0.1f, 10.f);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();

		if (showDemoWindow) {
			// Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
			ImGui::ShowDemoWindow(&showDemoWindow);
		}
	}
};
