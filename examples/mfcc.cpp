//
//  mfcc.cpp
//  vsys
//
//  Created by 代祥 on 2018/3/22.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include"mfcc.h"
#include"fftw3.h"
#include<cmath>
#include<cstring>
#include<fstream>
#include<string>
using namespace std;

template<class T> void print_Array(T* arr, int len, string filename);
#define TORPINT true
#define PRINT_FRAME 100

float mulMelRec[500][LEN_MELREC];

void MFCC(const short* waveData, int numSamples, int sampleRate){
    if(TORPINT) print_Array(waveData, 60000, "/Users/daixiang/Desktop/vsys/exp/wavDataAll.txt");
    // 预加重
    float* spreemp = new float[numSamples];
    preEmphasizing(waveData, spreemp, numSamples, -0.95);
    if(TORPINT) print_Array(spreemp, 60000, "/Users/daixiang/Desktop/vsys/exp/spreempAll.txt");
    // 计算帧的数量
    int numFrames = ceil((numSamples - FRAMES_PER_BUFFER) / NOT_OVERLAP) + 1;
    // 申请内存
    float* frameWindow = new float[FRAMES_PER_BUFFER];
    float* afterWin = new float[LEN_SPECTRUM];
    float* energySpectrum = new float[LEN_SPECTRUM];
    float* mel = new float[NUM_FILTER];
    float* melRec = new float[LEN_MELREC];
    /*float** mulMelRec = new float*[numFrames + 200];
     for(int i = 0; i < numFrames; i++){
     mulMelRec[i] = new float[LEN_MELREC];
     }*/
    float* sumMelRec = new float[LEN_MELREC];
    memset(sumMelRec, 0, sizeof(float)*LEN_MELREC);
    memset(mulMelRec, 0, sizeof(float)*numFrames*LEN_MELREC);
    // 设置窗参数
    setHammingWindow(frameWindow);
    //setHanningWindow(frameWindow);
    //setBlackManWindow(frameWindow);
    // 帧操作
    for(int i = 0; i < numFrames; i++){
        int j;
        // 加窗操作
        int seg_shift = i * NOT_OVERLAP;
        for(j = 0; j < FRAMES_PER_BUFFER && (seg_shift + j) < numSamples; j++){
            afterWin[j] = spreemp[seg_shift + j] * frameWindow[j];
        }
        // 满足FFT为2^n个点，补零操作
        for(int k = j - 1; k < LEN_SPECTRUM; k++){
            afterWin[k] = 0;
        }
        if(TORPINT && i == PRINT_FRAME)
            print_Array(afterWin, LEN_SPECTRUM, "/Users/daixiang/Desktop/vsys/exp/After.txt");
        // 计算能量谱
        FFT_Power(afterWin, energySpectrum);
        if(TORPINT && i == PRINT_FRAME)
            print_Array(energySpectrum, LEN_SPECTRUM, "/Users/daixiang/Desktop/vsys/exp/energySpectrum.txt");
        // 计算梅尔谱
        memset(mel, 0, sizeof(float)*NUM_FILTER);
        computeMel(mel, sampleRate, energySpectrum);
        if(TORPINT && i == PRINT_FRAME)
            print_Array(mel, NUM_FILTER, "/Users/daixiang/Desktop/vsys/exp/mel.txt");
        // 计算离散余弦变换
        memset(melRec, 0, sizeof(float)*LEN_MELREC);
        DCT(mel, melRec);
        if(TORPINT && i == PRINT_FRAME)
            print_Array(melRec, LEN_MELREC, "/Users/daixiang/Desktop/vsys/exp/melRec.txt");
        // 累计总值
        for(int p = 0; p < LEN_MELREC; p++){
            mulMelRec[i][p] = melRec[p];
            sumMelRec[p] += melRec[p] * melRec[p];
        }
    }
    // 归一化处理
    for(int i = 0; i < LEN_MELREC; i++){
        sumMelRec[i] = sqrt(sumMelRec[i] / numFrames);
    }
    fstream fout("/Users/daixiang/Desktop/vsys/exp/All_MelRec.txt", ios::out);
    fstream fout2("/Users/daixiang/Desktop/vsys/exp/All_MelRec_Bef.txt", ios::out);
    for(int i = 0; i < numFrames; i++){
        for(int j = 0; j < LEN_MELREC; j++){
            fout2 << mulMelRec[i][j] << " ";
            mulMelRec[i][j] /= sumMelRec[j];
            fout << mulMelRec[i][j] << " ";
        }
        fout << endl;
        fout2 << endl;
    }
    fout.close();
    fout2.close();
    
    // 释放内存
    delete[] spreemp;
    delete[] frameWindow;
    delete[] afterWin;
    delete[] energySpectrum;
    delete[] mel;
    delete[] melRec;
    delete[] sumMelRec;
    /*for(int i = 0; i < LEN_MELREC; i++){
     delete[] mulMelRec[i];
     }
     delete[] mulMelRec;*/
}

void preEmphasizing(const short* waveData, float* spreemp, int numSamples, float heavyFactor){
    spreemp[0] = (float)waveData[0];
    for(int i = 1; i < numSamples; i++){
        spreemp[i] = waveData[i] + heavyFactor * waveData[i - 1];
    }
}

void setHammingWindow(float* frameWindow){
    for(int i = 0; i < FRAMES_PER_BUFFER; i++){
        frameWindow[i] = 0.54 - 0.46*cos(2 * PI * i / (FRAMES_PER_BUFFER - 1));
    }
}

void setHanningWindow(float* frameWindow){
    for(int i = 0; i < FRAMES_PER_BUFFER; i++){
        frameWindow[i] = 0.5 - 0.5*cos(2 * PI * i / (FRAMES_PER_BUFFER - 1));
    }
}

void setBlackManWindow(float* frameWindow){
    for(int i = 0; i < FRAMES_PER_BUFFER; i++){
        frameWindow[i] = 0.42 - 0.5*cos(2 * PI * i / (FRAMES_PER_BUFFER - 1))
        + 0.08*cos(4 * PI*i / (FRAMES_PER_BUFFER - 1));
    }
}

void FFT_Power(float* in, float* energySpectrum){
    fftwf_complex* out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*LEN_SPECTRUM);
    fftwf_plan p = fftwf_plan_dft_r2c_1d(LEN_SPECTRUM, in, out, FFTW_ESTIMATE);
    fftwf_execute(p);
    for(int i = 0; i < LEN_SPECTRUM; i++){
        energySpectrum[i] = out[i][0] * out[i][0] + out[i][1] * out[i][1];
    }
    fftwf_destroy_plan(p);
    fftwf_free(out);
}

void computeMel(float* mel, int sampleRate, const float* energySpectrum){
    int fmax = sampleRate / 2;
    float maxMelFreq = 1125 * log(1 + fmax / 700);
    int delta = (int)(maxMelFreq / (NUM_FILTER + 1));
    // 申请空间
    float** melFilters = new float*[NUM_FILTER];
    for(int i = 0; i < NUM_FILTER; i++){
        melFilters[i] = new float[3];
    }
    float* m = new float[NUM_FILTER + 2];
    float* h = new float[NUM_FILTER + 2];
    float* f = new float[NUM_FILTER + 2];
    // 计算频谱到梅尔谱的映射关系
    for(int i = 0; i < NUM_FILTER + 2; i++){
        m[i] = i*delta;
        h[i] = 700 * (exp(m[i] / 1125) - 1);
        f[i] = floor((256 + 1)*h[i] / sampleRate);
    }
    // 计算梅尔滤波参数
    for(int i = 0; i < NUM_FILTER; i++){
        for(int j = 0; j < 3; j++){
            melFilters[i][j] = f[i + j];
        }
    }
    // 梅尔滤波
    for(int i = 0; i < NUM_FILTER; i++){
        for(int j = 0; j < 256; j++){
            if(j >= melFilters[i][0] && j <= melFilters[i][1]){
                mel[i] += ((j - melFilters[i][0]) / (melFilters[i][1] - melFilters[i][0]))*energySpectrum[j];
            }
            else if(j > melFilters[i][1] && j <= melFilters[i][2]){
                mel[i] += ((melFilters[i][2] - j) / (melFilters[i][2] - melFilters[i][1]))*energySpectrum[j];
            }
        }
    }
    // 释放内存
    for(int i = 0; i < 3; i++){
        delete[] melFilters[i];
    }
    delete[] melFilters;
    delete[] m;
    delete[] h;
    delete[] f;
}

void DCT(const float* mel, float* melRec){
    for(int i = 0; i < LEN_MELREC; i++){
        for(int j = 0; j < NUM_FILTER; j++){
            if(mel[j] <= -0.0001 || mel[j] >= 0.0001){
                melRec[i] += log(mel[j])*cos(PI*i / (2 * NUM_FILTER)*(2 * j + 1));
            }
        }
    }
}

//template<class T>
//void print_Array(T* arr, int len, string filename){
//    ofstream fout(filename, ios::out);
//
//    std::ofstream pcm_out(filename, std::ios::out);
//    pcm_out.write((char *)arr, len);
//
//    fout.close();
//    return;
//}

template<class T>
void print_Array(T* arr, int len, string filename){
    fstream fout(filename, ios::out);
    fout << len << endl;
    for(int i = 0; i < len; i++){
        fout << arr[i] << " ";
    }
    fout << endl;
    fout.close();
    return;
}
