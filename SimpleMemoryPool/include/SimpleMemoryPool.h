#pragma once

#include "Slot.h"

class SimpleMemoryPool{
private:
    Slot slots_[3];

    Slot* freeList_ = nullptr;

public:
    SimpleMemoryPool(){
        for(int i = 2; i >= 0; --i){
            slots_[i].next = freeList_;

            freeList_ = &slots_[i];
        }
    }

    SimpleMemoryPool(const SimpleMemoryPool&) = delete;
    SimpleMemoryPool& operator=(const SimpleMemoryPool&) = delete;

    Slot* allocate(){
        if (freeList_ == nullptr) return nullptr;

        Slot* result = freeList_;
        freeList_ = freeList_ -> next;
        result->next = nullptr;
        return result;
    }

    void deallocate(Slot* slot) {
        if (slot == nullptr) return;

        slot -> next = freeList_;
        freeList_ = slot;
    }

};
