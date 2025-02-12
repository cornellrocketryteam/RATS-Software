/**
 * @file main.cpp
 * @author YOUR NETID HERE
 * 
 * @brief The entry point for the program
 */

#include <cstdio>
#include "pico/stdlib.h"

#define LED 25

int main() {
    stdio_init_all();

    gpio_init(LED);

    gpio_set_dir(LED, GPIO_OUT);

    while (true) {
        printf("Hello, world!\n");

        gpio_put(LED, 1);
        sleep_ms(1000);
        gpio_put(LED, 0);
        sleep_ms(1000);
    }

    return 0;
}