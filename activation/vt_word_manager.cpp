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
    if(has_vt_word(vt_word->word_utf8)){
        VSYS_DEBUGE("%s already exists", vt_word->word_utf8);
        return -1;
    }
    {
        std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
        vt_words.push_back(*vt_word);
    }
    return sync_vt_word();
}

int32_t VtWordManager::remove_vt_word(const std::string& word){
    bool find = false;
    {
        std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
        
        std::vector<vt_word_t>::iterator it =  vt_words.begin();
        while (it != vt_words.end()) {
            if(word.compare(it->word_utf8) == 0){
                vt_words.erase(it);
                find = true;
            }
        }
    }
    if(!find) {
        VSYS_DEBUGE("%s is not exists", word.c_str());
        return -1;
    }
    return sync_vt_word();
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
    
bool VtWordManager::has_vt_word(const std::string &word){
    uint32_t word_num = vt_words.size();
    for (uint32_t i = 0; i < word_num; i++) {
        if(word.compare(vt_words[i].word_utf8) == 0){
            return true;
        }
    }
    return false;
}
    
uint32_t VtWordManager::vt_word_formation(WordInfo *&word_infos){
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    
    uint32_t word_num = vt_words.size();
    WordInfo* __word_infos = new WordInfo[word_num];
    
    for (uint32_t i = 0; i < word_num; i++) {
        std::string phone;
    
        float classify_shield = vt_words[i].classify_shield;
        float block_avg_score = vt_words[i].block_avg_score;
        float block_min_score = vt_words[i].block_min_score;
        if(block_avg_score < 3.2f) block_avg_score = 3.2f;
        if(block_min_score < 1.7f) block_min_score = 1.7f;
    
        bool left_sil_det = (vt_words[i].mask & VT_WORD_LEFT_SIL_DET_MASK) != 0 ? true : false;
        bool right_sil_det = (vt_words[i].mask & VT_WORD_RIGHT_SIL_DET_MASK) != 0 ? true : false;
        bool remote_asr_check_with_aec = (vt_words[i].mask & VT_WORD_REMOTE_CHECK_WITH_AEC_MASK) != 0 ? true : false;
        bool remote_asr_check_with_noaec = (vt_words[i].mask & VT_WORD_REMOTE_CHECK_WITH_NOAEC_MASK) != 0 ? true : false;
        bool local_classify_check = (vt_words[i].mask & VT_WORD_LOCAL_CLASSIFY_CHECK_MASK) != 0 ? true : false;
    
        if((vt_words[i].mask & VT_WORD_USE_OUTSIDE_PHONE_MASK) != 0){
            if(!pinyin2phoneme(vt_words[i].phone, phone)){
                return false;
            }
        }else{
            phone = vt_words[i].phone;
        }
        word_infos[i].iWordType = get_vt_type(vt_words[i].type);
        strcpy(__word_infos[i].pWordContent_UTF8, vt_words[i].word_utf8);
        strcpy(__word_infos[i].pWordContent_PHONE, phone.c_str());
        __word_infos[i].fBlockAvgScore = block_avg_score;
        __word_infos[i].fBlockMinScore = block_min_score;
        __word_infos[i].bLeftSilDet = left_sil_det;
        __word_infos[i].bRightSilDet = right_sil_det;
        __word_infos[i].bRemoteAsrCheckWithAec = remote_asr_check_with_aec;
        __word_infos[i].bRemoteAsrCheckWithNoAec = remote_asr_check_with_noaec;
        __word_infos[i].bLocalClassifyCheck = local_classify_check;
        __word_infos[i].fClassifyShield = classify_shield;
    }
    return word_num;
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
    
int32_t VtWordManager::sync_vt_word(){
    WordInfo* word_infos = nullptr;
    uint32_t word_num = 0;
    if((word_num = vt_word_formation(word_infos)) == 0){
        return -1;
    }
    return sync(token, word_infos, word_num);
}
    
}
