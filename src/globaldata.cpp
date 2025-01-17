#pragma once

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>

#define COUNTOF(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

constexpr char const* viewerName = "MyViewer";
constexpr glm::vec4 white = { 1.f, 1.f, 1.f, 1.f };
constexpr glm::vec4 blue = { 0.f, 0.f, 1.f, 1.f };
constexpr glm::vec4 green = { 0.f, 1.f, 0.f, 1.f };
constexpr glm::vec4 red = { 1.f, 0.f, 0.f, 1.f };
constexpr glm::vec4 pink = { 0.98f, 0.86f, 0.98f, 1.0f };
constexpr glm::vec4 translucideGreen = { 0.f, 1.f, 0.f, 0.4f };

struct VertexShaderAdditionalData {
	glm::vec3 Pos;
	/// beware of alignement (std430 rule)
};
