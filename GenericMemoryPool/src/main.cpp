#include "Player.h"
#include "Enemy.h"
#include "ObjectPool.h"

int main()
{
    ObjectPool<Player, 2> playerPool;
    ObjectPool<Enemy, 2> enemyPool;

    Player* p1 = playerPool.create(100);
    Player* p2 = playerPool.create(200);

    Enemy* e1 = enemyPool.create("Goblin", 50);

    // Remember the original storage address.
    const void* oldAddress =
        static_cast<const void*>(p1);

    // Destroy p1 and recycle its slot.
    playerPool.destroy(p1);

    // Reuse that slot for a new Player.
    Player* p3 = playerPool.create(300);

    std::cout << std::boolalpha;

    std::cout << "Reused Player slot: "
              << (static_cast<const void*>(p3)
                  == oldAddress)
              << '\n';

    playerPool.destroy(p2);
    playerPool.destroy(p3);
    enemyPool.destroy(e1);

    return 0;
}