/**
 * @file sd.cpp
 * @author sj728
 *
 * @brief SD card class impelementation for logging data
 */

#include "sd.hpp"
#include "string"
#include "time.h"

#ifdef RATS_VERBOSE
#define debug_log(...)       \
    do                       \
    {                        \
        printf(__VA_ARGS__); \
    } while (0)
#else
#define debug_log(...) \
    do                 \
    {                  \
    } while (0)
#endif

std::string filename;
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
bool SD::log_telemetry(const Telemetry &telemetry)
{
    // Open the log file
    FRESULT fr = f_open(&log_file, "Test-5-16-2025.txt", FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
    {
        debug_log("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }

    // Write the telemetry data to the log file
    if (f_printf(&log_file, 
                "Metadata: %u, "
                "MS Since Boot: %u, "
                "Events: %u, "
                "Altitude: %.2f, "
                "Temperature: %.2f, "
                "GPS Latitude: %d, "
                "GPS Longitude: %d, "
                "GPS Satellites: %u, "
                "Unix Time: %u, "
                "Horizontal Accuracy: %u, "
                "IMU Accel X: %.2f, "
                "IMU Accel Y: %.2f, "
                "IMU Accel Z: %.2f, "
                "IMU Gyro X: %.2f, "
                "IMU Gyro Y: %.2f, "
                "IMU Gyro Z: %.2f, "
                "IMU Orientation X: %.2f, "
                "IMU Orientation Y: %.2f, "
                "IMU Orientation Z: %.2f, "
                "Accel X: %.2f, "
                "Accel Y: %.2f, "
                "Accel Z: %.2f, "
                "Battery Volt: %.2f, "
                "Pressure PT3: %.2f, "
                "Pressure PT4: %.2f, "
                "RTD Temperature: %.2f, "
                "Motor Position: %.2f\n",
                telemetry.metadata,
                telemetry.ms_since_boot,
                telemetry.events,
                telemetry.altitude,
                telemetry.temperature,
                telemetry.gps_latitude,
                telemetry.gps_longitude,
                telemetry.gps_num_satellites,
                telemetry.unix_time,
                telemetry.horizontal_accuracy,
                telemetry.imu_accel_x,
                telemetry.imu_accel_y,
                telemetry.imu_accel_z,
                telemetry.imu_gyro_x,
                telemetry.imu_gyro_y,
                telemetry.imu_gyro_z,
                telemetry.imu_orientation_x,
                telemetry.imu_orientation_y,
                telemetry.imu_orientation_z,
                telemetry.accel_x,
                telemetry.accel_y,
                telemetry.accel_z,
                telemetry.battery_volt,
                telemetry.pressure_pt3,
                telemetry.pressure_pt4,
                telemetry.rtd_temperature,
                telemetry.motor_position) < 0)
    {
        debug_log("f_printf failed\n");
        return false;
    }

    // Close the log file
    fr = f_close(&log_file);
    if (FR_OK != fr)
    {
        debug_log("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }

    debug_log("Telemeetry data written in file\n");
    return true;
}

