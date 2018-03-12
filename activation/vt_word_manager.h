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
#include "vsys_types.h"
#include "phoneme.h"

namespace vsys {
    
enum AcousticModel{
    MODEL_DNN,
    MODEL_CTC,
};
    
class VtWordManager{
public:
    VtWordManager(void* _token, int32_t (*_sync_vt_word)(void* token, const WordInfo* word_info, const uint32_t& word_num), AcousticModel _model)
    :token(_token), sync_vt_word(_sync_vt_word), phoneme(std::make_shared<Phoneme>()), vt_model(_model){}
    
    int32_t add_vt_word(const vt_word_t* vt_word);
    
    int32_t remove_vt_word(const std::string& word);
    
    int32_t get_vt_words(vt_word_t*& vt_words_out);
    
private:
    bool is_valid_vt_type(word_type type);
    
    bool is_exist(const std::string& word);
    
    uint32_t get_word_size(const std::string& pinyin);
    
    WordType get_vt_type(word_type type);
    
    bool pinyin2phoneme(const std::string& pinyin, std::string& phone);
    
    bool vt_word_formation(const word_type type, const std::string& word, const std::string& pinyin, WordInfo& word_info);

    bool get_all_vt_words();
    
    int32_t (*sync_vt_word)(void* token, const WordInfo* word_info, const uint32_t& word_num);
    
private:
    std::vector<vt_word_t> word_infos;
    
    std::shared_ptr<Phoneme> phoneme;
    
    std::mutex vt_mutex;
    
    AcousticModel vt_model;
    
    void* token;
};
    
}

#endif /* VT_WORD_MANAGER_H */
