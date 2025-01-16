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
#include "defaultviewer.cpp"
#include "boidsviewer.cpp"
#include "particlesviewer.cpp"



int main(int argc, char** argv) {
	//MyDefaultViewer v;
	//MyBoidsViewer v;
	MyParticlesViewer v;
	return v.run();
}
