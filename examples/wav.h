//
//  wav.h
//  vsys
//
//  Created by 代祥 on 2018/3/22.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef _WAV_H
#define _WAV_H

#define MAXDATA (512*400)  //一般采样数据大小,语音文件的数据不能大于该数据
#define SFREMQ (16000)   //采样数据的采样频率8khz
#define NBIT 16

typedef struct WaveStruck{//wav数据结构
    //data head
    struct HEAD{
        char cRiffFlag[4];
        int nFileLen;
        char cWaveFlag[4];//WAV文件标志
        char cFmtFlag[4];
        int cTransition;
        short nFormatTag;
        short nChannels;
        int nSamplesPerSec;//采样频率,mfcc为8khz
        int nAvgBytesperSec;
        short nBlockAlign;
        short nBitNumPerSample;//样本数据位数，mfcc为12bit
    } head;
    
    //data block
    struct BLOCK{
        char cDataFlag[4];//数据标志符(data)
        int nAudioLength;//采样数据总数
    } block;
} WAVE;

int wavread(char* filename, double* destination);

struct ret_value
{
    char *data;
    unsigned long size;
    ret_value()
    {
        data = 0;
        size = 0;
    }
};

void load_wave_file(char *fname, struct ret_value *ret, short* waveData2);

#endif
