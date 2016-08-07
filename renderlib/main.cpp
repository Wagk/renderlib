#include <iostream>
#include <cstdio>

//include libraries
#pragma comment(lib, "./lib/glew32s.lib")
#pragma comment(lib, "./lib/glfw3.lib")
#pragma comment(lib, "opengl32.lib")

/*changes here*/
#pragma comment(lib, "winmm.lib")
/*to here*/

// GLEW
#define GLEW_STATIC
#include "GL/glew.h"

// GLFW
#include "GLFW/glfw3.h"

//imgui
#include "imgui/imgui.h"

// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui_impl_glfw_gl3.h"


#include "wav_loader.h"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

GLFWwindow* g_context = nullptr;


static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

static void sys_init()
{
	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		throw std::runtime_error("GLFW failed to init!");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	g_context = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
	glfwMakeContextCurrent(g_context);
	glewInit();

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(g_context, true);

	// Load Fonts
	// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
}

int main(int argc, char* argv[])
{
	std::string load_file;
	std::string save_file;

	if (argc != 3)
	{
		std::cout << "Loading from default test input" << std::endl;
		std::cout << "Saving to default test output" << std::endl;

		load_file = "test.wav";
		save_file = "test2.wav";
	}
	else
	{
		load_file = argv[1];
		save_file = argv[2];
	}


	auto sound_data = io::wav::LoadWAV(load_file);
	if (sound_data.second != io::wav::WAV_GOOD)
	{
		return -1;
	}
	else
	{
		io::wav::SaveWAV(save_file, sound_data.first);
	}
	auto pair_data = io::wav::FloatChannel(sound_data.first);



	sys_init();

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	//globals for button
	bool playsound = false;
	std::string playbuttonTxt = "play";

	// Main loop
	while (!glfwWindowShouldClose(g_context))
	{
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		//// 1. Show a simple window
		//// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		//{
		//	static float f = 0.0f;
		//	ImGui::Text("Hello, world!");
		//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		//	ImGui::ColorEdit3("clear color", (float*)&clear_color);
		//	if (ImGui::Button("Test Window")) show_test_window ^= 1;
		//	if (ImGui::Button("Another Window")) show_another_window ^= 1;
		//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//}

		//// 2. Show another simple window, this time using an explicit Begin/End pair
		//if (show_another_window)
		//{
		//	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		//	ImGui::Begin("Another Window", &show_another_window);
		//	ImGui::Text("Hello");
		//	ImGui::End();
		//}

		//// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		//if (show_test_window)
		//{
		//	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		//	ImGui::ShowTestWindow(&show_test_window);
		//}

		bool math_window = false;
		ImGui::Begin("Test", &math_window);
		{
			/*changes here*/
			if (ImGui::Button(playbuttonTxt.c_str(), ImVec2(50, 50)))
			{


				if (playbuttonTxt == "play")
				{
					io::wav::PlayWavFile("test.wav", true);

				}
				else
				{
					io::wav::PlayWavFile("test.wav", false);

				}


			}
			/*to here*/


			//stuff inside
			bool animate = true;
			ImGui::Checkbox("Animate", &animate);
			static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
			ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));


			static float values[90] = { 0 };
			static int values_offset = 0;
			if (animate)
			{
				static float refresh_time = ImGui::GetTime(); // Create dummy data at fixed 60 hz rate for the demo
				for (; ImGui::GetTime() > refresh_time + 1.0f/60.0f; refresh_time += 1.0f/60.0f)
				{
					static float phase = 0.0f;
					values[values_offset] = cosf(phase);
					values_offset = (values_offset+1) % IM_ARRAYSIZE(values);
					phase += 0.10f*values_offset;
				}
			}

			
			//ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, "avg 0.0", -1.0f, 1.0f, ImVec2(0,80));
			//ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0,80));

			//ImGui::PlotLines("CS225 Wav Sample", pair_data.data(), pair_data.size(), 0, "avg 0.0", -150.0f, 150.0f, ImVec2(0,80));
			ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0,80));

			// Use functions to generate output
			// FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
			struct Funcs
			{
				static float Sin(void*, int i) { return sinf(i * 0.1f); }
				static float Saw(void*, int i) { return (i & 1) ? 1.0f : 0.0f; }
			};
			static int func_type = 0, display_count = 70;
			ImGui::Separator();
			
			ImGui::PushItemWidth(100); 
			{
				ImGui::Combo("func", &func_type, "Sin\0Saw\0");
			}
			ImGui::PopItemWidth();

			ImGui::SameLine();

			ImGui::SliderInt("Sample count", &display_count, 1, 500);
			float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
			ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0,80));
			ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0,80));

			ImGui::Separator();

			// Animate a simple progress bar
			static float progress = 0.0f, progress_dir = 1.0f;
			if (animate)
			{
				progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
				if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
				if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
			}

			// Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
			ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f));
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text("Progress Bar");

			float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
			char buf[32];
			sprintf(buf, "%d/%d", (int)(progress_saturated*1753), 1753);
			ImGui::ProgressBar(progress, ImVec2(0.f,0.f), buf);
			}
			ImGui::End();

			/*changes here*/
			io::wav::UpdateWavFileStatus("test.wav");
			std::string curWavFileStatus = io::wav::globalWavState;
			if (curWavFileStatus == "stopped")
				playbuttonTxt = "play";
			else if (curWavFileStatus == "playing")
				playbuttonTxt = "pause";
			else if (curWavFileStatus == "paused")
				playbuttonTxt = "play";
			/*to here*/

			// Rendering
			int display_w, display_h;
			glfwGetFramebufferSize(g_context, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui::Render();
			glfwSwapBuffers(g_context);
	}

	// Cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();

	return 0;
}
