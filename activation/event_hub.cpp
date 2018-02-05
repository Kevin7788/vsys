//
//  event_hub.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/30.
//  Copyright © 2018年 薯条. All rights reserved.
//

#define LOG_TAG "activation"

#include "debug.h"
#include "event_hub.h"

namespace vsys {
    
EventHub::EventHub(): loop(true){
    thread = std::thread(&EventHub::thread_loop, this);
    event_callback = [](voice_event_t* voice_event, void* token){VSYS_DEBUGI("--------------------------------           %d", voice_event->event);};
}

EventHub::~EventHub(){
    loop = false;
    if(thread.joinable()){
        condition.notify_all();
        thread.join();
    }
}

void EventHub::add_callback(voice_event_callback callback, void* token){
    this->token = token;
    if(callback) event_callback = callback;
}

void EventHub::send_voice_event(voice_event_t *voice_event){
    std::lock_guard<decltype(mutex)> locker(mutex);
    voice_events.emplace_back(voice_event);
    condition.notify_one();
}

void EventHub::thread_loop(){
    
    std::unique_lock<decltype(mutex)> locker(mutex, std::defer_lock);
    
    while (loop) {
        locker.lock();
        condition.wait(locker, [this]{return !voice_events.empty() || !loop;});
        
        if(!voice_events.empty()){
            voice_event_t* voice_event = voice_events.front();
            voice_events.pop_front();
            
            locker.unlock();
            
            event_callback(voice_event, token);
            delete[] (char *)voice_event;
        }
    }
    VSYS_DEBUGI("event thread exit  %d", loop);
}
    
}
