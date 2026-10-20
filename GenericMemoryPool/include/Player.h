#pragma once

#include <iostream>

class Player
{
public:
    int health;

    explicit Player(int hp) : health(hp)
    {
        std::cout << "Player created: " << health << '\n';
    }

    ~Player()
    {
        std::cout << "Player destroyed: " << health << '\n';
    }
};
