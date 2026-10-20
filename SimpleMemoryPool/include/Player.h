#pragma once

#include <iostream>

class Player
{
public:
    int health_;

    Player(int hp)
    {
        health_ = hp;
        std::cout << "Player constructed: " << health_ << std::endl;
    }

    ~Player()
    {
        std::cout << "Player destroyed: " << health_ << std::endl;
    }
};