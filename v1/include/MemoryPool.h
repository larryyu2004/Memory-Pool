#include <cstddef> // For size_t
#include <mutex>   // For std::mutex
#include <cassert>  // For assert
#include <iostream> // For std::cout


namespace MemoryPool
{
#define MEMORY_POOL_NUM 64
#define SLOT_BASE_SIZE 8
#define MAX_SLOT_SIZE 512

    struct Slot
    {
        std::atomic<Slot *> next; // Atomic pointer
    };

    class MemoryPool
    {
    public:
        MemoryPool(size_t BlockSize = 4096);
        ~MemoryPool();

        void init(size_t);

        void *allocate();
        void deallocate(void *);

    private:
        void allocateNewBlock();
        size_t padPointer(char *p, size_t align);

        bool pushFreeList(Slot* slot);
        Slot* popFreeList();

    private:
        int BlockSize_;    // BlockSize_ is the size of each memory block;
        int SlotSize_;     // SlotSize is the size of each slot;
        Slot *firstBlock_; // Pointer points to the first slot of the memory pool;
        Slot *curSlot_;    // Pointer points to the unused slot of the memory pool;
        std::atomic<Slot *> freeList_;   // Pointer points to the free slot of the memory pool ();
        Slot *lastSlot_;   // Pointer points to the last position that can store element in the memory pool;

        std::mutex mutexForBlock_;    // Prevent unnecessary memory blocks allocation in multiple threads;
    };

}