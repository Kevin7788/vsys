//
//  audio_converter.h
//  vsys
//
//  Created by 薯条 on 2018/1/21.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef AUDIO_CONVERTER_H
#define AUDIO_CONVERTER_H

#include <inttypes.h>

#include "buf_manager.h"

namespace vsys {
    
class AudioConverter{
public:
    AudioConverter(uint32_t format, uint32_t num_channels, uint32_t* mic_ids)
    :format(format), num_channels(num_channels),mic_ids(mic_ids), buff(NULL), buff_len(0){}
    
    ~AudioConverter(){release_buffer2((void **)buff);}
    
    int32_t convert(void *input, uint32_t input_size, float **&output, uint32_t &output_size);
    
    void check_buffer(uint32_t length);
    
private:
    uint32_t* mic_ids;
    uint32_t format;
    uint32_t num_channels;
    
    float** buff;
    uint32_t buff_len;
};
    
}

#endif /* AUDIO_CONVERTER_H */
