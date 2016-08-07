#include <iostream> // Debug cout
// Just for now.

#include <cmath>
#include <vector>
#include "Compressor.h"
namespace
{
	double PerformLeakyIntegration(double time, int sample_rate)
	{
		// g = e^(-1 / (time * sample_rate))
		double g = (time == 0.0) ? (0.0) : exp(-1.0 / (sample_rate * time));
		return g;
	}

	double lerp(double start, double end, double weight)
	{
		return (1.0 - weight) * start + weight * end;
	}
}


Compressor::Compressor(const CompressionPacket& rhs) : data(rhs) {}
std::vector<float> Compressor::operator()()
{
	std::vector<float> output;
	output.resize(data.samples.size());
	typedef float   stereodata[2];
	double threshold = data.threshold* 0.01;          // threshold to unity (0...1)
	double ratio = data.slope * 0.01;              // slope to unity
	double lookahead_time = data.lookahead_time * 1e-3;                // lookahead time to seconds
	double window_time = data.window_time * 1e-3;               // window time to seconds
	double attack_time = data.attack * 1e-3;               // attack time to seconds
	double release_time = data.release *1e-3;               // release time to seconds
	int sample_rate = data.sample_rate;
								// attack and release "per sample decay"
	double  attack_value = PerformLeakyIntegration(attack_time, sample_rate);//(attack_time == 0.0) ? (0.0) : exp(-1.0 / (sample_rate * attack_time));
	double  release_value = PerformLeakyIntegration(release_time, sample_rate);//(release_time == 0.0) ? (0.0) : exp(-1.0 / (sample_rate * release_time));

	// envelope
	double  envelope = 0.0;

	// sample offset to lookahead wnd start
	int     lookahead_start_offset = (int)(sample_rate * lookahead_time);

	// samples count in lookahead window
	int     window_size = (int)(sample_rate * window_time);

	int N = data.samples.size();
	// for each sample...
	for (int i = 0; i < N; ++i)
	{
		// now compute RMS
		double  sum = 0;

		// for each sample in window
		for (int j = 0; j < window_size; ++j)
		{
			int     lookahead_index = i + j + lookahead_start_offset;
			double  current_sample;

			// if we in bounds of signal?
			// if so, convert to mono
			
			if(lookahead_index < N)
			current_sample = data.samples[lookahead_index];
			else
				current_sample = 0.0;      // if we out of bounds we just get zero in smp

			sum += current_sample * current_sample;  // square em..
		}

		double  root_mean_square = sqrt(sum / window_size);   // root-mean-square

										   // dynamic selection: attack or release?
		double  theta = root_mean_square > envelope ? attack_value : release_value;

		// smoothing with capacitor, envelope extraction...
		// here be aware of pIV denormal numbers glitch
		envelope = lerp(root_mean_square,envelope,theta);

		// the very easy hard knee 1:N compressor
		double  gain = data.gain;
		if (envelope > threshold)
			gain = gain - (envelope - threshold) * ratio;

		// result - two hard kneed compressed channels...
		//float  leftchannel = wav[i][0] * gain;
		//float  rightchannel = wav[i][1] * gain;
		output[i] = data.samples[i] * gain;
	}
	return output;
}