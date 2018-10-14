/*******************************************************************************
 * random-sort-pi/neopixel.cpp
 *
 * Copyright (C) 2018 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the GNU General Public License v3.0
 ******************************************************************************/

#include <NeoAnimation/Animation/Sort.hpp>
#include <NeoAnimation/RunAnimation.hpp>
#include <NeoAnimation/Strip/PiSPI_APA102.hpp>

using namespace NeoAnimation;
using namespace NeoSort;

PiSPI_APA102 my_strip("/dev/spidev0.0", /* strip_size */ 5 * 96);

bool g_terminate = false;

int main() {
    srandom(time(nullptr));

    RunRandomSortAnimation(my_strip);

    return 0;
}

/******************************************************************************/
