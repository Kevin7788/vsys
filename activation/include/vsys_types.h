//
//  vsys_types.h
//  vsys
//
//  Created by 薯条 on 2018/1/18.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VSYS_TYPES_H
#define VSYS_TYPES_H

#define CHANNEL_PARAM_POSTION_MASK  0x1
#define CHANNEL_PARAM_DELAY_MASK    0x2

#include <inttypes.h>
#include <cstddef>

enum {
    AUDIO_FORMAT_PCM_16_BIT         = 0x1,
    AUDIO_FORMAT_PCM_24_BIT         = 0x2,
    AUDIO_FORMAT_PCM_32_BIT         = 0x3,
    AUDIO_FORMAT_PCM_32F_BIT        = 0x4,
};

enum {
    AUDIO_SAMPLT_RATE_16K         = 16000,
//    AUDIO_SAMPLT_RATE_48K         = 48000,
//    AUDIO_SAMPLT_RATE_96K         = 96000,
};

typedef struct {
    float x, y, z;
}position_t;

typedef struct {
    position_t position;
    uint32_t id;
    float delay;
}channel_param_t;

typedef struct {
    channel_param_t* channel_params;
    uint32_t sample_rate;
    uint32_t sample_size_bits;
    uint32_t num_mics;
    uint32_t num_channels;
    uint32_t mask;
}activation_param_t;

typedef struct {
    size_t begin;
    size_t end;
    float energy;
} vt_info_t;

typedef struct{
    vt_info_t vt_info;
    
    uint32_t event;
    size_t length;
    
    float energy;
    float threshold_energy;
    float sound_location;
    
    void* data;
}voice_event_t;

enum {
    VOICE_EVENT_LOCAL_WAKE = 100,
    VOICE_EVENT_LOCAL_SLEEP,
    VOICE_EVENT_VT_INFO,
    VOICE_EVENT_HOTWORD,
    VOICE_EVENT_WAKE_NOCMD,
    VOICE_EVENT_VAD_COMING,
    VOICE_EVENT_VAD_START,
    VOICE_EVENT_VAD_DATA,
    VOICE_EVENT_VAD_END,
    VOICE_EVENT_VAD_CANCEL,
};

enum active_action{
    ACTIVATION_SET_STATE_AWAKE = 200,
    ACTIVATION_SET_STATE_SLEEP,
};

//enum active_param{
//
//};

enum word_type{
    VSYS_WORD_AWAKE = 1 ,
    VSYS_WORD_SLEEP ,
    VSYS_WORD_HOTWORD ,
};

typedef struct{
    word_type type;
    char word_utf8[128];
    char pinyin[128];
    char model[128];
    
    float block_avg_score;
    float block_min_score;
    float classify_shield;
    
    bool left_sil_det;
    bool right_sil_det;
    bool remote_check_with_aec;
    bool remote_check_without_aec;
    bool local_classify_check;
}vt_word_t;

#endif /* VSYS_TYPES_H */
