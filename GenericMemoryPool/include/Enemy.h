#pragma once

#include <iostream>
#include <string>
#include <utility>

class Enemy
{
public:
    std::string name;
    int power;

    Enemy(std::string n, int p): name(std::move(n)), power(p)
    {
        std::cout << "Enemy created: "<< name << '\n';
    }

    ~Enemy()
    {
        std::cout << "Enemy destroyed: " << name << '\n';
    }
};
