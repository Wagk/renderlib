#pragma once
#include <vector>
struct CompressionPacket
{
	std::vector<float> samples;
	int sample_rate;		//(samples / sec)
	double threshold;		//(percent)
	double slope;			//(slope angle - percents)
	double lookahead_time;	//(sec)
	double window_time;		//(sec)
	double attack;			//(sec)
	double release;			//(sec)
	double gain;

	CompressionPacket(std::vector<float>& copy,
		double _sample_rate = 22500,
		double _threshold = 50.0f,
		double _slope = 50.0f,
		double _window_time = 1.0,
		double _lookahead_time = 3.0,
		double _attack = 0.1,
		double _release = 300
	)
		:
		samples(copy)
		,sample_rate(_sample_rate)	//(samples / sec)
		,threshold(_threshold)		//(percent)
		,slope(_slope)				//(slope angle - percents)
		,lookahead_time(_window_time)//(sec)
		,window_time(_lookahead_time)//(sec)
		,attack(_attack)				//(sec)
		,release(_release)			//(sec)
		,gain(1.0)
	{

	}
};

struct Compressor
{
	CompressionPacket data;
	Compressor(const CompressionPacket& rhs);
	std::vector<float> operator ()();
};
