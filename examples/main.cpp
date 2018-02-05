//
//  main.cpp
//  vsys
//
//  Created by 薯条 on 2017/12/24.
//  Copyright © 2017年 薯条. All rights reserved.
//

#define CHANNEL_NUM 2
#define SPEAKER_NUM 2

#define FRAME_SIZE 160
#define FRAME_SIZE_SPEEX 256

#define SAMPLE_RATE 16000
#define SPEEX_AEC_TAIL 1024

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <semaphore.h>

#include "vsys_activation.h"
#include "audio_processing.h"
#include "speex_preprocess.h"
#include "speex_echo.h"

std::ifstream pcm_in("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.wav.f32.pcm", std::ios::in | std::ios::binary);
//std::ifstream pcm_in("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.wav.f32.pcm", std::ios::in | std::ios::binary);
std::ifstream pcm_out("/Users/daixiang/Desktop/vsys/data/sounds/pcm_out.pcm", std::ios::out | std::ios::binary);

char buff[8192];
float input[8192];
float output[FRAME_SIZE_SPEEX];

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

void test_activation(){
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
    
    for (uint32_t i = 0; i < FRAME_SIZE * 100; i++) {
        while(pcm_in.good()){
            pcm_in.read(buff, FRAME_SIZE * 8 * sizeof(float));
            VsysActivation_Process(handle, (uint8_t *)buff, FRAME_SIZE * 8 * sizeof(float));
        }
        printf("#############################################{%d/%d}#############################################\n", i, FRAME_SIZE * 100);
        pcm_in.clear();
        pcm_in.seekg(0, std::ios::beg);
    }
    loop = false;
    thread.join();
    VsysActivation_Free(handle);
}

void test_aec(){
    SpeexPreprocessState** preprocess_states = new SpeexPreprocessState*[CHANNEL_NUM];
    SpeexEchoState** echo_states = new SpeexEchoState*[SPEAKER_NUM];
    
    int sample_rate = SAMPLE_RATE;
    for(uint32_t i = 0; i < CHANNEL_NUM; i++){
        preprocess_states[i] = speex_preprocess_state_init(FRAME_SIZE_SPEEX, SAMPLE_RATE);
        echo_states[i] = speex_echo_state_init_mc(FRAME_SIZE_SPEEX, SPEEX_AEC_TAIL, 1, SPEAKER_NUM);
        
        speex_echo_ctl(echo_states[i], SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
        speex_preprocess_ctl(preprocess_states[i], SPEEX_PREPROCESS_SET_ECHO_STATE, echo_states[i]);
    }

    std::list<uint32_t> tasks;
    
    sem_t resume, pause;
    sem_t *resume_ref, *pause_ref;
#if defined(__APPLE__) || defined(__MACH__)
    sem_unlink("sem_speex_resume");
    sem_unlink("sem_speex_pause");
    resume_ref = sem_open("sem_speex_resume", O_CREAT | O_EXCL, 0644, 0);
    pause_ref = sem_open("sem_speex_pause", O_CREAT | O_EXCL, 0644, 0);
#else
    sem_init(&resume, 0, 0);
    sem_init(&pause, 0, 0);
    resume_ref = &resume;
    pause_ref = &pause;
#endif
    
    bool loop = true;
    auto thread_loop  = [&]{
        
        std::mutex mutex;
        std::unique_lock<decltype(mutex)> locker(mutex, std::defer_lock);
        while (loop) {
            sem_wait(resume_ref);
            
            locker.lock();
            uint32_t task_id = *tasks.begin();
            tasks.pop_front();
            locker.unlock();
            
            speex_echo_cancellation(echo_states[task_id],
                                    input + task_id * FRAME_SIZE_SPEEX,         //mic
                                    input + CHANNEL_NUM * FRAME_SIZE_SPEEX,     //speaker
                                    output + task_id * FRAME_SIZE_SPEEX);
            
            speex_preprocess_run(preprocess_states[task_id], output + task_id * FRAME_SIZE_SPEEX);
            
            sem_post(pause_ref);
        }
    };
    
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < 2; i++) {
        threads.emplace_back(thread_loop);
    }
    
    uint32_t num_channels = CHANNEL_NUM + SPEAKER_NUM;
    uint32_t total = num_channels * FRAME_SIZE_SPEEX;
    while (pcm_in.good()) {
        pcm_in.read(buff, total * sizeof(short));
        for (uint32_t i = 0; i < num_channels; i++) {
            for (uint32_t j = 0; j < total; j++) {
                input[i * FRAME_SIZE_SPEEX + j] = ((short *)buff)[j * num_channels + i];
            }
        }
        for (uint32_t i = FRAME_SIZE_SPEEX; i < num_channels; i++) {
            for (uint32_t j = 0; j < CHANNEL_NUM; j++) {
                input[i * SPEAKER_NUM +j] = input[j * FRAME_SIZE_SPEEX + i];
            }
        }
        for (uint32_t i = 0; i < CHANNEL_NUM; i++) {
            tasks.push_back(i);
            sem_post(resume_ref);
        }
        for (uint32_t i = 0; i < CHANNEL_NUM; i++) {
            sem_wait(pause_ref);
        }
    }

    loop = false;
    for (uint32_t i = 0; i < CHANNEL_NUM; i++) {
        threads[i].join();
    }
    for (uint32_t i = 0; i < CHANNEL_NUM; i++) {
        speex_echo_state_destroy(echo_states[i]);
        speex_preprocess_state_destroy(preprocess_states[i]);
    }
    
#if defined(__APPLE__) || defined(__MACH__)
    sem_close(resume_ref);
    sem_close(pause_ref);
    sem_unlink("sem_speex_resume");
    sem_unlink("sem_speex_pause");
#else
    sem_destroy(resume_ref);
    sem_destroy(pause_ref);
#endif
}

int main(int argc, const char * argv[]) {
    time_t t1, t2;
    time(&t1);
    
    test_activation();
    
    time(&t2);
    printf("已运行%d秒\n",t2-t1);
    return 0;
}