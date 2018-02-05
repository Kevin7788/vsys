//
//  vsys_activation.h
//  vsys
//
//  Created by 薯条 on 2018/1/17.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VSYS_ACTIVATION_H
#define VSYS_ACTIVATION_H

#include "vsys_types.h"

typedef uint64_t VsysActivationInst;
typedef void (*voice_event_callback)(voice_event_t* voice_event, void* token);

#ifdef __cplusplus
extern "C" {
#endif
    
VsysActivationInst VsysActivation_Create(const activation_param_t* param, const char* path, bool vad_enable);

void VsysActivation_Free(VsysActivationInst handle);
    
void VsysActivation_RegisterVoiceEventCallback(VsysActivationInst handle, voice_event_callback callback, void* token);

int32_t VsysActivation_Process(VsysActivationInst handle, const uint8_t* input, const size_t byte_size);
    
int32_t VsysActivation_Control(VsysActivationInst handle, active_action action);
    
//int32_t VsysActivation_Config(VsysActivationInst handle, active_param key, const void* val);
    
int32_t VsysActivation_AddVtWord(VsysActivationInst handle, const vt_word_t* vt_word);
    
int32_t VsysActivation_RemoveVtWord(VsysActivationInst handle, const char* word);
    
int32_t VsysActivation_GetVtWords(VsysActivationInst handle, vt_word_t** vt_words_out);

#ifdef __cplusplus
}
#endif

#endif /* VSYS_ACTIVATION_H */
