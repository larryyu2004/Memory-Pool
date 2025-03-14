#include "../include/MemoryPool.h"  


namespace MemoryPool
{
    MemoryPool::MemoryPool(size_t BlockSize)
        : BlockSize_(BlockSize),
        SlotSize_(0),
        firstBlock_(nullptr),
        curSlot_(nullptr),
        freeList_(nullptr),
        lastSlot_(nullptr) {}

    MemoryPool::~MemoryPool() {
        Slot* cur = firstBlock_;
        while (cur) {
            Slot* next = cur->next;

            operator delete(reinterpret_cast<void *>(cur));
            cur = next;
        }
    }

    void MemoryPool::init(size_t size) {
        // Assert is a debugging tool;
        // It will check if size > 0,
        // If not, it will terminate the program and throw an error;
        assert(size > 0); 
        SlotSize_ = size;
        firstBlock_ = nullptr;
        curSlot_ = nullptr;
        freeList_ = nullptr;
        lastSlot_ = nullptr;
    }

    void* MemoryPool::allocate() {

        // Memory slots in free linked list are preferred;
        Slot* slot = popFreeList();
        if(slot != nullptr) {
            return slot;
        }

        Slot* temp;

        {
            // Atomically lock the mutex;
            // Prevent unnecessary memory blocks allocation in multiple threads;
            std::lock_guard<std::mutex> lock(mutexForBlock_);

            // If there is any slot can be used, allocate a new memory block;
            if(curSlot_ >= lastSlot_) {
                allocateNewBlock();
            }

            temp = curSlot_;

            // In the 64 bit OS, the size of Slot (pointer) is 8 bytes;
            curSlot_ += SlotSize_ / sizeof(Slot);
        }
        return temp;
    }

    void MemoryPool::deallocate(void* ptr) {
        if(!ptr) {
            return;
        }

        Slot* slot = reinterpret_cast<Slot *>(ptr);
        pushFreeList(slot);
    }

    void MemoryPool::allocateNewBlock() {

        std::cout << "Apply for a new memory block, SlotSize: " << SlotSize_ << std::endl;

        // A new block is inserted is inserted by head insertion;
        // Example: Block1 (<-firstBlock) -> Block2 -> ...
        // Block0 (newBlock) -> Block1 (<-firstBlock) -> Block2 -> ...
        // Block0 (newBlock, <-firstBlock) -> Block1  -> Block2 -> ...
        void* newBlock = operator new(BlockSize_);
        reinterpret_cast<Slot *>(newBlock) -> next = firstBlock_;
        firstBlock_ = reinterpret_cast<Slot *>(newBlock);

        // A char is always 1 byte in size;
        // Example:
        // SlotSize_ = 16;
        // BlockSize_ = 64;
        // sizeof(Slot *) = 8;

        // Address: 0x1000 - 0x103F (64 bytes);
        // body = 0x1000 + 8 = 0x1008;
        // paddingSize = (16 - 8) % 16 = 8;
        // curSlot_ = 0x1008 + 8 = 0x1010;
        char* body = reinterpret_cast<char *>(newBlock) + sizeof(Slot *);
        size_t paddingSize = padPointer(body, SlotSize_);
        curSlot_ = reinterpret_cast<Slot *>(body + paddingSize);

        // lastSlot = 0x1000 + 64 - 16 + 1 = 0x103F;
        lastSlot_ = reinterpret_cast<Slot *>(newBlock) + BlockSize_ - SlotSize_ + 1; 

        freeList_ = nullptr;
    }

    size_t MemoryPool::padPointer(char* p, size_t align) {
        return (align - reinterpret_cast<size_t>(p)) % align;
    }



    bool MemoryPool::pushFreeList(Slot* slot) {
        // Use CAD to implement lock-free queue operations;

        // Example:
        // Assume freeList_ (pointer) -> Slot1 (Free Slot)
        // ThreafA and ThreadB;
        // ThreadA: oldHead = Slot1; ThreadB: oldhead = Slot1;

        // Firstly, ThreadA: SlotA -> next = oldHead (Slot1); ThreadB: SlotB -> next = oldHead (Slot1);
        // Secondly, check if SlotA -> next == oldHead (Slot1), and update freeList_ -> SlotA;

        // Now, threadB find SlotB -> next (oldHead (Slot1)) != SlotA
        // ThreadB: update SlotB -> next = SlotA;
        // Check SlotB -> next (oldHead (SlotA)) == SlotA, update freeList_ -> SlotB;

        // Now freeList_ -> SlotB -> SlotA -> Slot1;
        while (true) {

            // Get current head node;
            Slot* oldHead = freeList_.load(std::memory_order_relaxed);

            // Set next node of current thread's head to oldHead;
            slot -> next.store(oldHead, std::memory_order_relaxed);

            if(freeList_.compare_exchange_weak(oldHead, slot, std::memory_order_release, std::memory_order_relaxed)) {
                return true;
            }

            // If a thread fails
            // Means another thread has already modified the first node;
            // This thread needs to try again;
        }
    }


    Slot* MemoryPool::popFreeList() {
        // Use CAS to implement lock-free queue operations;

        // Example:
        // Assume freeList_ (pointer) -> Slot1 -> Slot2 -> Slot3;
        // ThreafA and ThreadB;

        // ThreadA: oldHead = Slot1; ThreadB: oldhead = Slot1;
        // ThreadA: newHead = Slot2; ThreadB: newHead = Slot2;
        // ThreadA: oldHead (Slot1) == freeList_ (pointer) -> Slot1;
        // Update freeList_ -> Slot2 (newHead) -> Slot3;

        // Now, threadB: oldHead (Slot1) != freeList_ (pointer) -> Slot2;
        // ThreadB: update oldHead = Slot2; newHead = Slot3;
        // ThreadB: oldHead (Slot2) == freeList_ (pointer) -> Slot2;
        // Update freeList_ -> Slot3;

        while(true) {

            // Update current head node;
            Slot* oldHead = freeList_.load(std::memory_order_relaxed);

            if(oldHead == nullptr) {
                return nullptr; // empty queue;
            }

            // Update the next node of the current head node;
            Slot* newHead = oldHead->next.load(std::memory_order_relaxed);

            // Compare the thread's oldHead with the freelist_;
            if(freeList_.compare_exchange_weak(oldHead, newHead, std::memory_order_acquire, std::memory_order_relaxed)) {
                return oldHead;
            }

            // If a thread fails
            // Means another thread has already modified the first node;
            // This thread needs to try again;
        }
    }


    void HashBucket::initMemoryPool() {
        for(int i = 0; i < MEMORY_POOL_NUM; i++) {
            getMemoryPool(i);
        }
    }

    MemoryPool& HashBucket::getMemoryPool(int index) {
        static MemoryPool memoryPool[MEMORY_POOL_NUM];
        return memoryPool[index];
    } 

} // namespace MemoryPool