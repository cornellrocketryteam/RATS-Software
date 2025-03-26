#include <cstdio>
#include <cstring>
#include <chrono>
#include <vector>
#include <cstdio>
#include "telemetry.hpp"

Telemetry generate_dummy_telemetry()
{
    // Static variables to hold and increment dummy values
    static uint16_t metadata_val = 0;
    static uint32_t ms_since_boot_val = 0;
    static uint32_t events_val = 0;
    static float altitude_val = 0.0f;
    static float temperature_val = 20.0f;          // Starting temperature
    static int32_t gps_latitude_val = 37000000;    // e.g., representing 37.000000° in fixed point
    static int32_t gps_longitude_val = -122000000; // e.g., representing -122.000000°
    static uint8_t gps_num_satellites_val = 5;
    static uint32_t unix_time_val = 1609459200;   // Jan 1, 2021
    static uint32_t horizontal_accuracy_val = 50; // 50 meters
    static float imu_accel_x_val = 0.0f;
    static float imu_accel_y_val = 0.0f;
    static float imu_accel_z_val = 9.8f;
    static float imu_gyro_x_val = 0.0f;
    static float imu_gyro_y_val = 0.0f;
    static float imu_gyro_z_val = 0.0f;
    static float imu_orientation_x_val = 0.0f;
    static float imu_orientation_y_val = 0.0f;
    static float imu_orientation_z_val = 0.0f;
    static float accel_x_val = 0.0f;
    static float accel_y_val = 0.0f;
    static float accel_z_val = 0.0f;
    static float battery_volt_val = 12.0f;
    static float pressure_pt3_val = 101.3f;
    static float pressure_pt4_val = 101.3f;
    static float rtd_temperature_val = 25.0f;
    static float motor_position_val = 0.0f;

    Telemetry t;
    t.metadata = metadata_val;
    t.ms_since_boot = ms_since_boot_val;
    t.events = events_val;
    t.altitude = altitude_val;
    t.temperature = temperature_val;
    t.gps_latitude = gps_latitude_val;
    t.gps_longitude = gps_longitude_val;
    t.gps_num_satellites = gps_num_satellites_val;
    t.unix_time = unix_time_val;
    t.horizontal_accuracy = horizontal_accuracy_val;
    t.imu_accel_x = imu_accel_x_val;
    t.imu_accel_y = imu_accel_y_val;
    t.imu_accel_z = imu_accel_z_val;
    t.imu_gyro_x = imu_gyro_x_val;
    t.imu_gyro_y = imu_gyro_y_val;
    t.imu_gyro_z = imu_gyro_z_val;
    t.imu_orientation_x = imu_orientation_x_val;
    t.imu_orientation_y = imu_orientation_y_val;
    t.imu_orientation_z = imu_orientation_z_val;
    t.accel_x = accel_x_val;
    t.accel_y = accel_y_val;
    t.accel_z = accel_z_val;
    t.battery_volt = battery_volt_val;
    t.pressure_pt3 = pressure_pt3_val;
    t.pressure_pt4 = pressure_pt4_val;
    t.rtd_temperature = rtd_temperature_val;
    t.motor_position = motor_position_val;

    // Increment static variables for next call
    metadata_val++;
    ms_since_boot_val += 100; // Simulate a 100 ms increase per call
    events_val++;
    altitude_val += 0.5f;
    temperature_val += 0.1f;
    gps_latitude_val += 10; // Small fixed-point increment
    gps_longitude_val += 10;
    gps_num_satellites_val = (gps_num_satellites_val % 12) + 1; // Cycle between 1 and 12
    unix_time_val += 1;
    horizontal_accuracy_val += 1;
    imu_accel_x_val += 0.01f;
    imu_accel_y_val += 0.01f;
    imu_accel_z_val += 0.01f;
    imu_gyro_x_val += 0.005f;
    imu_gyro_y_val += 0.005f;
    imu_gyro_z_val += 0.005f;
    imu_orientation_x_val += 0.02f;
    imu_orientation_y_val += 0.02f;
    imu_orientation_z_val += 0.02f;
    accel_x_val += 0.1f;
    accel_y_val += 0.1f;
    accel_z_val += 0.1f;
    battery_volt_val += 0.05f;
    pressure_pt3_val += 0.1f;
    pressure_pt4_val += 0.1f;
    rtd_temperature_val += 0.1f;
    motor_position_val += 0.5f;

    return t;
}

void print_telemetry(const Telemetry *t)
{
    if (t == NULL)
    {
        printf("Telemetry pointer is NULL\n");
        return;
    }

    printf("Telemetry:\n");
    printf("  Metadata: %u\n", t->metadata);
    printf("  Timestamp: %u\n", t->ms_since_boot);
    printf("  Events: %u\n", t->events);
    printf("  Altitude: %f\n", t->altitude);
    printf("  Temperature: %f\n", t->temperature);
    printf("  GPS Latitude: %d\n", t->gps_latitude);
    printf("  GPS Longitude: %d\n", t->gps_longitude);
    printf("  GPS Number of Satellites: %u\n", t->gps_num_satellites);
    printf("  Unix Time: %u\n", t->unix_time);
    printf("  Horizontal Accuracy: %u\n", t->horizontal_accuracy);
    printf("  IMU Accel X: %f\n", t->imu_accel_x);
    printf("  IMU Accel Y: %f\n", t->imu_accel_y);
    printf("  IMU Accel Z: %f\n", t->imu_accel_z);
    printf("  IMU Gyro X: %f\n", t->imu_gyro_x);
    printf("  IMU Gyro Y: %f\n", t->imu_gyro_y);
    printf("  IMU Gyro Z: %f\n", t->imu_gyro_z);
    printf("  IMU Orientation X: %f\n", t->imu_orientation_x);
    printf("  IMU Orientation Y: %f\n", t->imu_orientation_y);
    printf("  IMU Orientation Z: %f\n", t->imu_orientation_z);
    printf("  Accel X: %f\n", t->accel_x);
    printf("  Accel Y: %f\n", t->accel_y);
    printf("  Accel Z: %f\n", t->accel_z);
    printf("  Battery Voltage: %f\n", t->battery_volt);
    printf("  Pressure PT3: %f\n", t->pressure_pt3);
    printf("  Pressure PT4: %f\n", t->pressure_pt4);
    printf("  RTD Temperature: %f\n", t->rtd_temperature);
    printf("  Motor Position: %f\n", t->motor_position);
}
