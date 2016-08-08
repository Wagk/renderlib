#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>

#include "BiquadFilter.h"

namespace Biquad
{
	sample_type PerformBiquadOnSample(sample_type sample, BiquadObject& obj)
	{
		sample_type result;

		result = obj.a0 * sample + obj.a1 * obj.x1 + obj.a2 * obj.x2
			- obj.a3 * obj.y1 - obj.a4 * obj.y2;

		obj.x2 = obj.x1;
		obj.x1 = sample;

		obj.y2 = obj.y1;
		obj.y1 = result;

		return result;
	}

	BiquadObject GetBiquadObject(BiquadFilterTypes type, sample_type dbGain, sample_type freq,
		sample_type sampleRate, sample_type bandwidth)
	{
		BiquadObject obj;
		sample_type omega = 2 * M_PI * freq / sampleRate;
		sample_type cos_omega = cos(omega);
		sample_type sin_omega = sin(omega);
		sample_type alpha = sin_omega * sinh(M_LN2 / 2 * bandwidth * omega / sin_omega);

		// For peaking and shelving EQ filters only
		sample_type A = pow(10, dbGain / 40);

		sample_type a0, a1, a2, b0, b1, b2;

		switch (type)
		{
		case BiquadFilterTypes::LOW_PASS:
			b0 = (1 - cos_omega) / 2;
			b1 = 1 - cos_omega;
			b2 = b0;
			a0 = 1 + alpha;
			a1 = -2 * cos_omega;
			a2 = 1 - alpha;
			break;

		case BiquadFilterTypes::HIGH_PASS:
			b0 = (1 + cos_omega) / 2;
			b1 = -1 - cos_omega;
			b2 = b0;
			a0 = 1 + alpha;
			a1 = -2 * cos_omega;
			a2 = 1 - alpha;
			break;

		case BiquadFilterTypes::BAND_PASS:
			b0 = alpha;
			b1 = 0;
			b2 = -alpha;
			a0 = 1 + alpha;
			a1 = -2 * cos_omega;
			a2 = 1 - alpha;
			break;

		case BiquadFilterTypes::NOTCH:
			b0 = 1;
			b1 = -2 * cos_omega;
			b2 = 1;
			a0 = 1 + alpha;
			a1 = -2 * cos_omega;
			a2 = 1 - alpha;
			break;

		case BiquadFilterTypes::ALL_PASS:
			b0 = 1 - alpha;
			b1 = -2 * cos_omega;
			b2 = 1 + alpha;
			a0 = 1 + alpha;
			a1 = -2 * cos_omega;
			a2 = 1 - alpha;
			break;

		case BiquadFilterTypes::PEAKING_EQ:
			b0 = 1 + alpha * A;
			b1 = -2 * cos_omega;
			b2 = 1 - alpha * A;
			a0 = 1 + alpha/A;
			a1 = -2 * cos_omega;
			a2 = 1 - alpha/A;
			break;

		default:
			std::cout << "Should not be happening." << std::endl;
			//return BiquadObject{ 1, 0, 0, 0, 0, 0, 0, 0, 0 };
			break;
		}

		obj.a0 = b0 / a0;
		obj.a1 = b1 / a0;
		obj.a2 = b2 / a0;
		obj.a3 = a1 / a0;
		obj.a4 = a2 / a0;

		obj.x1 = obj.x2 = obj.y1 = obj.y2 = 0;

		return obj;
	}
}