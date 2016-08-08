#include "LeWahWah.h"

#define LEWAHWAH_DEFAULT_RATE 44100

#define clamp(a, b, c) a < b ? b : (a > c ? c : a)

LeWahWah::LeWahWah()
	: m_IsInit(false)
	, m_SampleRate(LEWAHWAH_DEFAULT_RATE)
	, m_LFOSkipSamples(25U)
{
}

void LeWahWah::Init(float freq, float startphase, float fb, int delay)
{
	if (m_IsInit)
		return;

	m_Frequency = freq;
	m_StartPhase = startphase;
	m_Feedback = clamp(fb / 4 + 0.74f, 0.0f, 1.0f);
	m_Delay = static_cast<int>(delay / static_cast<float>(LEWAHWAH_DEFAULT_RATE) * m_SampleRate);

	if (m_Delay < 1) m_Delay = 1;

	m_DelayBuffer.resize(m_Delay, std::complex<double>(0.0, 0.0));
	m_LFOSkipPhase = freq * 2 * 3.141592653589f / m_SampleRate;
	m_LFOSkipCounter = 0U;
	m_LFONumber = std::complex<double>(0.0, 0.0);
	m_DelayCounter = 0U;

	m_IsInit = true;
}

void LeWahWah::Deinit()
{
	if (!m_IsInit)
		return;

	m_IsInit = false;

	m_DelayBuffer.clear();
}

LeWahWah::sample_type LeWahWah::ProcessOnSample(sample_type sample)
{
	sample_type result = sample;

	if (!m_IsInit)
		return result;

	std::complex<double> outc;

	if (m_LFOSkipCounter++ % m_LFOSkipSamples == 0)
	{
		double lfo = (1 + cos(m_LFOSkipCounter * m_LFOSkipPhase + m_StartPhase));
		m_LFONumber = std::complex<double>(cos(lfo) * m_Feedback, sin(lfo) * m_Feedback);
	};

	outc = m_LFONumber * m_DelayBuffer[m_DelayCounter] + (1.0f - m_Feedback) * sample;
	m_DelayBuffer[m_DelayCounter] = outc;

	if ((++m_DelayCounter) >= m_Delay)
		m_DelayCounter = 0U;

	result = clamp(real(outc) * 3, -1.0, 1.0);

	return result;
}

bool LeWahWah::IsInit() const
{
	return m_IsInit;
}

void LeWahWah::SetSampleRate(unsigned rate)
{
	m_SampleRate = rate;
}

unsigned LeWahWah::GetSampleRate() const
{
	return m_SampleRate;
}

void LeWahWah::SetSkipCount(unsigned skipCount)
{
	m_LFOSkipSamples = skipCount > 0U ? skipCount : 1U;
}

unsigned LeWahWah::GetSkipCount() const
{
	return m_LFOSkipSamples;
}
