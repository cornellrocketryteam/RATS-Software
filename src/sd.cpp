/**
 * @file sd.cpp
 * @author sj728
 *
 * @brief SD card class impelementation for logging data
 */

#include "sd.hpp"

/* Begins the SD card interface by mounting the SD card.
 * @return True on successful mount, false on mount failure.
 */
bool SD::begin()
{
    // Mount the SD card
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr)
    {
        return false;
    }
    return true;
}

/**
 * Logs the current state to the SD card.
 * @return True on successful log, false on file open, write, or close failures.
 */
bool SD::log()
{
}