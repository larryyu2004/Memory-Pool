#pragma once

#include <cstddef>
#include "Player.h"

struct Slot
{
    alignas(Player) std::byte storage[sizeof(Player)];

    Slot* next = nullptr;
};