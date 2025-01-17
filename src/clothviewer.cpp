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

struct MyClothViewer : Viewer {

	//CLOTH STUFF
	float gravity = 0.9f;
	double previousTime = 0.0f;

	const int clothWidth = 10;
	const int clothHeight = 10;

	std::vector<glm::vec3> clothNodes;
	std::vector<glm::vec3> clothVelocities;
	std::vector<bool> applyPhysics;

	std::vector<int> controls;
	std::vector<glm::vec3> controlPositions;

	glm::vec3 lookNodes(int x, int y) const{
		if (x >= 0 && y >= 0 && x < clothWidth && y < clothHeight)
			return clothNodes[getIndex(x, y)];
		else
			return { 0, 0, 0 };
	}

	int getIndex(int x, int y) const
	{
		return y * clothWidth + x;
	}

	std::vector<glm::vec3> getAdjacents(int i) {
		std::vector<glm::vec3> Adjacents;

		if (i % clothWidth + 1 < clothWidth)
			Adjacents.push_back(lookNodes(i % clothWidth + 1, i / clothHeight));
		if (i / clothHeight + 1 < clothHeight)
			Adjacents.push_back(lookNodes(i % clothWidth, i / clothHeight + 1));
		if (i % clothWidth - 1 >= 0)
			Adjacents.push_back(lookNodes(i % clothWidth - 1, i / clothHeight));
		if (i / clothHeight - 1 >= 0)
			Adjacents.push_back(lookNodes(i % clothWidth, i / clothHeight - 1));

		return Adjacents;
	}

	std::vector<glm::vec3> getDiagonalAdjacents(int i) {
		std::vector<glm::vec3> Adjacents;

		if (i % clothWidth + 1 < clothWidth && i / clothHeight + 1 < clothHeight)
			Adjacents.push_back(lookNodes(i % clothWidth + 1, i / clothHeight + 1));

		if (i % clothWidth + 1 < clothWidth && i / clothHeight - 1 >= 0)
			Adjacents.push_back(lookNodes(i % clothWidth + 1, i / clothHeight - 1));

		if (i % clothWidth - 1 >= 0 && i / clothHeight + 1 < clothHeight)
			Adjacents.push_back(lookNodes(i % clothWidth - 1, i / clothHeight + 1));

		if (i % clothWidth - 1 >= 0 && i / clothHeight - 1 >= 0)
			Adjacents.push_back(lookNodes(i % clothWidth - 1, i / clothHeight - 1));

		return Adjacents;
	}

	std::vector<glm::vec3> getDoubleAdjacents(int i) {
		std::vector<glm::vec3> Adjacents;

		if (i % clothWidth + 2 < clothWidth)
			Adjacents.push_back(lookNodes(i % clothWidth + 2, i / clothHeight));
		if (i / clothHeight + 2 < clothHeight)
			Adjacents.push_back(lookNodes(i % clothWidth, i / clothHeight + 2));
		if (i % clothWidth - 2 >= 0)
			Adjacents.push_back(lookNodes(i % clothWidth - 2, i / clothHeight));
		if (i / clothHeight - 2 >= 0)
			Adjacents.push_back(lookNodes(i % clothWidth, i / clothHeight - 2));

		return Adjacents;
	}

	void applyForces(double deltaTime) {
		//Gravity

		for (int i = 0; i < clothWidth * clothHeight; i++)
		{
			(clothVelocities[i]) = glm::vec3();

			if (applyPhysics[i])
			{
				clothVelocities[i].y -= deltaTime * gravity * 0.7f;
			}
			else
			{
				clothVelocities[i] = glm::vec3();
			}
		}

		//Attraction
		for (int i = 0; i < clothWidth * clothHeight; i++)
		{
			for each (glm::vec3 n in getAdjacents(i))
			{
				const float rl = 1.0f;
				glm::vec3 diff = lookNodes(i % clothWidth, i / clothHeight) - n;
				clothVelocities[i] += (5.0f * (rl - glm::length(diff)) * (diff / glm::length(diff))) * static_cast<float>(deltaTime);
			}

			for each (glm::vec3 diagN in getDiagonalAdjacents(i))
			{
				const float rl = std::sqrt(2.0) * 1.0f;
				glm::vec3 diff = lookNodes(i % clothWidth, i / clothHeight) - diagN;
				clothVelocities[i] += (5.0f * (rl - glm::length(diff)) * (diff / glm::length(diff))) * static_cast<float>(deltaTime);
			}

			for each (glm::vec3 doubN in getDoubleAdjacents(i))
			{
				const float rl = 2 * 1.0f;
				glm::vec3 diff = lookNodes(i % clothWidth, i / clothHeight) - doubN;
				clothVelocities[i] += (5.0f * (rl - glm::length(diff)) * (diff / glm::length(diff))) * static_cast<float>(deltaTime);
			}
			if (applyPhysics[i])
				clothNodes[i] += clothVelocities[i];
		}
	}
	//
	glm::vec3 jointPosition;
	glm::vec3 cubePosition;
	float boneAngle;

	glm::vec2 mousePos;

	bool leftMouseButtonPressed;
	bool altKeyPressed;

	VertexShaderAdditionalData additionalShaderData;

	MyClothViewer() : Viewer(viewerName, 1280, 720) {}

	void init() override {
		cubePosition = glm::vec3(1.f, 0.25f, -1.f);
		jointPosition = glm::vec3(-1.f, 2.f, -1.f);
		boneAngle = 0.f;
		mousePos = { 0.f, 0.f };
		leftMouseButtonPressed = false;

		altKeyPressed = false;

		additionalShaderData.Pos = { 0.,0.,0. };
		for (int i = 0; i < clothWidth * clothHeight; i++) {
			clothNodes.push_back(glm::vec3(i % clothWidth, 0, i / clothHeight));
			applyPhysics.push_back(true);
			clothVelocities.push_back(glm::vec3());
		}
		controls.push_back(getIndex(0, 0));
		controls.push_back(getIndex(clothWidth - 1, 0));

		for (int point : controls)
		{
			applyPhysics[point] = false;
			controlPositions.push_back(clothNodes[point]);
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
		applyForces(elapsedTime - previousTime);
		previousTime = elapsedTime;
	}

	void render3D_custom(const RenderApi3D& api) const override {
		//Here goes your drawcalls affected by the custom vertex shader
		//api.horizontalPlane({ 0, 2, 0 }, { 4, 4 }, 200, glm::vec4(0.0f, 0.2f, 1.f, 1.f));
	}

	void render3D(const RenderApi3D& api) const override {
		//api.horizontalPlane({ 0, 0, 0 }, { 10, 10 }, 1, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));

		//api.grid(10.f, 10, glm::vec4(0.5f, 0.5f, 0.5f, 1.f), nullptr);

		api.axisXYZ(nullptr);

		/*constexpr float cubeSize = 0.5f;
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

		api.axisXYZ(nullptr);
		glm::vec3* line = new glm::vec3[2]();
		for (int i = 0; i < clothWidth * clothHeight; i++)
		{

			if (i % clothWidth + 1 < clothWidth)
			{
				line[0] = lookNodes(i % clothWidth, i / clothHeight);
				line[1] = lookNodes(i % clothWidth + 1, i / clothHeight);
				api.lines(line, 2, white, nullptr);
			}
			if (i / clothHeight + 1 < clothHeight)
			{
				line[0] = lookNodes(i % clothWidth, i / clothHeight);
				line[1] = lookNodes(i % clothWidth, i / clothHeight + 1);
				api.lines(line, 2, white, nullptr);
			}
		}
		delete[] line;
	}

	void render2D(const RenderApi2D& api) const override {

		//constexpr float padding = 50.f;

		// Circle/ square following mouse position
		// Shows how to use inputs
		//if (altKeyPressed) {
		//	if (leftMouseButtonPressed) {
		//		api.circleFill(mousePos, padding, 10, white);
		//	}
		//	else {
		//		api.circleContour(mousePos, padding, 10, white);
		//	}

		//}
		//else {
		//	const glm::vec2 min = mousePos + glm::vec2(padding, padding);
		//	const glm::vec2 max = mousePos + glm::vec2(-padding, -padding);
		//	if (leftMouseButtonPressed) {
		//		api.quadFill(min, max, white);
		//	}
		//	else {
		//		api.quadContour(min, max, white);
		//	}
		//}

		//// draw debug arrow
		//{
		//	const glm::vec2 from = { viewportWidth * 0.5f, padding };
		//	const glm::vec2 to = { viewportWidth * 0.5f, 2.f * padding };
		//	constexpr float thickness = padding * 0.25f;
		//	constexpr float hatRatio = 0.3f;
		//	api.arrow(from, to, thickness, hatRatio, white);
		//}

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
	}

	void drawGUI() override {
		static bool showDemoWindow = false;

		ImGui::Begin("3D Sandbox");

		ImGui::Checkbox("Show demo window", &showDemoWindow);

		ImGui::ColorEdit4("Background color", (float*)&backgroundColor, ImGuiColorEditFlags_NoInputs);

		ImGui::SliderFloat("Point size", &pointSize, 0.1f, 10.f);
		ImGui::SliderFloat("Line Width", &lineWidth, 0.1f, 10.f);
		ImGui::Separator();
		//ImGui::SliderFloat3("Light dir", (float(&)[3])lightDir, -1.f, 1.f);
		//ImGui::SliderFloat("Light Strength", &lightStrength, 0.f, 2.f);
		//ImGui::SliderFloat("Ligh Ambient", &lightAmbient, 0.f, 0.5f);
		//ImGui::SliderFloat("Ligh Specular", &specular, 0.f, 1.f);
		//ImGui::SliderFloat("Ligh Specular Pow", &specularPow, 1.f, 200.f);
		ImGui::SliderFloat("Gravity", &gravity, 0.0f, 9.0f);
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
