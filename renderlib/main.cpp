#include <iostream>
#include <string>

#pragma comment(lib, "./lib/glew32s.lib")
#pragma comment(lib, "./lib/glfw3.lib")
#pragma comment(lib, "opengl32.lib")

// GLEW
#define GLEW_STATIC
#include "GL/glew.h"

// GLFW
#include "GLFW/glfw3.h"

const int g_window_width = 800;
const int g_window_height = 600;
GLFWwindow* g_window = nullptr;

std::string vertex_shader =
{
	"	#version 330\n"
	"	uniform		mat4	ProjMtx;						"
	"	in			vec2	Position;						"
	"	in			vec2	UV;								"
	"	in			vec4	Color;							"
	"	out			vec2	Frag_UV;						"
	"	out			vec4	Frag_Color;						"
	"	void main()											"
	"	{													"
	"		Frag_UV = UV;									"
	"		Frag_Color = Color;								"
	"		gl_Position = ProjMtx * vec4(Position.xy,0,1);	"
	"	}													"
};

std::string fragment_shader =
{
	"#version 330\n"
	"uniform sampler2D Texture;"
	"in vec2 Frag_UV;"
	"in vec4 Frag_Color;"
	"out vec4 Out_Color;"
	"void main()"
	"{"
	"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);"
	"}"
};


int main()
{
	//initialise glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	g_window = glfwCreateWindow(g_window_width, g_window_height, "renderlib", nullptr, nullptr);

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, g_window_width, g_window_height);


}