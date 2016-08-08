#pragma once
// This file is titled as such as I have no idea what's going on
// Source link: http://musicdsp.org/files/alienwah.c
#include <vector>
#include <complex>

class LeWahWah
{
public:
	using sample_type = double;

	LeWahWah();

	void Init(float freq, float startphase, float fb, int delay);
	void Deinit();

	sample_type ProcessOnSample(sample_type sample);

	bool IsInit() const;

	void SetSampleRate(int rate);
	int GetSampleRate() const;

	void SetSkipCount(unsigned skipCount);
	unsigned GetSkipCount() const;

private:
	bool m_IsInit;

	int m_SampleRate;

	unsigned m_LFOSkipSamples;

	float m_Frequency;
	float m_StartPhase;
	float m_Feedback;
	unsigned m_Delay;

	// Internal only values, set on init
	std::vector<std::complex<double>> m_DelayBuffer;
	float m_LFOSkipPhase;
	unsigned m_LFOSkipCounter;
	std::complex<double> m_LFONumber;
	unsigned m_DelayCounter;
};
