#include "our_fft.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<std::complex<double> > Redix2( unsigned inc, unsigned N, const std::vector<std::complex<double> >& list)
{
	std::vector<std::complex<double> > temp;
		
	double TWOPIoverN = 2.0 * M_PI / N;
		
	if(list.size() == 2)
	{		
		std::complex<double> W1(1.0,0.0);
				
		temp.push_back(list[0] + list[1] * W1);
		
		std::complex<double> W2(-1.0,0.0);
		
		temp.push_back(list[0] + list[1] * W2);
	}
	else
	{
		std::vector<std::complex<double> > odd;
		std::vector<std::complex<double> > even;
		
		for (unsigned i = 0; i < list.size(); ++i)
		{
			if(i % 2)
			{
				odd.push_back(list[i]);								
			}
			else
			{
				even.push_back(list[i]);
			}
		}
		
		std::vector<std::complex<double> >  even_list = Redix2(inc * 2,N , even);
		std::vector<std::complex<double> >  odd_list = Redix2(inc * 2,N ,odd);
		
		unsigned factor = list.size() / 2;
				
		for (unsigned i = 0; i < list.size(); ++i)
		{
			unsigned m = i * inc;
			
			std::complex<double> W(cos(TWOPIoverN * m),-sin(TWOPIoverN * m));
			
			temp.push_back(even_list[i% factor] + W * odd_list[i % factor]);
		}
		
	}
	
	return temp;
}

std::vector<std::complex<double> > ourFFT(const std::vector<double>& list)
{
	unsigned N = list.size();
	
	std::vector<std::complex<double> > wave_data; wave_data.reserve(N);
	
	for(unsigned i = 0; i < N; ++i)
	{
		wave_data.push_back(std::complex<double>(list[i],0.0));
	}
		
	while(N & (N - 1))
	{
		wave_data.push_back(std::complex<double>(0.0,0.0));
		
		N = wave_data.size();
	}
	
	return Redix2(1, N , wave_data);
}

std::vector<double> ourIFFT(const std::vector<std::complex<double> >& rhs)
{
	unsigned N = rhs.size();
	
	std::vector<std::complex<double> > temp; temp.reserve(N);
				
	for(unsigned i = 0; i < N; ++i)
	{		
		temp.push_back(std::complex<double>(rhs[i].imag(),rhs[i].real()));
	}
	
	std::vector<std::complex<double> > temp2 = Redix2(1,N,temp);
	
	std::vector<double > result; result.reserve(N);
	
	double scale = 1.0 / N;
	
	for(unsigned i = 0; i < N; ++i)
	{
		result.push_back(scale * temp2[i].imag());
	}

	return result;	
}