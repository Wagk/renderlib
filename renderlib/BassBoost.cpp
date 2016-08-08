#include "BassBoost.h"
#include <algorithm>
#include <iostream>

float BassBoost::BassBoostFunc(float sample, float selectivity, float gain2, float ratio)
{
	static float cap = 0.0f;
	float gain1 = 1.0f / (selectivity + 1.0f);


	//std::cout << selectivity << std::endl;

	cap = (sample + cap*selectivity)*gain1;

	//std::cout << gain1 << std::endl;

	
	float output = std::min<float>(std::max<float>(-1.0f, (sample + cap*ratio)*gain2), 1.0f);
	


	return output;
}
