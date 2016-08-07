#include <complex>
#include <vector>

std::vector<std::complex<double> > ourFFT(const std::vector<double>& list);
std::vector<double> ourIFFT(const std::vector<std::complex<double> >& rhs);