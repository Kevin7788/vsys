//
//  wav.cpp
//  vsys
//
//  Created by 代祥 on 2018/3/22.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include "wav.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>

int wavread(char* filename, double* destination){
    WAVE wave[1];
    FILE * f;
    f = fopen(filename, "rb");
    if(!f)
    {
        printf("Cannot open %s for reading\n", filename);
        return -1;
    }
    
    //读取wav文件头并且分析
    fread(wave, 1, sizeof(wave), f);
    
    if(wave[0].head.cWaveFlag[0] == 'W'&&wave[0].head.cWaveFlag[1] == 'A'
       &&wave[0].head.cWaveFlag[2] == 'V'&&wave[0].head.cWaveFlag[3] == 'E')//判断是否是wav文件
    {
        printf("It's not .wav file\n");
        return -1;
    }
    if(wave[0].head.nSamplesPerSec != SFREMQ || wave[1].head.nBitNumPerSample != NBIT)//判断是否采样频率是16khz,16bit量化
    {
        printf("It's not 16khz and 16 bit\n");
        return -1;
    }
    
    if(wave[0].block.nAudioLength>MAXDATA / 2)//wav文件不能太大,为sample长度的一半
    {
        printf("wav file is to long\n");
        return -1;
    }
    
    //读取采样数据
    fread(destination, sizeof(char), wave[0].block.nAudioLength, f);
    fclose(f);
    
    return wave[0].block.nAudioLength;
}

void load_wave_file(char *fname, struct ret_value *ret, short* waveData2)
{
    FILE *fp;
    fp = fopen(fname, "rb");
    if(fp)
    {
        char id[5];          // 5个字节存储空间存储'RIFF'和'\0'，这个是为方便利用strcmp
        unsigned long size;  // 存储文件大小
        short format_tag, channels, block_align, bits_per_sample;    // 16位数据
        unsigned long format_length, sample_rate, avg_bytes_sec, data_size; // 32位数据
        fread(id, sizeof(char), 4, fp); // 读取'RIFF'
        id[4] = '\0';
        
        if(!strcmp(id, "RIFF"))
        {
            fread(&size, sizeof(unsigned int), 1, fp); // 读取文件大小
            fread(id, sizeof(char), 4, fp);         // 读取'WAVE'
            id[4] = '\0';
            if(!strcmp(id, "WAVE"))
            {
                fread(id, sizeof(char), 4, fp);     // 读取4字节 "fmt ";
                fread(&format_length, sizeof(unsigned int), 1, fp);
                fread(&format_tag, sizeof(short), 1, fp); // 读取文件tag
                fread(&channels, sizeof(short), 1, fp);    // 读取通道数目
                fread(&sample_rate, sizeof(unsigned int), 1, fp);   // 读取采样率大小
                fread(&avg_bytes_sec, sizeof(unsigned int), 1, fp); // 读取每秒数据量
                fread(&block_align, sizeof(short), 1, fp);     // 读取块对齐
                fread(&bits_per_sample, sizeof(short), 1, fp);       // 读取每一样本大小
                fread(id, sizeof(char), 4, fp);                      // 读入'data'
                fread(&data_size, sizeof(unsigned int), 1, fp);     // 读取数据大小
                ret->size = data_size;
                ret->data = (char*)malloc(sizeof(char)*data_size); // 申请内存空间
                //fread(ret->data, sizeof(char), data_size, fp);       // 读取数据
                fread(waveData2, sizeof(short), data_size, fp); // my fix
            }
            else
            {
                printf("Error: RIFF file but not a wave file\n");
            }
        }
        else
        {
            printf("Error: not a RIFF file\n");
        }
    }
}
