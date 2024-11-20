/**
 * @file main.cpp
 * @author YOUR NETID HERE
 * 
 * @brief The entry point for the program
 */

#include <cstdio>
#include "pico/stdlib.h"
#include "pins.hpp"

int main() {
    stdio_init_all();

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 1);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    return 0;
}