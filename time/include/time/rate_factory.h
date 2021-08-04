/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#pragma once

#include <cstdint>
#include "./rate.h"
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace time {
    /// @enum SmpteRate
    /// @brief Smpte standard rates
    enum class SmpteRate : int32_t {
      film_24p      = 2400,
      ntsc_24p      = 2397,
      pal_25p       = 2500,
      pal_50pi      = 5000,
      ntsc_30p      = 2997,
      ntsc_60pi     = 5994,
      ntsc_non_drop_30p = 3000,
      ntsc_non_drop_60pi = 6000,
    };
    
    /// @enum AudioSampleRate
    /// @brief Audio standard sampling rates
    enum class AudioSampleRate : int32_t {
      note_a     = 440,
      audio_8k   = 8000,
      audio_16k  = 16000,
      audio_24k  = 24000,
      audio_32k  = 32000,
      audio_44k  = 44100,
      audio_48k  = 48000,
      audio_96k  = 96000,
      audio_192k = 192000
    };

    // ---

    /// @class RateFactory
    /// @brief Utility to build standard rate values
    class RateFactory {
    public:
      RateFactory() = delete;

      /// @brief Create rate object - Smpte standards
      static constexpr inline Rate fromSmpteRate(SmpteRate type) noexcept {
        switch (type) {
          case SmpteRate::film_24p      : return Rate(24, 1u);
          case SmpteRate::ntsc_24p      : return Rate(24000, 1001u);
          case SmpteRate::pal_25p       : return Rate(25, 1u);
          case SmpteRate::pal_50pi      : return Rate(50, 1u);
          case SmpteRate::ntsc_30p      : return Rate(30000, 1001u);
          case SmpteRate::ntsc_60pi     : return Rate(60000, 1001u);
          case SmpteRate::ntsc_non_drop_30p  : return Rate(30, 1u);
          case SmpteRate::ntsc_non_drop_60pi : return Rate(60, 1u);
          default: break;
        }
        return Rate(1, 1u);
      }
      
      /// @brief Create rate object - audio sampling standards
      static constexpr inline Rate fromAudioSampleRate(AudioSampleRate type) noexcept {
        return Rate(static_cast<int32_t>(type), 1u);
      }
    };
    
  }
}
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
