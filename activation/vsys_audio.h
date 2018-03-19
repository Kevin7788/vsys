//
//  vsys_audio.h
//  vsys
//
//  Created by 薯条 on 2018/1/20.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VSYS_AUDIO_H
#define VSYS_AUDIO_H

#include "vsys_types.h"

static inline bool audio_is_valid_sample_rate(uint32_t sample_rate){
    switch (sample_rate) {
        case AUDIO_SAMPLT_RATE_16K:
            return true;
    }
    return false;
}

static inline bool audio_is_valid_format(uint32_t format){
    switch (format) {
        case AUDIO_FORMAT_ENCODING_PCM_16BIT:
        case AUDIO_FORMAT_ENCODING_PCM_FLOAT:
            return true;
    }
    return false;
}

static inline uint32_t audio_bytes_per_sample(uint32_t format){
    switch (format) {
        case AUDIO_FORMAT_ENCODING_PCM_16BIT:
            return 2;
        case AUDIO_FORMAT_ENCODING_PCM_FLOAT:
            return 4;
    }
    return 0;
}

#endif /* VSYS_AUDIO_H */
