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

    class HashBucket 
    {
    public:
        static void initMemoryPool();
        static MemoryPool & getMemoryPool(int index);

        static void *useMemory(size_t size)
        {
            if(size <= 0) {
                return nullptr;
            }
            if(size > MAX_SLOT_SIZE) {
                // If it needs to allocate >= 512 bytes, use new();
                return operator new(size);
            }

            return getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).allocate();
        }

        static void freeMemory(void *ptr, size_t size) {
            if (!ptr) {
                return;
            }

            if(size > MAX_SLOT_SIZE) {
                operator delete(ptr);
                return;
            }

            getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).deallocate(ptr);
        }

        template <typename T, typename... Args>
        friend T *newElement(Args &&...args);

        template <typename T>
        friend void deleteElement(T *p);
    };

    template <typename T, typename... Args>
    T *newElement(Args &&... args)
    {
        T *p = nullptr;

        // Allocate memory based on the size of the element;
        if((p = reinterpret_cast<T *>(HashBucket::useMemory(sizeof(T))) != nullptr)) {

            // Create an new object of type T;
            new (p) T(std::forward<Args>(args)...);
        }
        
        return p;
    }


    template <typename T>
    void deleteElement(T *p)
    {
        if(p) {

            // Here, p is a pointer to an object of type T;
            // Deconstruct the object;
            p->~T();
            HashBucket::freeMemory(reinterpret_cast<void *>(p), sizeof(T));
        }
    }

} // namespace MemoryPool