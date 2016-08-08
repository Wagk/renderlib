#include <complex>
#include <vector>

std::vector<std::complex<float> > ourFFT(const std::vector<float>& list);
std::vector<float> ourIFFT(const std::vector<std::complex<float> >& rhs);