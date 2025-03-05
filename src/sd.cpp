/**
 * @file sd.cpp
 * @author sj728
 *
 * @brief SD card class impelementation for logging data
 */

#include "sd.hpp"

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

// TODO: Make the file unique every time the program is re-executed
bool SD::log_telemetry(Telemetry &telemetry)
{
    // Open the log file
    FRESULT fr = f_open(&log_file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
    {
        return false;
    }

    // Write the telemetry data to the log file
    if (f_printf(&log_file, "Time: %d, Altitude: %d, Temperature: %d\n",
                 telemetry.unix_time, telemetry.altitude, telemetry.temperature) < 0)
    {
        return false;
    }

    // Close the log file
    fr = f_close(&log_file);
    if (FR_OK != fr)
    {
        return false;
    }

    return true;
}