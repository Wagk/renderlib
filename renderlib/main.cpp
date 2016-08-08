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
#include "BassBoost.h"
#include "LeWahWah.h"
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

static void FindGraphMinMax(float& min, float& max, float* graph, size_t elemSize)
{
	min = FLT_MAX;
	max = -FLT_MAX;

	for (size_t i = 0; i < elemSize; ++i)
	{
		if (min > graph[i])
			min = graph[i];
		if (max < graph[i])
			max = graph[i];
	}
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
	bool compressor_Recompute = true;
	bool compressor_AutoRecompute = false;
	
	const int view_buffer_size = 90;
	static const float Amplitude = 2.0f;
	float view_buffer[view_buffer_size] = { 0 };
	std::vector<float> compressor_result;
	std::vector<float> sinVec(512);
	std::vector<float> result_sinVec;

	//for high and low pass
	bool cutoff_Enable = false;
	bool cutoff_Recompute = true;
	bool cutoff_AutoRecompute = false;
	float cutoffLow = 100.f;
	float cutoffHigh = 100.f;
	std::vector<float> lowpass, highpass;

	// --- for biquad filters
	Biquad::BiquadFilterTypes biquad_current_filter = Biquad::BiquadFilterTypes::LOW_PASS;
	std::vector<float> biquad_out;
	float biquad_centerFreq = 440;
	float biquad_bandwidth = 15;
	float biquad_dbGain = 40;
	bool biquad_enable = false;
	bool biquad_autorecompute = false;
	bool biquad_recompute = true;

	auto biquad_sound_data = io::wav::LoadWAV("biquad_out.wav");
	auto biquad_sound_sample = io::wav::ToSample(biquad_sound_data.first);
	std::vector<float> biquad_sound_float_data(biquad_sound_sample.m_samples.size());
	std::copy(biquad_sound_sample.m_samples.begin(), biquad_sound_sample.m_samples.end(), biquad_sound_float_data.begin());
	// --- for biquad filters

	//-- for bass boost filter

	bool bb_enable = false;
	bool recompute_bb = true;
	std::vector<float> bb_out;
	float bb_selectivity = 140.f;
	float bb_gain2 = 0.f;
	float bb_ratio = 0.f;

	//-- for bass boost filter

	// --- LeWahwah effect

	LeWahWah lww;
	lww.SetSampleRate(pair_data.m_header.sample_rate);

	bool lww_enable = false;
	bool lww_autorecompute = false;
	bool lww_recompute = true;
	std::vector<float> lww_out;
	int lww_skips = 25;
	float lww_frequency = 0.6f;
	float lww_startphase = 0.0f;
	float lww_feedback = 0.5f;
	int lww_delay = 20;

	lww.SetSkipCount(lww_skips);

	// --- LeWahwah effect

	//for fft/ifft
	bool fft_Enable = false;
	bool fft_Recompute = true;
	bool fft_AutoRecompute = false;
	std::vector<float> mag_sig, output_sig;

	// Final steps for spectrum rendering...
	bool rs_enable = false;

	const unsigned render_width = 2048;
	std::vector<float> spectrum_graph(render_width, 0.0f);

	float x_min = pair_data.m_header.sample_rate / static_cast<float>(windowSize);
	float x_max = pair_data.m_header.sample_rate / 2.0f;
	float x_step = (x_max - x_min) / render_width;
	float x_pos = x_min;

	float spec_min, spec_max;

	for (unsigned i = 0U; i < render_width; ++i)
	{
		spectrum_graph[i] = SignalAnalyst::GetProcessedValue(x_pos, x_pos + x_step);
		x_pos += x_step;
	}

	FindGraphMinMax(spec_min, spec_max, spectrum_graph.data(), spectrum_graph.size());

	// For changing .wav files
	std::string prevTxt = load_file;
	std::string prevInputTxt;

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
			{
				char txtbook[129];
				int txtbooklen = sizeof(txtbook) - 1;

				memset(txtbook, 0, txtbooklen);
				strcpy(txtbook, prevTxt.c_str());

				if (ImGui::InputText("Audio File", txtbook, txtbooklen))
				{
					if (strcmp(txtbook, "") != 0)
					{
						std::string curWavFileStatus = io::wav::globalWavState;

						if (curWavFileStatus == "playing")
						{
							io::wav::PlayWavFile(prevTxt.c_str(), false);
						}

						prevTxt = txtbook;
					}
				}
			}

			{
				char txtbook[129];
				int txtbooklen = sizeof(txtbook) - 1;

				memset(txtbook, 0, txtbooklen);
				strcpy(txtbook, prevInputTxt.c_str());

				if (ImGui::InputText("Input File", txtbook, txtbooklen))
				{
					if (strcmp(txtbook, "") != 0)
						prevInputTxt = txtbook;

				}
			}

			if (ImGui::Button(playbuttonTxt.c_str(), ImVec2(50, 50)))
			{
				if (playbuttonTxt == "play")
				{
					io::wav::PlayWavFile(prevTxt.c_str(), true);
				}
				else
				{
					io::wav::PlayWavFile(prevTxt.c_str(), false);
				}
			}

			if (ImGui::Button("Reload Input file", ImVec2(150, 50)))
			{
				auto tmp_sound_data = io::wav::LoadWAV(prevInputTxt);
				if (tmp_sound_data.second != io::wav::WAV_GOOD)
				{
					prevInputTxt = "Error yo!";
					//return -1;
				}
				else
				{
					sound_data = tmp_sound_data;
					pair_data = io::wav::ToSample(sound_data.first);

					// compressor
					shortvec.clear();
					shortvec.resize(sound_data.first.m_data.size());
					std::copy(sound_data.first.m_data.begin(), sound_data.first.m_data.end(), shortvec.begin());

					// spectrum
					SignalAnalyst::SetDataReference(pair_data.m_samples.data(), pair_data.m_samples.size());
					SignalAnalyst::SetDataRate(pair_data.m_header.sample_rate);

					SignalAnalyst::ComputePlot();

					x_min = pair_data.m_header.sample_rate / static_cast<float>(windowSize);
					x_max = pair_data.m_header.sample_rate / 2.0f;
					x_step = (x_max - x_min) / render_width;
					x_pos = x_min;

					for (unsigned i = 0U; i < render_width; ++i)
					{
						spectrum_graph[i] = SignalAnalyst::GetProcessedValue(x_pos, x_pos + x_step);
						x_pos += x_step;
					}

					FindGraphMinMax(spec_min, spec_max, spectrum_graph.data(), spectrum_graph.size());
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
				if (compressor_Recompute)
				{
					for (unsigned i = 0; i < sinVec.size(); ++i)
					{
						sinVec[i] = Amplitude * sinf(i * 0.5f) + Amplitude*1.5f * sinf(i * 0.2f);
					}

					CompressionPacket pkt2(sinVec);
					result_sinVec = Compressor(pkt2)();

					compressor_Recompute = false;
				}

				for (int i = 0; i < view_buffer_size; ++i)
				{
					view_buffer[i] = pair_data.m_samples[(i + (int)skipval) % pair_data.m_samples.size()];
				}
				std::vector<float> HACK_BUFFER(view_buffer, view_buffer + view_buffer_size);
				CompressionPacket pkt(HACK_BUFFER);

				compressor_result = Compressor(pkt)();

				ImGui::PlotLines("Dynamic Waveform", view_buffer, view_buffer_size, 0, "", -1.0f, 1.0f, ImVec2(0, 100), 4);
				ImGui::PlotLines("Compressed Dynamic Waveform", compressor_result.data(), compressor_result.size(), 0, "input", -1.0f, 1.0f, ImVec2(0, 100));
				ImGui::PlotHistogram("File Value Histogram", shortvec.data(), shortvec.size() / 2, 0, NULL, SHRT_MIN, SHRT_MAX, ImVec2(0, 100));
				ImGui::PlotLines("Uncompressed Waveform", sinVec.data(), sinVec.size(), 0, "input", -1.0f, 1.0f, ImVec2(0, 100));
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
				ImGui::Checkbox("Always Recompute On Change", &cutoff_AutoRecompute);

				ImGui::PlotLines("Entire Waveform", pair_data.m_samples.data(), pair_data.m_samples.size() / 2, 0, "", -1.0f, 1.0f, ImVec2(0, 100), 4);

				if (cutoff_Recompute)
				{
					lowpass = LowPassFilter(pair_data.m_samples, sound_data.first.m_header.sample_rate, cutoffLow);
					highpass = HighPassFilter(pair_data.m_samples, sound_data.first.m_header.sample_rate, cutoffHigh);
					cutoff_Recompute = false;
				}


				{
					bool dummy_bool = false;
					ImGui::PlotLines("Low", lowpass.data(), pair_data.m_samples.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));
					dummy_bool |= ImGui::SliderFloat("Cutoff Low", &cutoffLow, 100, 10000);

					ImGui::PlotLines("High", highpass.data(), highpass.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));
					dummy_bool |= ImGui::SliderFloat("Cutoff High", &cutoffHigh, 100, 10000);

					if (cutoff_AutoRecompute)
						cutoff_Recompute |= dummy_bool;
				}

				if (ImGui::Button("Recompute", ImVec2(100, 30)))
				{
					cutoff_Recompute = true;
				}

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
				dummy_bool |= ImGui::SliderFloat("Bandwidth", &biquad_bandwidth, 0, 50);
				if (biquad_current_filter == Biquad::BiquadFilterTypes::PEAKING_EQ)
					dummy_bool |= ImGui::SliderFloat("Decibel Gain", &biquad_dbGain, -50, 50);
				dummy_bool |= ImGui::RadioButton("LOW PASS", (int*)&biquad_current_filter, 0);
				dummy_bool |= ImGui::RadioButton("HIGH PASS", (int*)&biquad_current_filter, 1);
				dummy_bool |= ImGui::RadioButton("BAND PASS", (int*)&biquad_current_filter, 2);
				dummy_bool |= ImGui::RadioButton("NOTCH", (int*)&biquad_current_filter, 3);
				dummy_bool |= ImGui::RadioButton("ALL PASS", (int*)&biquad_current_filter, 4);
				dummy_bool |= ImGui::RadioButton("PEAKING EQ", (int*)&biquad_current_filter, 5);

				if (biquad_autorecompute)
					biquad_recompute |= dummy_bool;

				if (ImGui::Button("Recompute", ImVec2(100, 30)))
				{
					biquad_recompute = true;
				}

				if (biquad_recompute)
				{
					biquad_out.clear();

					Biquad::BiquadObject obj = Biquad::GetBiquadObject(biquad_current_filter, biquad_dbGain, biquad_centerFreq,
						sound_data.first.m_header.sample_rate, biquad_bandwidth);
					for (const auto& sample : pair_data.m_samples)
					{
						biquad_out.push_back(Biquad::PerformBiquadOnSample(sample, obj));
					}

					biquad_recompute = false;
				}

				ImGui::PlotLines("Biquad Filter Output", biquad_out.data(), biquad_out.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));

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

			//start bb filter
			ImGui::Checkbox("Enable Volume Filters", &bb_enable);
			if (bb_enable)
			{
				if (recompute_bb)
				{
					bb_out.clear();
					for (const auto& sample : pair_data.m_samples)
					{
						bb_out.push_back(BassBoost::BassBoostFunc(sample, bb_selectivity, bb_gain2, bb_ratio));
					}
					recompute_bb = false;
				}


				bool dummy_bool = false;
				//dummy_bool |= ImGui::SliderFloat("Selectivity", &bb_selectivity, 0.0, 200.0);
				dummy_bool |= ImGui::SliderFloat("Boost", &bb_gain2, 0, 10.0f);
				dummy_bool |= ImGui::SliderFloat("Cap", &bb_ratio, 0, 1.0);

				recompute_bb |= dummy_bool;



				ImGui::PlotLines("Volume Filter Output", bb_out.data(), bb_out.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));


				if (ImGui::Button("Save To File", ImVec2(100, 30)))
				{
					io::wav::sample bb_sample = pair_data;
					bb_sample.m_samples = bb_out;
					io::wav::file bb_file = io::wav::ToFile(bb_sample);
					io::wav::SaveWAV("bb_out.wav", bb_file);


				}
			}

			ImGui::Separator();
			//end bb filter

			//start lww
			ImGui::Checkbox("Enable Le Wah Wah", &lww_enable);
			if (lww_enable)
			{
				ImGui::Checkbox("Always Recompute On Change", &lww_autorecompute);

				bool dummy_bool = false;
				dummy_bool |= ImGui::SliderInt("Skip Samples", &lww_skips, 0, 100);
				dummy_bool |= ImGui::SliderFloat("Frequency", &lww_frequency, 0, 5.0f);
				dummy_bool |= ImGui::SliderFloat("Start Phase", &lww_startphase, 0, M_TAU);
				dummy_bool |= ImGui::SliderFloat("Feedback", &lww_feedback, 0, 1.0f);
				dummy_bool |= ImGui::SliderInt("Delay", &lww_delay, 0, 100);

				if (lww_autorecompute)
					lww_recompute |= dummy_bool;

				if (ImGui::Button("Recompute", ImVec2(100, 30)))
				{
					lww_recompute = true;
				}

				if (lww_recompute)
				{
					lww_out.clear();

					lww.SetSkipCount(lww_skips);
					lww.SetSampleRate(pair_data.m_header.sample_rate);
					lww.Init(lww_frequency, lww_startphase, lww_feedback, lww_delay);

					for (const auto& sample : pair_data.m_samples)
					{
						lww_out.push_back(lww.ProcessOnSample(sample));
					}

					lww.Deinit();

					lww_recompute = false;
				}

				ImGui::PlotLines("Le Wah Wah Output", lww_out.data(), lww_out.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));

				if (ImGui::Button("Save To File", ImVec2(100, 30)))
				{
					io::wav::sample lww_sample = pair_data;
					lww_sample.m_samples = lww_out;
					io::wav::file lww_file = io::wav::ToFile(lww_sample);
					io::wav::SaveWAV("lww_out.wav", lww_file);
				}
			}

			ImGui::Separator();
			//end lww

			ImGui::Checkbox("Enable FFT/IFFT", &fft_Enable);
			if (fft_Enable)
			{
				//ImGui::Checkbox("Always Recompute On Change", &fft_AutoRecompute);

				ImGui::PlotLines("Entire Waveform", pair_data.m_samples.data(), pair_data.m_samples.size() / 2, 0, "", -1.0f, 1.0f, ImVec2(0, 100), 4);

				if (fft_Recompute)
				{
					std::vector<std::complex<float> > temp = ourFFT(pair_data.m_samples);
					mag_sig.clear();
					for (const auto& elem : temp)
					{
						mag_sig.push_back(sqrt(elem.real() * elem.real() + elem.imag() * elem.imag()));
					}
					output_sig = ourIFFT(temp);
					fft_Recompute = false;
				}

				{
					bool dummy_bool = false;
					ImGui::PlotHistogram("FFT", mag_sig.data(), mag_sig.size() / 2, 0, NULL, 0.0, 500.0f, ImVec2(0, 100));
					ImGui::PlotLines("IFFT", output_sig.data(), output_sig.size() / 2, 0, "output", -1, 1, ImVec2(0, 100));


					if (fft_AutoRecompute)
						fft_Recompute |= dummy_bool;
				}

				if (ImGui::Button("Recompute", ImVec2(100, 30)))
				{
					fft_Recompute = true;
				}
			}
			ImGui::Separator();

			ImGui::Checkbox("Enable Spectrum View (Raw file)", &rs_enable);
			if (rs_enable)
			{
				ImGui::PlotLines("Spectrum", spectrum_graph.data(), spectrum_graph.size(), 0, "raw data", spec_min, spec_max, ImVec2(0, 100));
			}
			ImGui::Separator();

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
		io::wav::UpdateWavFileStatus(prevTxt);
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
