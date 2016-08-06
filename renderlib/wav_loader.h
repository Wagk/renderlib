#pragma once

#include <fstream>
#include <vector>
#include <string>

namespace io
{
	namespace wav
	{

		//establish the waveheader format
		//http://soundfile.sapp.org/doc/WaveFormat/
		struct header
		{
			//RIFF chunk descriptor
			char chunk_id[4];
			unsigned int chunk_size;
			char format[4];

			//fmt subchunk
			char subchunk1_id[4];
			unsigned int subchunk1_size;
			unsigned short audio_format;
			unsigned short num_channels;
			unsigned int sample_rate;
			unsigned int byte_rate;
			unsigned short block_align; // == NumChannels * BitsPerSample/8
			unsigned short bits_per_sample;

			//data subchunk
			char subchunk2_id[4];
			unsigned subchunk2_size;  //all of this is the normal wave header
		};

		enum status
		{
			WAV_BAD, //!< flag for a corrupt, or non-wav file 
			WAV_MISSING, //!< flag for a file that cannot be opened
			WAV_GOOD //!< flag for a working file
		};

		struct file
		{
			header m_header;
			std::vector<short> m_data;
		};

		std::pair<file, status> LoadWAV(const std::string& filepath);
		void SaveWAV(const std::string& filepath, const file& wav);

	}
} //