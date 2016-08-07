#pragma once

std::vector<float> LowPassFilter(const std::vector<float>& input, float SAMPLE_RATE, float CUTOFF);

std::vector<float> HighPassFilter(const std::vector<float>& input, float SAMPLE_RATE, float CUTOFF);
