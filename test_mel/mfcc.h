//
//  mfcc.h
//  vsys
//
//  Created by 代祥 on 2018/3/22.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef _MFCC_H
#define _MFCC_H

#define FRAMES_PER_BUFFER 400
#define NOT_OVERLAP 200
#define NUM_FILTER 40
#define PI 3.1415926
#define LEN_SPECTRUM 512
#define LEN_MELREC 13

void MFCC(const short* waveData, int numSamples, int sampleRate);
void preEmphasizing(const short* waveData, float* spreemp, int numSamples, float heavyFactor);
void setHammingWindow(float* frameWindow);
void setHanningWindow(float* frameWindow);
void setBlackManWindow(float* frameWindow);
void FFT_Power(float* in, float* energySpectrum);
void computeMel(float* mel, int sampleRate, const float* energySpectrum);
void DCT(const float* mel, float* melRec);

#endif
