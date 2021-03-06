/*******************************************************************************
 * blinken-sort-pi/blinken-sort.cpp
 *
 * Copyright (C) 2018 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the GNU General Public License v3.0
 ******************************************************************************/

#include <BlinkenAlgorithms/Animation/RandomAlgorithm.hpp>
#include <BlinkenAlgorithms/Strip/PiSPI_APA102.hpp>

using namespace BlinkenAlgorithms;

PiSPI_APA102 my_strip("/dev/spidev0.0", /* strip_size */ 5 * 96);

bool g_terminate = false;
size_t g_delay_factor = 1000;

int main() {
    srandom(time(nullptr));

    while (1) {
        RunRandomAlgorithmAnimation(my_strip);
    }

    return 0;
}

/******************************************************************************/
