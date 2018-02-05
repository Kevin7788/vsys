//
//  vt_word_manager.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/19.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include "debug.h"
#include "vt_word_manager.h"

namespace vsys {
    
int32_t VtWordManager::set_vt_word(const vt_word_t* vt_word){
    if(!is_valid_vt_type(vt_word->type)){
        VSYS_DEBUGE("Invalid vt type %d", vt_word->type);
        return -1;
    }
    if(is_exist(vt_word->word_utf8)){
        VSYS_DEBUGI("vt word is existed %s", vt_word->word_utf8);
        return 0;
    }
    WordInfo word_info;
    if(!vt_word_formation(vt_word->type, vt_word->word_utf8, vt_word->pinyin, word_info)){
        return -1;
    }
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    word_infos.push_back(word_info);
    return 0;
}

int32_t VtWordManager::remove_vt_word(const std::string& word){
    
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    
    if(!is_exist(word)){
        VSYS_DEBUGI("vt word no existed %s", word.c_str());
        return -1;
    }
    return 0;
}

int32_t VtWordManager::get_vt_words(vt_word_t*& vt_word_out){
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    return 0;
}
    
bool VtWordManager::is_valid_vt_type(word_type type){
    switch (type) {
        case VSYS_WORD_AWAKE:
        case VSYS_WORD_SLEEP:
        case VSYS_WORD_HOTWORD:
            return true;
    }
    return false;
}
    
bool VtWordManager::is_exist(const std::string& word){
    if(!word_infos.empty()){
        std::vector<WordInfo>::iterator it_begin = word_infos.begin();
        std::vector<WordInfo>::iterator it_end = word_infos.end();
        while (it_begin != it_end) {
            if(!strcmp(word.c_str(), it_begin->pWordContent_UTF8)){
                return true;
            }
            it_begin++;
        }
    }
    return false;
}
    
bool VtWordManager::vt_word_formation(const word_type type, const std::string& word, const std::string& pinyin, WordInfo& word_info){
    std::string vt_word = word;
    std::string vt_phone;
    uint32_t word_size = 0;
    float vt_block_avg_score = 4.2;
    float vt_block_min_score = 2.7;
    
    if(model == AcousticModel::MODEL_CTC){
        vt_phone = pinyin;
        for (uint32_t i = 0; i < pinyin.length(); i++) {
            if(vt_phone[i] >= 48 && vt_phone[i] <= 53){
                word_size++;
                vt_phone[i] = 32;
            }
        }
    }else if(model == AcousticModel::MODEL_DNN){
        if(!pinyin_to_phoneme(pinyin, vt_phone)){
            VSYS_DEBUGE("vt pinyin", pinyin.c_str());
            return false;
        }
    }
    int32_t ecx = (word_size + 1) / 2 - 1;
    if(ecx > 0 && ecx < 3) {
        for(int i = 0; i < ecx; i++) {
            vt_block_avg_score -= 0.5f;
            vt_block_min_score -= 0.5f;
        }
    }else{
        VSYS_DEBUGE("vt word is too long");
        return false;
    }
    if(vt_block_avg_score < 3.2f) vt_block_avg_score = 3.2f;
    if(vt_block_min_score < 1.7f) vt_block_min_score = 1.7f;
    
    word_info.iWordType = get_vt_type(type);
    strcpy(word_info.pWordContent_UTF8, word.c_str());
    strcpy(word_info.pWordContent_PHONE, vt_phone.c_str());
    word_info.fBlockAvgScore = vt_block_avg_score;
    word_info.fBlockMinScore = vt_block_min_score;
    word_info.bLeftSilDet = true;
    word_info.bRightSilDet = false;
    word_info.bRemoteAsrCheckWithAec = true;
    word_info.bRemoteAsrCheckWithNoAec = true;
    word_info.bLocalClassifyCheck = false;
    word_info.fClassifyShield = -0.3;
    return true;
}
    
WordType VtWordManager::get_vt_type(word_type type){
    switch (type) {
        case VSYS_WORD_AWAKE:
            return WORD_AWAKE;
        case VSYS_WORD_SLEEP:
            return WORD_SLEEP;
        case VSYS_WORD_HOTWORD:
            return WORD_HOTWORD;
    }
}

bool VtWordManager::pinyin_to_phoneme(const std::string &pinyin, std::string &phone){
    return false;
}
    
bool VtWordManager::get_all_vt_words(){
    
    uint32_t word_num = word_infos.size();
    char* buf = new char[sizeof(WordInfo) * word_num + 1];
    
    for (uint32_t i = 0; i < word_num + 1; i++) {
        
    }
    return false;
}
    
}
