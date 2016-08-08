#include "our_fft.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<std::complex<float> > Redix2( unsigned inc, unsigned N, const std::vector<std::complex<float> >& list)
{
	std::vector<std::complex<float> > temp;
		
	float TWOPIoverN = 2.0 * M_PI / N;
		
	if(list.size() == 2)
	{		
		std::complex<float> W1(1.0,0.0);
				
		temp.push_back(list[0] + list[1] * W1);
		
		std::complex<float> W2(-1.0,0.0);
		
		temp.push_back(list[0] + list[1] * W2);
	}
	else
	{
		std::vector<std::complex<float> > odd;
		std::vector<std::complex<float> > even;
		
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
		
		std::vector<std::complex<float> >  even_list = Redix2(inc * 2,N , even);
		std::vector<std::complex<float> >  odd_list = Redix2(inc * 2,N ,odd);
		
		unsigned factor = list.size() / 2;
				
		for (unsigned i = 0; i < list.size(); ++i)
		{
			unsigned m = i * inc;
			
			std::complex<float> W(cos(TWOPIoverN * m),-sin(TWOPIoverN * m));
			
			temp.push_back(even_list[i% factor] + W * odd_list[i % factor]);
		}
		
	}
	
	return temp;
}

std::vector<std::complex<float> > ourFFT(const std::vector<float>& list)
{
	unsigned N = list.size();
	
	std::vector<std::complex<float> > wave_data; wave_data.reserve(N);
	
	for(unsigned i = 0; i < N; ++i)
	{
		wave_data.push_back(std::complex<float>(list[i],0.0));
	}
		
	while(N & (N - 1))
	{
		wave_data.push_back(std::complex<float>(0.0,0.0));
		
		N = wave_data.size();
	}
	
	return Redix2(1, N , wave_data);
}

std::vector<float> ourIFFT(const std::vector<std::complex<float> >& rhs)
{
	unsigned N = rhs.size();
	
	std::vector<std::complex<float> > temp; temp.reserve(N);
				
	for(unsigned i = 0; i < N; ++i)
	{		
		temp.push_back(std::complex<float>(rhs[i].imag(),rhs[i].real()));
	}
	
	std::vector<std::complex<float> > temp2 = Redix2(1,N,temp);
	
	std::vector<float > result; result.reserve(N);
	
	float scale = 1.0 / N;
	
	for(unsigned i = 0; i < N; ++i)
	{
		result.push_back(scale * temp2[i].imag());
	}

	return result;	
}