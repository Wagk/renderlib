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
#include "fft_impl.h"
#include "LowHighPassFilter.h"
#include "Compressor.h"
#include "BiquadFilter.h"
#include "our_fft.h"

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
		//auto sample = io::wav::ToSample(sound_data.first);
		//auto wavfile = io::wav::ToFile(sample);
		//io::wav::SaveWAV(save_file, wavfile);
		//
		//
		//auto compress_sample = sample;
		//
		//CompressionPacket sample_packet(compress_sample.m_samples);
		//compress_sample.m_samples = Compressor(sample_packet)();
		//wavfile = io::wav::ToFile(compress_sample);
		//io::wav::SaveWAV("compress.wav", wavfile);
	}
	auto pair_data = io::wav::ToSample(sound_data.first);

	//auto sin_data = pair_data;
	//for (size_t i = 0; i < sin_data.m_samples.size(); ++i)
	//{
	//	sin_data.m_samples[i] = std::sin(i);
	//}

	std::vector<float> shortvec(sound_data.first.m_data.size());
	std::copy(sound_data.first.m_data.begin(), sound_data.first.m_data.end(), shortvec.begin());
	//for (auto elem : sound_data.first.m_data)
	//{
	//	shortvec.push_back(elem);
	//}

	const unsigned windowSize = 2048;

	SignalAnalyst::Init();
	SignalAnalyst::SelectWindowFunction(WTF_HANNING);
	SignalAnalyst::SetWindowSize(windowSize);
	SignalAnalyst::SetDataReference(pair_data.m_samples.data(), pair_data.m_samples.size());
	SignalAnalyst::SetDataRate(pair_data.m_header.sample_rate);

	// Do this here because we only need it done once; if we support file change or updating above settings, plot needs to be recomputed
	SignalAnalyst::ComputePlot();

	sys_init();

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	//globals for button
	bool playsound = false;
	std::string playbuttonTxt = "play";
	float skipval = 0;
	float delta_t = 0.f;
	float time_prev = 0.f;
	float time_now = 0.f;

	// for compressor
	bool compressor_Enable = false;

	//for high and low pass
	bool cutoff_Enable = false;
	float cutoffLow = 100.f;
	float cutoffHigh = 100.f;
	std::vector<float> lowpass, highpass;

  // --- for biquad filters
  Biquad::BiquadFilterTypes biquad_current_filter = Biquad::BiquadFilterTypes::LOW_PASS;
  std::vector<float> biquad_out;
  float biquad_centerFreq = 440;
  float biquad_bandwidth = 15;
  bool biquad_enable = false;
  bool biquad_autorecompute = false;
  bool biquad_recompute = true;

  auto biquad_sound_data = io::wav::LoadWAV("biquad_out.wav");
  auto biquad_sound_sample = io::wav::ToSample(biquad_sound_data.first);
  std::vector<float> biquad_sound_float_data(biquad_sound_sample.m_samples.size());
  std::copy(biquad_sound_sample.m_samples.begin(), biquad_sound_sample.m_samples.end(), biquad_sound_float_data.begin());
  // --- for biquad filters

	// Final steps for spectrum rendering...
	const unsigned render_width = 512;
	std::vector<float> spectrum_graph(512, 0.0f);

	float x_min = pair_data.m_header.sample_rate / static_cast<float>(windowSize);
	float x_max = pair_data.m_header.sample_rate / 2.0f;
	float x_step = (x_max - x_min) / render_width;
	float x_pos = x_min;

	for (unsigned i = 0U; i < render_width; ++i)
	{
		spectrum_graph[i] = SignalAnalyst::GetProcessedValue(x_pos, x_pos + x_step);
		x_pos += x_step;
	}

	// Main loop
	while (!glfwWindowShouldClose(g_context))
	{
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		time_prev = time_now;
		time_now = ImGui::GetTime();
		delta_t = time_now - time_prev;
		skipval += delta_t;

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
			//ImGui::Checkbox("Animate", &animate);
			static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
			//ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));


			static float values[90] = { 0 };
			static int values_offset = 0;
			if (animate)
			{
				static float refresh_time = ImGui::GetTime(); // Create dummy data at fixed 60 hz rate for the demo
				for (; ImGui::GetTime() > refresh_time + 1.0f / 60.0f; refresh_time += 1.0f / 60.0f)
				{
					static float phase = 0.0f;
					values[values_offset] = cosf(phase);
					values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
					phase += 0.10f*values_offset;
				}
			}


			//ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, "avg 0.0", -1.0f, 1.0f, ImVec2(0, 80));
			//ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0,80));

			

			ImGui::PlotLines("Entire Waveform", pair_data.m_samples.data(), pair_data.m_samples.size() / 2, 0, "", -1.0f, 1.0f, ImVec2(0, 100), 4);


			ImGui::Separator();
			ImGui::Checkbox("Enable Compressor", &compressor_Enable);
			if (compressor_Enable)
			{
				const int view_buffer_size = 90;
				float view_buffer[view_buffer_size] = { 0 };
				for (int i = 0; i < view_buffer_size; ++i)
				{
					view_buffer[i] = pair_data.m_samples[(i + (int)skipval) % pair_data.m_samples.size()];
				}
				std::vector<float> HACK_BUFFER(view_buffer, view_buffer + view_buffer_size);
				CompressionPacket pkt(HACK_BUFFER);
				//
				//
				std::vector<float> result = Compressor(pkt)();



				ImGui::PlotLines("Dynamic Waveform", view_buffer, view_buffer_size, 0, "", -1.0f, 1.0f, ImVec2(0, 100), 4);
				ImGui::PlotLines("Compressed Dynamic Waveform", result.data(), result.size(), 0, "input", -1.0f, 1.0f, ImVec2(0, 100));
				ImGui::PlotHistogram("File Value Histogram", shortvec.data(), shortvec.size() / 2, 0, NULL, SHRT_MIN, SHRT_MAX, ImVec2(0, 100));

				static const float Amplitude = 2.0f;
				std::vector<float> sinVec(512);
				for (unsigned i = 0; i < sinVec.size(); ++i)
				{
					sinVec[i] = Amplitude * sinf(i * 0.5f) + Amplitude*1.5f * sinf(i * 0.2f);
				}

				ImGui::PlotLines("Uncompressed Waveform", sinVec.data(), sinVec.size(), 0, "input", -1.0f, 1.0f, ImVec2(0, 100));

				CompressionPacket pkt2(sinVec);
				std::vector<float> result_sinVec = Compressor(pkt2)();

				ImGui::PlotLines("Compressed Waveform", result_sinVec.data(), result_sinVec.size(), 0, "input", -1.0f, 1.0f, ImVec2(0, 100));
				// high low pass filters
				// Render spectrum
				ImGui::PlotLines("Spectrum", spectrum_graph.data(), spectrum_graph.size(), 0, "file data", -46.0f, -10.0f, ImVec2(0, 100));
			}
			// Use functions to generate output
			// FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
			struct Funcs
			{
				static float Sin(void*, int i) { return sinf(i * 0.1f); }
				static float Saw(void*, int i) { return (i & 1) ? 1.0f : 0.0f; }
			};
			static int func_type = 0, display_count = 70;

			ImGui::Separator();
			ImGui::Checkbox("Enable Low/High Pass Filters", &cutoff_Enable);
			if (cutoff_Enable)
			{
				ImGui::PlotLines("Entire Waveform", pair_data.m_samples.data(), pair_data.m_samples.size() / 2, 0, "", -1.0f, 1.0f, ImVec2(0, 100), 4);

				lowpass = LowPassFilter(pair_data.m_samples, sound_data.first.m_header.sample_rate, cutoffLow);
				//fft.do_ifft(lowpass.data(), ifft_output.data());
				//fft.rescale(ifft_output.data());
				ImGui::PlotLines("Low", lowpass.data(), pair_data.m_samples.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));
				ImGui::SliderFloat("Cutoff Low", &cutoffLow, 100, 10000);

				highpass = HighPassFilter(pair_data.m_samples, sound_data.first.m_header.sample_rate, cutoffHigh);
				ImGui::PlotLines("High", highpass.data(), highpass.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));
				ImGui::SliderFloat("Cutoff High", &cutoffHigh, 100, 10000);
				// end high low pass filters

			}

		ImGui::Separator();
		// begin Biquad Filters
	  ImGui::Checkbox("Enable Biquad Filters", &biquad_enable);
	  if (biquad_enable)
	  {
		  ImGui::Checkbox("Always Recompute On Change", &biquad_autorecompute);

		  bool dummy_bool = false;
		  dummy_bool |= ImGui::SliderFloat("Center Frequency", &biquad_centerFreq, 100, 10000);
		  dummy_bool |= ImGui::SliderFloat("Bandwidth", &biquad_bandwidth, 0, 100);
		  dummy_bool |= ImGui::RadioButton("LOW PASS", (int*)&biquad_current_filter, 0);
		  dummy_bool |= ImGui::RadioButton("HIGH PASS", (int*)&biquad_current_filter, 1);
		  dummy_bool |= ImGui::RadioButton("BAND PASS", (int*)&biquad_current_filter, 2);

		  if (biquad_autorecompute)
			  biquad_recompute |= dummy_bool;

		  if (ImGui::Button("Recompute", ImVec2(100, 30)))
		  {
			  biquad_recompute = true;
		  }

		  if (biquad_recompute)
		  {
			  biquad_out.clear();

			  Biquad::BiquadObject obj = Biquad::GetBiquadObject(biquad_current_filter, biquad_centerFreq,
				  sound_data.first.m_header.sample_rate, biquad_bandwidth);
			  for (const auto& sample : pair_data.m_samples)
			  {
				  biquad_out.push_back(Biquad::PerformBiquadOnSample(sample, obj));
			  }

			  ImGui::PlotLines("Biquad Filter Output", biquad_out.data(), biquad_out.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));

			  biquad_recompute = false;
		  }

		  if (ImGui::Button("Save To File", ImVec2(100, 30)))
		  {
			  io::wav::sample biquad_sample = pair_data;
			  biquad_sample.m_samples = biquad_out;
			  io::wav::file biquad_file = io::wav::ToFile(biquad_sample);
			  io::wav::SaveWAV("biquad_out.wav", biquad_file);

			  biquad_sound_data = io::wav::LoadWAV("biquad_out.wav");
			  io::wav::sample inputsample = io::wav::ToSample(biquad_sound_data.first);
			  biquad_sound_float_data.resize(inputsample.m_samples.size());
			  std::copy(inputsample.m_samples.begin(), inputsample.m_samples.end(), biquad_sound_float_data.begin());
		  }

		  ImGui::PlotLines("Biquad Out Waveform", biquad_sound_float_data.data(), biquad_sound_float_data.size() / 2, 0, "", -1.f, 1.f, ImVec2(0, 100), 4);

	  }

	  ImGui::Separator();
      // end Biquad Filters

			//// Use functions to generate output
			//// FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
			//struct Funcs
			//{
			//	static float Sin(void*, int i) { return sinf(i * 0.1f); }
			//	static float Saw(void*, int i) { return (i & 1) ? 1.0f : 0.0f; }
			//};
			//static int func_type = 0, display_count = 70;
			//ImGui::Separator();

			//ImGui::PushItemWidth(100);
			//{
			//	ImGui::Combo("func", &func_type, "Sin\0Saw\0");
			//}
			//ImGui::PopItemWidth();

			//ImGui::SameLine();

			////ImGui::SliderInt("Sample count", &display_count, 1, 500);
			//float(*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
			////ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0,80));
			////ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0,80));

			//ImGui::Separator();

			//// Animate a simple progress bar
			//static float progress = 0.0f, progress_dir = 1.0f;
			//if (animate)
			//{
			//	progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
			//	if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
			//	if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
			//}

			//// Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
			////ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f));
			////ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			////ImGui::Text("Progress Bar");

			//float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
			//char buf[32];
			//sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
			////ImGui::ProgressBar(progress, ImVec2(0.f,0.f), buf);
		}
		ImGui::End();

		/*changes here*/
		io::wav::UpdateWavFileStatus("test.wav");
		std::string curWavFileStatus = io::wav::globalWavState;
		if (curWavFileStatus == "stopped")
		{
			playbuttonTxt = "play";
		}
		else if (curWavFileStatus == "playing")
		{
			playbuttonTxt = "pause";
		}
		else if (curWavFileStatus == "paused")
		{
			playbuttonTxt = "play";
		}
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

	SignalAnalyst::Deinit();

	// Cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();

	return 0;
}
