#pragma once
// FFT code as done in Audacity
#ifndef M_PI
#define	M_PI		3.14159265358979323846  /* pi */
#endif

#ifndef M_TAU
#define	M_TAU		M_PI * 2.0  /* pi */
#endif

class SignalAnalyst
{
public:
	static void Init();
	static void Deinit();

	static void FFT(int n, float* realIn, float* imagIn, float* realOut, float* imagOut);
	static void IFFT(int n, float* realIn, float* imagIn, float* realOut, float* imagOut);

	static void PowerSpectrum(int n, float* in, float* out);

protected:
	static bool TemplateFFT(int n, double fullAngle, float* realIn, float* imagIn, float* realOut, float* imagOut);

private:
	static int** st_FFTBitTable;
	static const int st_FFTBitTableLimit;

	static double st_AngleNumerator;

	static int FastReverseBit(int i, int bitCount);
};
