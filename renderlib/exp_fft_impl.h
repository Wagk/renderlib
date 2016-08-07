#pragma once
// Note: This file is literally RealFFTf.h/.cpp by Audacity!!

#define fft_type float
struct FFTParam {
	int *BitReversed;
	fft_type *SinTable;
	int Points;
};
typedef FFTParam * HFFT;

namespace Experimental
{
	HFFT InitializeFFT(int);
	void EndFFT(HFFT);
	HFFT GetFFT(int);
	void ReleaseFFT(HFFT);
	void CleanupFFT();
	void RealFFT(fft_type *, HFFT);
	void InverseRealFFT(fft_type *, HFFT);
	void ReorderToTime(HFFT hFFT, fft_type *buffer, fft_type *TimeOut);
	void ReorderToFreq(HFFT hFFT, fft_type *buffer, fft_type *RealOut, fft_type *ImagOut);
}
