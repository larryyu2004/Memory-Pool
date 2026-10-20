#include <iostream>
#include <new>
#include "SimpleMemoryPool.h"
using namespace std;

int main()
{
    SimpleMemoryPool pool;

    // Allocate all three slots.
    Slot* a = pool.allocate();
    Slot* b = pool.allocate();
    Slot* c = pool.allocate();

    // The pool is now full.
    Slot* extra = pool.allocate();

    cout << "Pool exhausted: "
         << boolalpha
         << (extra == nullptr)
         << endl;

    // Construct objects in the slots.
    Player* p1 = new (a->storage) Player(100);
    Player* p2 = new (b->storage) Player(80);
    Player* p3 = new (c->storage) Player(60);

    // Destroy Player 1.
    p1->~Player();

    // Return its slot to the pool.
    pool.deallocate(a);

    // Allocate again.
    Slot* d = pool.allocate();

    cout << "Reused Slot A: "
         << boolalpha
         << (d == a)
         << endl;

    // Construct another Player in the reused slot.
    Player* p4 = new (d->storage) Player(120);

    // Clean up the remaining objects.
    p2->~Player();
    pool.deallocate(b);

    p3->~Player();
    pool.deallocate(c);

    p4->~Player();
    pool.deallocate(d);

    return 0;
}
