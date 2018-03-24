//
//  main.cpp
//  test_mel
//
//  Created by 薯条 on 2018/3/24.
//  Copyright © 2018年 薯条. All rights reserved.
//

#define mel

#include <thread>
#include"mfcc.h"
#include"wav.h"
#include "debug.h"

void test_mel(){
    ret_value temp;
    short waveData2[60000];
    
    load_wave_file("/Users/daixiang/Desktop/vsys/data/sounds/BAC009S0916W0466.wav", &temp, waveData2);
    MFCC(waveData2, 60000, 16000);
}

int main(int argc, const char * argv[]) {
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    test_mel();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp);
    VSYS_DEBUGI("已运行%lld毫秒\n", elapsed.count());
    return 0;
}
