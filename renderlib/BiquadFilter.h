#pragma once

namespace Biquad
{
  using sample_type = double;

  struct BiquadObject
  {
    sample_type a0, a1, a2, a3, a4;
    sample_type x1, x2, y1, y2;
  };

  enum BiquadFilterTypes
  {
    LOW_PASS,
    HIGH_PASS,
    BAND_PASS
  };

  sample_type PerformBiquadOnSample(sample_type sample, BiquadObject& obj);

  BiquadObject GetBiquadObject(BiquadFilterTypes type, sample_type freq, 
                                sample_type sampleRate, sample_type bandwidth);
}
