//
//  vt_word_manager.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/19.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include <cctype>

#include "debug.h"
#include "vt_word_manager.h"

namespace vsys {
    
int32_t VtWordManager::add_vt_word(const vt_word_t* vt_word){
    if(!is_valid_vt_type(vt_word->type)){
        VSYS_DEBUGE("Unknown vt type %d", vt_word->type);
        return -1;
    }
    
    
    
    WordInfo word_info;
    if(!vt_word_formation(vt_word->type, vt_word->word_utf8, vt_word->phone, word_info)){
        return -1;
    }
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
//    word_infos.push_back(word_info);
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
//    if(!word_infos.empty()){
//        std::vector<WordInfo>::iterator it_begin = word_infos.begin();
//        std::vector<WordInfo>::iterator it_end = word_infos.end();
//        while (it_begin != it_end) {
//            if(!strcmp(word.c_str(), it_begin->pWordContent_UTF8)){
//                return true;
//            }
//            it_begin++;
//        }
//    }
    return false;
}
    
bool VtWordManager::vt_word_formation(const word_type type, const std::string& word, const std::string& pinyin, WordInfo& word_info){
    std::string vt_word = word;
    std::string phone;
    uint32_t word_size;
    float block_avg_score = 4.2;
    float block_min_score = 2.7;
    
    word_size = get_word_size(pinyin);
    int32_t iter = (word_size + 1) / 2 - 1;
    if(iter > 0 && iter < 3) {
        for(int i = 0; i < iter; i++) {
            block_avg_score -= 0.5f;
            block_min_score -= 0.5f;
        }
    }else{
        return false;
    }
    if(block_avg_score < 3.2f) block_avg_score = 3.2f;
    if(block_min_score < 1.7f) block_min_score = 1.7f;
    
    if(!pinyin2phoneme(pinyin, phone)){
        return false;
    }
    word_info.iWordType = get_vt_type(type);
    strcpy(word_info.pWordContent_UTF8, word.c_str());
    strcpy(word_info.pWordContent_PHONE, phone.c_str());
    word_info.fBlockAvgScore = block_avg_score;
    word_info.fBlockMinScore = block_min_score;
    word_info.bLeftSilDet = true;
    word_info.bRightSilDet = false;
    word_info.bRemoteAsrCheckWithAec = true;
    word_info.bRemoteAsrCheckWithNoAec = true;
    word_info.bLocalClassifyCheck = false;
    word_info.fClassifyShield = -0.3;
    return true;
}

bool VtWordManager::pinyin2phoneme(const std::string &pinyin, std::string &phone){
    std::string result;
    if(vt_model == AcousticModel::MODEL_DNN){
        uint32_t left = 0, right = 0;
        std::string target;
        bool is_first = true;
        
        uint32_t length = pinyin.length();
        while (right < length) {
            if(!std::isalnum(pinyin[right])){
                VSYS_DEBUGE("contains bad pinyin : %s", pinyin.c_str());
                return false;
            }
            if (std::isdigit(pinyin[right])){
                target.assign(pinyin, left, right - left);
                std::string phone = phoneme->find_phoneme(target);
                if(phone.empty()){
                    VSYS_DEBUGE("cannot find phoneme for %s", target.c_str());
                    return false;
                }
                if(!is_first){
                    result.append(" ");
                }
                result.append(phone);
                is_first = false;
                left = right + 1;
            }
            right++;
        }
    }else if(vt_model == AcousticModel::MODEL_CTC){
        result = pinyin;
        for (uint32_t i = 0; i < pinyin.length(); i++) {
            if(std::isdigit(result[i])){
                result[i] = 32;
            }
        }
    }else{
        VSYS_DEBUGE("unknown model");
    }
    phone.assign(result);
    return true;
}
    
uint32_t VtWordManager::get_word_size(const std::string& pinyin){
    uint32_t word_size = 0;
    for (uint32_t i = 0; i < pinyin.length(); i++) {
        if(pinyin[i] >= 48 && pinyin[i] <= 53){
            word_size++;
        }
    }
    return word_size;
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
    
bool VtWordManager::get_all_vt_words(){
    
    uint32_t word_num = word_infos.size();
    char* buf = new char[sizeof(WordInfo) * word_num + 1];
    
    for (uint32_t i = 0; i < word_num + 1; i++) {
        
    }
    return false;
}
    
}
