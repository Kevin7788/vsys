//
//  main.cpp
//  vsys
//
//  Created by 薯条 on 2017/12/24.
//  Copyright © 2017年 薯条. All rights reserved.
//
#define FRAME_SIZE 160

#include <iostream>
#include <fstream>
#include <thread>

#include "vsys_activation.h"

std::ifstream pcm_in("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.wav.f32.pcm", std::ios::in | std::ios::binary);

char buff[8192];

float mic_pos[] = {
    0.0425000000, 0.0000000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000,
    0.0000000000, 0.0425000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000,
    0.0425000000, 0.0000000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000,
    0.0000000000, 0.0425000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000
};

void test(){
    channel_param_t channel_param[8];
    for (uint32_t i = 0; i < 8; i++) {
        channel_param[i].position.x = mic_pos[i * 3 + 0];
        channel_param[i].position.y = mic_pos[i * 3 + 1];
        channel_param[i].position.z = mic_pos[i * 3 + 2];
        channel_param[i].id = i;
//        channel_param[0].delay = 0.0000000000;
    }
    
    activation_param_t param;
    param.channel_params = channel_param;
    param.sample_rate = AUDIO_SAMPLT_RATE_16K;
    param.sample_size_bits = AUDIO_FORMAT_PCM_32F_BIT;
    param.num_mics = 8;
    param.num_channels = 8;
    param.mask |= CHANNEL_PARAM_POSTION_MASK;
//    param.mask |= CHANNEL_PARAM_DELAY_MASK;
    
    bool loop = true;
    srand(time(nullptr));
    VsysActivationInst handle =  VsysActivation_Create(&param, "/Users/daixiang/external/thirdlib", true);
    
    std::thread thread([&]{
        while (loop) {
            VsysActivation_Control(handle, (rand() % 2) ? ACTIVATION_SET_STATE_AWAKE : ACTIVATION_SET_STATE_SLEEP);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000 + 1));
        }
    });
    
    for (uint32_t i = 0; i < FRAME_SIZE * 10; i++) {
        while(pcm_in.good()){
            pcm_in.read(buff, FRAME_SIZE * 8 * sizeof(float));
            VsysActivation_Process(handle, (uint8_t *)buff, FRAME_SIZE * 8 * sizeof(float));
        }
        printf("#############################################{%d/%d}#############################################\n", i, FRAME_SIZE * 10);
        pcm_in.clear();
        pcm_in.seekg(0, std::ios::beg);
    }
    loop = false;
    thread.join();
    VsysActivation_Free(handle);
}

int main(int argc, const char * argv[]) {
    time_t t1, t2;
    time(&t1);
    test();
    time(&t2);
    printf("已运行%d秒\n",t2-t1);
    return 0;
}
