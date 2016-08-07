#include "wav_loader.h"

#include <Windows.h>
#include <mmsystem.h>

namespace io
{
	namespace wav
	{
		const int WAVE_LABEL_SIZE = 4; //!<the size of a wav header label tag 
		const int WAVE_HEADER_SIZE = 44; //!< the size of a wav file header
		const int AUDIO_FORMAT = 1; //!< the only permissible audio format
		const int MAX_CHANNELS = 2; //!< the maximum permissible number of channels
		const int MAX_SAMPLE = 16; //!< The maximum permissable sample size

								   /*changes here*/
		std::string globalWavState = ""; //!<lame global to track status of state of one file.
										 /*to here*/

		namespace
		{
			int file_size(const std::string& file)
			{
				std::ifstream obj(file, std::ifstream::binary | std::ifstream::ate);

				if (obj.bad()) //if file is just bad
				{
					return -1;
				}

				std::streampos num = obj.tellg();
				obj.close();

				return static_cast<int>(num);
			}

		}

		std::pair<file, status> LoadWAV(const std::string & filepath)
		{
			using result = std::pair<file, status>;

			int filesize = file_size(filepath);

			if (filesize == -1) return result(file(), WAV_MISSING);
			if (filesize < sizeof header) return result(file(), WAV_BAD);

			std::ifstream wav(filepath, std::ifstream::binary);

			header wav_header;

			wav.read(reinterpret_cast<char*>(&wav_header), sizeof header);

			if (
				std::memcmp(wav_header.chunk_id, "RIFF", 4) ||
				std::memcmp(wav_header.format, "WAVE", 4) ||
				std::memcmp(wav_header.subchunk1_id, "fmt ", 4) ||
				std::memcmp(wav_header.subchunk2_id, "data", 4) ||
				wav_header.audio_format != AUDIO_FORMAT ||
				wav_header.num_channels > MAX_CHANNELS ||
				wav_header.bits_per_sample > MAX_SAMPLE ||
				wav_header.subchunk2_size != wav_header.chunk_size - sizeof(header) + sizeof(unsigned int) + 4
				)
			{
				return result(file(), WAV_BAD);
			}

			/*if (std::memcmp(riff_label, riff_cmp, sizeof(riff_label)) ||
				std::memcmp(file_tag, wave_cmp, sizeof(file_tag)) ||
				std::memcmp(fmt_label, fmt_cmp, sizeof(fmt_label)) ||
				std::memcmp(data_label, data_cmp, sizeof(data_label)) ||
				audio_format != AUDIO_FORMAT ||
				channel_count > MAX_CHANNELS ||
				bits_per_sample != MAX_SAMPLE ||
				data_size != riff_size - WAVE_HEADER_SIZE + unintsz
				+ charsz * WAVE_LABEL_SIZE)
			{
				infile.close();
				return wavefile::WAV_BAD;
			}*/

			std::vector<short> wav_data(wav_header.subchunk2_size);

			for (size_t i = 0; i < wav_header.subchunk2_size; ++i)
			{
				wav.read(reinterpret_cast<char*>(&wav_data[i]), sizeof(short));
			}

			file wav_file;
			wav_file.m_header = wav_header;
			wav_file.m_data = wav_data;

			return result(wav_file, WAV_GOOD);
		}

		void SaveWAV(const std::string& path, const file& wav)
		{
			std::ofstream ofs(path, std::ofstream::binary);

			ofs.write(reinterpret_cast<const char*>(&wav.m_header), sizeof header);
			ofs.write(reinterpret_cast<const char*>(wav.m_data.data()), wav.m_header.subchunk2_size);
		}

		std::pair<std::vector<float>, std::vector<float>> SplitChannels(const file & file)
		{
			union splitter
			{
				char pair[2] = { 0 };
				short val;
			};

			std::vector<float> left; left.reserve(file.m_data.size());
			std::vector<float> right; right.reserve(left.capacity());

			for (short piece : file.m_data)
			{
				splitter split;

				split.val = piece;

				left.push_back(split.pair[0]);
				right.push_back(split.pair[1]);
			}

			return std::make_pair(left, right);
		}

		std::vector<float> FloatChannel(const file & file)
		{
			union splitter
			{
				char pair[2] = { 0 };
				short val;
			};

			std::vector<float> data;
			data.reserve(file.m_data.size());

			for (short elem : file.m_data)
			{
				splitter split;

				split.val = elem;

				data.push_back(split.val);
				//data.push_back(split.pair[1]);
			}

			return data;
		}

		/*changes here*/
		void PlayWavFile(const std::string &filepath, const bool &toplay)
		{
			if (toplay)
			{
				if (globalWavState == "paused")
					mciSendString(std::string("resume \"" + filepath + "\"").c_str(), NULL, 0, NULL);
				else if (globalWavState == "stopped" || globalWavState == "")
					mciSendString(std::string("play \"" + filepath + "\"").c_str(), NULL, 0, NULL);
			}
			else
			{
				if (globalWavState == "playing")
					mciSendString(std::string("pause \"" + filepath + "\"").c_str(), NULL, 0, NULL);
			}


		}
		void UpdateWavFileStatus(const std::string &filepath)
		{
			char mcidata[129];
			int mcidatalen = sizeof(mcidata) - 1;

			mciSendString(std::string("status " + filepath + " mode").c_str(), mcidata, mcidatalen, NULL);

			globalWavState = mcidata;

		}

		/*to here*/

	}
}