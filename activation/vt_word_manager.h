//
//  vt_word_formation.h
//  vsys
//
//  Created by 薯条 on 2018/1/19.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VT_WORD_MANAGER_H
#define VT_WORD_MANAGER_H

#include <string>
#include <vector>
#include <mutex>

#include "zvtapi.h"
#include "vt_phoneme.h"
#include "vsys_types.h"

namespace vsys {
    
enum AcousticModel{
    MODEL_DNN,
    MODEL_CTC,
};
    
class VtWordManager{
public:
    VtWordManager(void* _token, int32_t (*_set)(void* token, const WordInfo* word_info, const uint32_t& word_num), AcousticModel _model)
    :token(_token), set(_set), model(_model){}
    
    int32_t set_vt_word(const vt_word_t* vt_word);
    
    int32_t remove_vt_word(const std::string& word);
    
    int32_t get_vt_words(vt_word_t*& vt_words_out);
    
private:
    bool is_valid_vt_type(word_type type);
    
    bool is_exist(const std::string& word);
    
    WordType get_vt_type(word_type type);
    
    bool pinyin_to_phoneme(const std::string& pinyin, std::string& phone);
    
    bool vt_word_formation(const word_type type, const std::string& word, const std::string& pinyin, WordInfo& word_info);

    bool get_all_vt_words();
    
    int32_t (*set)(void* token, const WordInfo* word_info, const uint32_t& word_num);
    
private:
    std::vector<WordInfo> word_infos;
    
    std::mutex vt_mutex;
    
    AcousticModel model;
    
    void* token;
};
    
}

#endif /* VT_WORD_MANAGER_H */
