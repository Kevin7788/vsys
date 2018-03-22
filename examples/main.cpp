//
//  main.cpp
//  vsys
//
//  Created by 薯条 on 2017/12/24.
//  Copyright © 2017年 薯条. All rights reserved.
//
#define LOG_TAG "examples"

#define MIC_CHANNEL 2
#define SPEAKER_CHANNEL 2

#define FRAME_SIZE 160
#define FRAME_SIZE_SPEEX 256

#define SAMPLE_RATE 48000
#define SPEEX_AEC_TAIL 1024

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <semaphore.h>

#include "debug.h"
#include "audio_processing.h"
#include "vsys_activation.h"
#include "speex_preprocess.h"
#include "speex_echo.h"

std::ifstream pcm_in("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.wav.f32.pcm", std::ios::in | std::ios::binary);
//std::ifstream pcm_in("/Users/daixiang/Desktop/vsys/data/sounds/aec.16bit.pcm", std::ios::in | std::ios::binary);
std::ofstream pcm_out("/Users/daixiang/Desktop/vsys/data/sounds/pcm_out.pcm", std::ios::out | std::ios::binary);

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
    mic_param_t channel_param[8];
    for (uint32_t i = 0; i < 8; i++) {
        channel_param[i].position.x = mic_pos[i * 3 + 0];
        channel_param[i].position.y = mic_pos[i * 3 + 1];
        channel_param[i].position.z = mic_pos[i * 3 + 2];
        channel_param[i].id = i;
//        channel_param[0].delay = 0.0000000000;
    }
    
    activation_param_t param;
    param.mic_params = channel_param;
    param.sample_rate = AUDIO_SAMPLT_RATE_16K;
    param.sample_size_bits = AUDIO_FORMAT_ENCODING_PCM_FLOAT;
    param.num_mics = 8;
    param.num_channels = 8;
    param.mask |= MIC_PARAM_POSTION_MASK;
//    param.mask |= MIC_PARAM_DELAY_MASK;
    
    bool loop = true;
    srand(time(nullptr));
    VsysActivationInst handle =  VsysActivation_Create(&param, "/Users/daixiang/external/thirdlib", true);
    
    vt_word_t vt_word;
    memset(&vt_word, 0, sizeof(vt_word_t));
    vt_word.type = VSYS_WORD_AWAKE;
    strcpy(vt_word.phone, "r|l|r_B|l_B|# w o4|o4_E|## q|q_B|# i2|i2_E|##");
    strcpy(vt_word.word_utf8, "若琪");
    strcpy(vt_word.nnet_path, "/Users/daixiang/external/thirdlib/workdir_cn/final.ruoqi.mod");
    vt_word.mask |= VT_WORD_USE_OUTSIDE_PHONE_MASK
                | VT_WORD_LOCAL_CLASSIFY_CHECK_MASK;
    VsysActivation_AddVtWord(handle, &vt_word);
    
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

void test_audio_processing(){
    SpeexPreprocessState** preprocess_states = new SpeexPreprocessState*[MIC_CHANNEL];
    SpeexEchoState** echo_states = new SpeexEchoState*[SPEAKER_CHANNEL];
    
    int sample_rate = SAMPLE_RATE;
    for(uint32_t i = 0; i < MIC_CHANNEL; i++){
        preprocess_states[i] = speex_preprocess_state_init(FRAME_SIZE_SPEEX, SAMPLE_RATE);
        echo_states[i] = speex_echo_state_init_mc(FRAME_SIZE_SPEEX, SPEEX_AEC_TAIL, 1, SPEAKER_CHANNEL);
        
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
    
    bool thread_exit = false;
    std::mutex task_mutex;
    auto thread_loop  = [&]{
        std::unique_lock<decltype(task_mutex)> locker(task_mutex, std::defer_lock);
        while (true) {
            sem_wait(resume_ref);
            if(thread_exit) break;

            locker.lock();
            uint32_t task_id = *tasks.begin();
            tasks.pop_front();
            VSYS_DEBUGI("-------------------------------");
            locker.unlock();
            
            speex_echo_cancellation(echo_states[task_id],
                                    input + task_id * FRAME_SIZE_SPEEX,         //mic
                                    input + MIC_CHANNEL * FRAME_SIZE_SPEEX,     //speaker
                                    output + task_id * FRAME_SIZE_SPEEX);
            speex_preprocess_run(preprocess_states[task_id], output + task_id * FRAME_SIZE_SPEEX);
            
            sem_post(pause_ref);
        }
    };
    
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < 2; i++) {
        threads.emplace_back(thread_loop);
    }
    
    uint32_t num_channels = MIC_CHANNEL + SPEAKER_CHANNEL;
    uint32_t total = num_channels * FRAME_SIZE_SPEEX;

    while (pcm_in.good()) {
        pcm_in.read(buff, total * sizeof(short));
        for (uint32_t i = 0; i < num_channels; i++) {
            for (uint32_t j = 0; j < total; j++) {
                input[i * FRAME_SIZE_SPEEX + j] = ((short *)buff)[j * num_channels + i];
            }
        }
        for (uint32_t i = FRAME_SIZE_SPEEX; i < num_channels; i++) {
            for (uint32_t j = 0; j < MIC_CHANNEL; j++) {
                input[i * SPEAKER_CHANNEL +j] = input[j * FRAME_SIZE_SPEEX + i];
            }
        }
        for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
            tasks.push_back(i);
            sem_post(resume_ref);
        }
        for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
            sem_wait(pause_ref);
        }
        pcm_out.write((char *)output, FRAME_SIZE_SPEEX * sizeof(float));
    }

    thread_exit = true;
    for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
        sem_post(resume_ref);
        threads[i].join();
    }
    for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
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

#include"mfcc.h"
#include"wav.h"
void test_mel(){
    ret_value temp;
    short waveData2[60000];
    
    load_wave_file("/Users/daixiang/Desktop/vsys/data/sounds/BAC009S0916W0466.wav", &temp, waveData2);
    MFCC(waveData2, 60000, 16000);
}

int main(int argc, const char * argv[]) {
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
//    test_audio_processing();
//    test_activation();
    test_mel();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp);
    VSYS_DEBUGI("已运行%lld毫秒\n", elapsed.count());
    return 0;
}
