#pragma once

#include <fstream>
#include <vector>
#include <string>

namespace io
{
	namespace wav
	{

		const int WAVE_LABEL_SIZE = 4; //!<the size of a wav header label tag 
		const int WAVE_HEADER_SIZE = 44; //!< the size of a wav file header
		const int AUDIO_FORMAT = 1; //!< the only permissible audio format
		const int MAX_CHANNELS = 2; //!< the maximum permissible number of channels
		const int MAX_SAMPLE = 16; //!< The maximum permissable sample size

		std::string riff_cmp = "RIFF";
		std::string wave_cmp = "WAVE";
		std::string fmt_cmp = "fmt ";
		std::string data_cmp = "data";

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
			unsigned short bits_per_sample;
			unsigned short block_align;

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