#include "fft_impl.h"
#include <cmath>

// Local functions
bool IsPowerOfTwo(int x);
int BitsRequired(int pow2val);
int ReverseBits(int index, int bitCount);

// Class functions
int** SignalAnalyst::st_FFTBitTable = nullptr;
const int SignalAnalyst::st_FFTBitTableLimit = 16;

double st_AngleNumerator = 0.0;

void SignalAnalyst::Init()
{
	st_FFTBitTable = new int* [st_FFTBitTableLimit];

	int length = 2;

	for (int bit = 1; bit <= st_FFTBitTableLimit; ++bit)
	{
		st_FFTBitTable[bit - 1] = new int[length];

		for (int i = 0; i < length; ++i)
			st_FFTBitTable[bit - 1][i] = ReverseBits(i, bit);

		length <<= 1;
	}
}

void SignalAnalyst::Deinit()
{
	if (st_FFTBitTable)
	{
		for (int i = 0; i < st_FFTBitTableLimit; ++i)
			delete[] st_FFTBitTable[i];

		delete[] st_FFTBitTable;
	}
}

void SignalAnalyst::FFT(int n, float* realIn, float* imagIn, float* realOut, float* imagOut)
{
	TemplateFFT(n, -M_TAU, realIn, imagIn, realOut, imagOut);
}

void SignalAnalyst::IFFT(int n, float* realIn, float* imagIn, float* realOut, float* imagOut)
{
	if (TemplateFFT(n, M_TAU, realIn, imagIn, realOut, imagOut))
	{
		float reciprocal = 1.0f / n;

		for (int i = 0; i < n; ++i)
		{
			realOut[i] *= reciprocal;
			imagOut[i] *= reciprocal;
		}
	}
}

void SignalAnalyst::PowerSpectrum(int n, float* in, float* out)
{
	int half = n / 2;
	int quarter = half / 2;
	int i;

	float theta = M_PI / half;

	float *tmpReal = new float[half];
	float *tmpImag = new float[half];
	float *realOut = new float[half];
	float *imagOut = new float[half];

	for (i = 0; i < half; i++) {
		tmpReal[i] = in[2 * i];
		tmpImag[i] = in[2 * i + 1];
	}

	FFT(half, tmpReal, tmpImag, realOut, imagOut);

	float wtemp = float(sin(0.5 * theta));

	float wpr = -2.0 * wtemp * wtemp;
	float wpi = -1.0 * float(sin(theta));
	float wr = 1.0 + wpr;
	float wi = wpi;

	int i3;

	float h1r, h1i, h2r, h2i, rt, it;

	for (i = 1; i < quarter; i++) {

		i3 = half - i;

		h1r = 0.5 * (realOut[i] + realOut[i3]);
		h1i = 0.5 * (imagOut[i] - imagOut[i3]);
		h2r = 0.5 * (imagOut[i] + imagOut[i3]);
		h2i = -0.5 * (realOut[i] - realOut[i3]);

		rt = h1r + wr * h2r - wi * h2i;
		it = h1i + wr * h2i + wi * h2r;

		out[i] = rt * rt + it * it;

		rt = h1r - wr * h2r + wi * h2i;
		it = -h1i + wr * h2i + wi * h2r;

		out[i3] = rt * rt + it * it;

		wr = (wtemp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	}

	rt = (h1r = realOut[0]) + imagOut[0];
	it = h1r - imagOut[0];
	out[0] = rt * rt + it * it;

	rt = realOut[quarter];
	it = imagOut[quarter];
	out[quarter] = rt * rt + it * it;

	delete[] tmpReal;
	delete[] tmpImag;
	delete[] realOut;
	delete[] imagOut;
}

bool SignalAnalyst::TemplateFFT(int n, double fullAngle, float* realIn, float* imagIn, float* realOut, float* imagOut)
{
	if (!IsPowerOfTwo(n))
		return false;

	// Would return false, but not as "user-friendly"
	if (!st_FFTBitTable)
		Init();

	// Set up
	int bitCount = BitsRequired(n);

	for (int i = 0; i < n; ++i)
	{
		int j = FastReverseBit(i, bitCount);
		realOut[j] = realIn[i];
		imagOut[j] = (imagOut == nullptr ? 0.0f : imagIn[i]);
	}

	// Actual FFT
	int blockSize = 2, blockEnd = 1;

	while (blockSize <= n)
	{
		double deltaAngle = fullAngle / blockSize;

		double sm2 = sin(-2 * deltaAngle);
		double sm1 = sin(-deltaAngle);
		double cm2 = cos(-2 * deltaAngle);
		double cm1 = cos(-deltaAngle);
		double w = 2 * cm1;
		double ar0, ar1, ar2, ai0, ai1, ai2;

		for (int i = 0; i < n; i += blockSize)
		{
			ar2 = cm2;
			ar1 = cm1;

			ai2 = sm2;
			ai1 = sm1;

			int j = i;

			for (int l = 0; l < blockEnd; ++l) {
				ar0 = w * ar1 - ar2;
				ar2 = ar1;
				ar1 = ar0;

				ai0 = w * ai1 - ai2;
				ai2 = ai1;
				ai1 = ai0;

				int k = j + blockEnd;
				double tr = ar0 * realOut[k] - ai0 * imagOut[k];
				double ti = ar0 * imagOut[k] + ai0 * realOut[k];

				realOut[k] = realOut[j] - tr;
				imagOut[k] = imagOut[j] - ti;

				realOut[j] += tr;
				imagOut[j] += ti;

				++j;
			}
		}

		blockSize <<= 1;
	}

	return true;
}

int SignalAnalyst::FastReverseBit(int i, int bitCount)
{
	if (bitCount <= st_FFTBitTableLimit)
		return st_FFTBitTable[bitCount - 1][i];

	return ReverseBits(i, bitCount);
}

// Local functions definition
bool IsPowerOfTwo(int x)
{
	if (x < 2)
		return false;

	return (x & (x - 1)) == 0;
}

int BitsRequired(int pow2val)
{
	// Assumes the given value is a power of 2
	static const int UPPER_LIMIT = 8192;
	int i = 0;

	do
	{
		if (pow2val & (1 << i))
			return i;
	} while (i < UPPER_LIMIT);

	// This part should never be reached
	return 0;
}

int ReverseBits(int index, int bitCount)
{
	int i, rev;

	for (i = rev = 0; i < bitCount; ++i)
	{
		rev = (rev << 1) | (index & 1);
		index >>= 1;
	}

	return rev;
}
