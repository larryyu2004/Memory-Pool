#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <new>
#include <utility>
#include <stdexcept>

template<typename T, std::size_t SlotsPerBlock = 4>
class ObjectPool {
private:
    static_assert(SlotsPerBlock > 0);

    struct Slot {
        alignas(T) std::byte storage[sizeof(T)];

        Slot* next = nullptr;

        // `active` identifies the currently constructed object
        T* active = nullptr;
    };

    // Dynamic array stores a unique pointer points to an array of Slot
    std::vector<std::unique_ptr<Slot[]>> blocks_;

    Slot* freeList_ = nullptr;

    void grow(){
        // create an array contains `SlotsPerBlock` Slots
        // unique pointer `block` points to the start of the array.
        auto block = std::make_unique<Slot[]>(SlotsPerBlock);
        
        // gets the address of the first Slot in the array and stores it in a normal pointer called data.
        Slot* data = block.get();

        // Transfer ownership
        // Without transferring ownership, the local block would be destroyed when grow() returns, 
        // and its managed array would be released.
        // std::move(block) does not move the actual array. It allows ownership of the array to be transferred.
        blocks_.push_back(std::move(block));

        for (std::size_t i = SlotsPerBlock; i > 0; i--) {
            data[i - 1].next = freeList_;
            freeList_ = &data[i - 1];
        }
        // freeList_ -> [Slot 0] -> [Slot 1] -> [Slot 2] -> nullptr
    }

public:
    ObjectPool() = default;

    // Do not copy or move the pool.
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // Construct an object using an available slot.
    template<typename ...Args>
    T* create(Args&&... args) {

        if (freeList_ == nullptr) grow();
        
        Slot* slot = freeList_;
        freeList_ = slot->next;
        slot->next = nullptr;

        try 
        {
            T* object = new (slot->storage) T(std::forward<Args>(args)...);
            slot->active = object;
            return object;
        } catch (...) {
            slot->next = freeList_;
            freeList_ = slot;

            throw;
        }
    }

    void destroy(T* object) {
        if (object == nullptr) return;
        
        for (auto& block : blocks_)
        {
            for (std::size_t i = 0; i < SlotsPerBlock; ++i)
            {
                Slot& slot = block[i];

                if (slot.active == object)
                {
                    slot.active->~T();
                    slot.active = nullptr;

                    slot.next = freeList_;
                    freeList_ = &slot;

                    return;
                }
            }
        }

        throw std::invalid_argument(
            "Object not found in this pool"
        );
    }

    // Destroy any remaining live objects.
    ~ObjectPool()
    {
        for (auto& block : blocks_)
        {
            for (std::size_t i = 0; i < SlotsPerBlock; ++i)
            {
                if (block[i].active != nullptr)
                {
                    block[i].active->~T();
                    block[i].active = nullptr;
                }
            }
        }
    }
};
