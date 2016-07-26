#include <iostream>

//include libraries
#pragma comment(lib, "./lib/glew32s.lib")
#pragma comment(lib, "./lib/glfw3.lib")
#pragma comment(lib, "opengl32.lib")

// GLEW
#define GLEW_STATIC
#include "GL/glew.h"

// GLFW
#include "GLFW/glfw3.h"

//imgui
#include "imgui/imgui.h"

//window variables
const int g_window_width = 800;
const int g_window_height = 600;
GLFWwindow* g_window = nullptr;

//GLhandles
GLint g_shader;
GLint g_vert;
GLint g_frag;

GLint g_uniform_proj;
GLint g_uniform_tex;
GLint g_attrib_pos;
GLint g_attrib_uv;
GLint g_attrib_col;


//shaders
const GLchar* vertex_shader =
{
	"	#version 330\n											"
	"	uniform		mat4	proj_mtx;							"
	"	in			vec2	position;							"
	"	in			vec2	uv;									"
	"	in			vec4	color;								"
	"	out			vec2	frag_uv;							"
	"	out			vec4	frag_color;							"
	"	void main()												"
	"	{														"
	"		frag_uv = uv;										"
	"		frag_color = color;									"
	"		gl_Position = proj_mtx * vec4(position.xy,0,1);		"
	"	}														"
};
const GLchar* fragment_shader =
{
	"	#version 330\n												"
	"	uniform		sampler2D	tex;								"
	"	in			vec2		frag_uv;							"
	"	in			vec4		frag_color;							"
	"	out			vec4		out_color;							"
	"	void main()													"
	"	{															"
	"		out_color = frag_color * texture( tex, frag_uv.st);		"
	"	}															"
};

int main()
{
	//initialise glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//set up window
	g_window = glfwCreateWindow(g_window_width, g_window_height, "renderlib", nullptr, nullptr);

	//initialize glew
	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, g_window_width, g_window_height);

	//compile shaders
	g_shader = glCreateProgram();
	g_vert = glCreateShader(GL_VERTEX_SHADER);
	g_frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g_vert, 1, &vertex_shader, 0);
	glShaderSource(g_frag, 1, &fragment_shader, 0);
	glCompileShader(g_vert);
	glCompileShader(g_frag);
	glAttachShader(g_shader, g_vert);
	glAttachShader(g_shader, g_frag);
	glLinkProgram(g_shader);

	//set up uniforms and attributes;
	g_uniform_proj = glGetUniformLocation(g_shader, "proj_mtx");
	g_uniform_tex = glGetUniformLocation(g_shader, "tex");
	g_attrib_pos = glGetAttribLocation(g_shader, "position");
	g_attrib_uv = glGetAttribLocation(g_shader, "uv");
	g_attrib_col = glGetAttribLocation(g_shader, "color");



}
