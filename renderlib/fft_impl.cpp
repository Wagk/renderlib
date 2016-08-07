#include "fft_impl.h"
#include <cmath>

#ifndef max
#define max(a, b) a > b ? a : b
#endif

// Local functions
bool IsPowerOfTwo(int x);
int BitsRequired(int pow2val);
int ReverseBits(int index, int bitCount);

// Class functions
int** SignalAnalyst::st_FFTBitTable = nullptr;
const int SignalAnalyst::st_FFTBitTableLimit = 16;

double SignalAnalyst::st_AngleNumerator = 0.0;

WindowFuncPtr SignalAnalyst::st_WinFunc = SignalAnalyst::WFInitialise;
int SignalAnalyst::st_WinSize = 0;
float* SignalAnalyst::st_Data = nullptr;
int SignalAnalyst::st_DataLength = 0;
float SignalAnalyst::st_Rate = 0.0f;

std::vector<float> SignalAnalyst::st_Processed;
float SignalAnalyst::st_YMin = 0.0f, SignalAnalyst::st_YMax = 0.0f;

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

void SignalAnalyst::RealFFT(int n, float* realIn, float* realOut, float* imagOut)
{
	int half = n / 2;
	float theta = static_cast<float>(M_PI / half);

	float* tempReal = new float[half];
	float* tempImag = new float[half];

	for (int i = 0; i < half; ++i)
	{
		tempReal[i] = realIn[2 * i];
		tempImag[i] = realIn[2 * i + 1];
	}

	FFT(half, tempReal, tempImag, realOut, imagOut);

	float wtemp = sin(0.5f * theta);
	float wpr = -2.0f * wtemp * wtemp;
	float wpi = -1.0f * sin(theta);
	float wr = 1.0f + wpr;
	float wi = wpi;

	for (int i = 1; i < half / 2; i++) {

		int i3 = half - i;

		float h1r = 0.5f * (realOut[i] + realOut[i3]);
		float h1i = 0.5f * (imagOut[i] - imagOut[i3]);
		float h2r = 0.5f * (imagOut[i] + imagOut[i3]);
		float h2i = -0.5f * (realOut[i] - realOut[i3]);

		realOut[i] = h1r + wr * h2r - wi * h2i;
		imagOut[i] = h1i + wr * h2i + wi * h2r;
		realOut[i3] = h1r - wr * h2r + wi * h2i;
		imagOut[i3] = -h1i + wr * h2i + wi * h2r;

		wr = (wtemp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	}

	wtemp = realOut[0];
	realOut[0] += imagOut[0];
	imagOut[0] = wtemp - imagOut[0];

	delete[] tempReal;
	delete[] tempImag;
}

void SignalAnalyst::PowerSpectrum(int n, float* in, float* out)
{
	int half = n / 2;
	int quarter = half / 2;
	int i;

	float theta = static_cast<float>(M_PI) / half;

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

	float wpr = -2.0f * wtemp * wtemp;
	float wpi = -1.0f * float(sin(theta));
	float wr = 1.0f + wpr;
	float wi = wpi;

	int i3;

	float h1r, h1i, h2r, h2i, rt, it;

	for (i = 1; i < quarter; i++) {

		i3 = half - i;

		h1r = 0.5f * (realOut[i] + realOut[i3]);
		h1i = 0.5f * (imagOut[i] - imagOut[i3]);
		h2r = 0.5f * (imagOut[i] + imagOut[i3]);
		h2i = -0.5f * (realOut[i] - realOut[i3]);

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

WindowFuncPtr SignalAnalyst::GetWindowFunction(const WindowTypeFunctions& type)
{
	switch (type)
	{
	case WTF_RECTANGULAR:
		return WFRectangular;

	case WTF_HANNING:
		return WFHanning;

	case WTF_BLACKMAN:
		return WFBlackman;
	}

	return WFInitialise;
}

void SignalAnalyst::SelectWindowFunction(const WindowTypeFunctions& type)
{
	st_WinFunc = GetWindowFunction(type);
}

void SignalAnalyst::SetWindowSize(int size)
{
	st_WinSize = size;
}

void SignalAnalyst::SetDataReference(float* data, int dataLen)
{
	st_Data = data;
	st_DataLength = dataLen;
}

void SignalAnalyst::SetDataRate(float rate)
{
	if (rate > 0.0f)
		st_Rate = rate;
}

float SignalAnalyst::CubicInterpolate(float y0, float y1, float y2, float y3, float x)
{
	float a, b, c, d;

	a = y0 / -6.0f + y1 / 2.0f - y2 / 2.0f + y3 / 6.0f;
	b = y0 - 5.0f * y1 / 2.0f + 2.0f * y2 - y3 / 2.0f;
	c = -11.0f * y0 / 6.0f + 3.0f * y1 - 3.0f * y2 / 2.0f + y3 / 3.0f;
	d = y0;

	float xx = x * x;
	float xxx = xx * x;

	return (a * xxx + b * xx + c * x + d);
}

void SignalAnalyst::ComputePlot()
{
	if (!st_DataLength ||
		st_WinSize < 32 ||
		st_WinSize > 65536 ||
		st_DataLength < st_WinSize)
	{
		return;
	}

	int half = st_WinSize / 2;
	st_Processed.resize(st_WinSize, 0.0f);

	float *in = new float[st_WinSize];
	float *out = new float[st_WinSize];
	float *win = new float[st_WinSize];

	WFInitialise(st_WinSize, win);
	st_WinFunc(st_WinSize, win);

	// Compute window scaling (0 dB for amplitude of 1.0)
	double wss = 0.0;
	for (int i = 0; i < st_WinSize; ++i)
		wss += win[i];

	wss = (wss > 0.0) ? 4.0 / (wss * wss) : 1.0;

	int start = 0;
	int windows = 0;

	while (start + st_WinSize <= st_DataLength)
	{
		for (int i = 0; i < st_WinSize; ++i)
			in[i] = win[i] * st_Data[start + i];

		PowerSpectrum(st_WinSize, in, out);

		for (int i = 0; i < half; ++i)
			st_Processed[i] += out[i];

		start += half;
		++windows;
	}

	st_YMin = 1000000.f;
	st_YMax = -1000000.f;

	const double scale = wss / static_cast<double>(windows);

	for (int i = 0; i < half; ++i)
	{
		st_Processed[i] = 10.0f * log10(st_Processed[i] * static_cast<float>(scale));

		if (st_Processed[i] > st_YMax)
			st_YMax = st_Processed[i];
		else if (st_Processed[i] < st_YMin)
			st_YMin = st_Processed[i];
	}

	delete[] in;
	delete[] out;
	delete[] win;
}

std::vector<float>& SignalAnalyst::GetProcessed()
{
	return st_Processed;
}

float* SignalAnalyst::GetProcessedRaw()
{
	return st_Processed.data();
}

int SignalAnalyst::GetProcessedSize()
{
	return static_cast<int>(st_Processed.size()) / 2;
}

float SignalAnalyst::GetProcessedValue(float freq0, float freq1)
{
	float value = NAN;

	if (st_Rate <= 0.0f || st_Processed.empty())
		return value;

	float bin0 = freq0 * st_WinSize / st_Rate;
	float bin1 = freq1 * st_WinSize / st_Rate;
	float binwidth = bin1 - bin0;

	value = 0.0f;

	if (binwidth < 1.0f)
	{
		float binmid = (bin0 + bin1) / 2.0f;
		int ibin = int(binmid) - 1;
		if (ibin < 1)
			ibin = 1;
		if (ibin >= GetProcessedSize() - 3)
			ibin = max(0, GetProcessedSize() - 4);

		value = CubicInterpolate(st_Processed[ibin],
								 st_Processed[ibin + 1],
								 st_Processed[ibin + 2],
								 st_Processed[ibin + 3],
								 binmid - ibin);

	}
	else
	{
		if (bin0 < 0.0f)
			bin0 = 0.0f;
		if (bin1 >= GetProcessedSize())
			bin1 = GetProcessedSize() - 1.0f;

		if (int(bin1) > int(bin0))
			value += st_Processed[int(bin0)] * (int(bin0) + 1 - bin0);

		bin0 = 1.0f + int(bin0);

		while (bin0 < int(bin1)) {
			value += st_Processed[int(bin0)];
			bin0 += 1.0f;
		}
		value += st_Processed[int(bin1)] * (bin1 - int(bin1));

		value /= binwidth;
	}

	return value;
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

				realOut[k] = static_cast<float>(realOut[j] - tr);
				imagOut[k] = static_cast<float>(imagOut[j] - ti);

				realOut[j] += static_cast<float>(tr);
				imagOut[j] += static_cast<float>(ti);

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

void SignalAnalyst::WFInitialise(int size, float* window)
{
	for (int i = 0; i < size; ++i)
		window[i] = 1.0f;
}

void SignalAnalyst::WFRectangular(int size, float* window)
{
	// Literally do nothing
}

void SignalAnalyst::WFHanning(int size, float* window)
{
	const double multiplier = M_TAU / size;
	static const double coeff0 = 0.5, coeff1 = -0.5;

	for (int i = 0; i < size; ++i)
		window[i] *= static_cast<float>(coeff0 + coeff1 * cos(i * multiplier));
}

void SignalAnalyst::WFBlackman(int size, float* window)
{
	const double multiplier = M_TAU / size;
	const double multiplier2 = 2.0 * multiplier;
	static const double coeff0 = 0.42, coeff1 = -0.5, coeff2 = 0.08;

	for (int i = 0; i < size; ++i)
		window[i] *= static_cast<float>(coeff0 + coeff1 * cos(i * multiplier) + coeff2 * cos(i * multiplier2));
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
