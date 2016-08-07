#pragma once
// FFT code as done in Audacity
#include <vector>

#ifndef M_PI
#define	M_PI		3.14159265358979323846  /* pi */
#endif

#ifndef M_TAU
#define	M_TAU		M_PI * 2.0
#endif

typedef void (*WindowFuncPtr)(int, float*);

enum WindowTypeFunctions
{
	WTF_RECTANGULAR,
	WTF_HANNING,
	WTF_BLACKMAN,
	WTF_COUNT
};

class SignalAnalyst
{
public:
	static void Init();
	static void Deinit();

	static void FFT(int n, float* realIn, float* imagIn, float* realOut, float* imagOut);
	static void IFFT(int n, float* realIn, float* imagIn, float* realOut, float* imagOut);

	static void RealFFT(int n, float* realIn, float* realOut, float* imagOut);
	static void ERealFFT(int n, float* realIn, float* realOut, float* imagOut);
	static void ERealIFFT(int n, float* realIn, float* imagIn, float* realOut);

	static void PowerSpectrum(int n, float* in, float* out);

	static WindowFuncPtr GetWindowFunction(const WindowTypeFunctions& type);

	// Analysis related
	static void SelectWindowFunction(const WindowTypeFunctions& type);
	static void SetWindowSize(int size);
	static void SetDataReference(float* data, int dataLen);
	static void SetDataRate(float rate);

	static float CubicInterpolate(float y0, float y1, float y2, float y3, float x);
	static void ComputePlot();

	static std::vector<float>& GetProcessed();
	static float* GetProcessedRaw();
	static int GetProcessedSize();
	static float GetProcessedValue(float freq0, float freq1);

protected:
	static bool TemplateFFT(int n, double fullAngle, float* realIn, float* imagIn, float* realOut, float* imagOut);

private:
	// FFT related
	static int** st_FFTBitTable;
	static const int st_FFTBitTableLimit;

	static double st_AngleNumerator;

	static int FastReverseBit(int i, int bitCount);

	static void WFInitialise(int size, float* window);
	static void WFRectangular(int size, float* window);
	static void WFHanning(int size, float* window);
	static void WFBlackman(int size, float* window);

	// Analysis related
	static WindowFuncPtr st_WinFunc;
	static int st_WinSize;
	static float* st_Data;
	static int st_DataLength;
	static float st_Rate;

	static std::vector<float> st_Processed;
	static float st_YMin, st_YMax;
};
