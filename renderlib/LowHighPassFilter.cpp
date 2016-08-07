#include <vector>
#include "LowHighPassFilter.h"

std::vector<float> LowPassFilter(const std::vector<float>& input, float SAMPLE_RATE, float CUTOFF)
{
	float RC = 1.0 / (CUTOFF * 2 * 3.14159265358979323846264338327950288);
	float dt = 1.0 / SAMPLE_RATE;
	float alpha = dt / (RC + dt);

	unsigned numSamples = input.size();

	std::vector<float> output(numSamples);


	output[0] = input[0];
	for (unsigned i = 1; i<numSamples; i++) {
		output[i] = output[i - 1] + (alpha*(input[i] - output[i - 1]));
	}

	return output;
}

std::vector<float> HighPassFilter(const std::vector<float>& input, float SAMPLE_RATE, float CUTOFF)
{
	float RC = 1.0 / (CUTOFF * 2 * 3.14159265358979323846264338327950288);
	float dt = 1.0 / SAMPLE_RATE;
	float alpha = RC / (RC + dt);

	unsigned numSamples = input.size();

	std::vector<float> output(numSamples);

	output[0] = input[0];

	for (unsigned i = 1; i < numSamples; i++) {
		output[i] = alpha * (output[i - 1] + input[i] - input[i - 1]);
	}

	return output;
}