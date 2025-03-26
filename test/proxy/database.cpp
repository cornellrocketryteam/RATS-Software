#include "database.hpp"
#include <iostream>
#include <variant>
#include <map>

// Define a variant type that can hold all of the telemetry field types.
using FieldType = std::variant<uint16_t, uint32_t, int32_t, uint8_t, float>;

void writeRadioTelemetry(const Telemetry &t, std::unique_ptr<influxdb::InfluxDB> &influxdb)
{
    std::cout << "Writing Telemetry data." << std::endl;

    std::map<std::string, FieldType> field_map;
    field_map["metadata"] = t.metadata;
    field_map["ms_since_boot"] = t.ms_since_boot;
    field_map["events"] = t.events;
    field_map["altitude"] = t.altitude;
    field_map["temperature"] = t.temperature;
    field_map["gps_latitude"] = t.gps_latitude;
    field_map["gps_longitude"] = t.gps_longitude;
    field_map["gps_num_satellites"] = t.gps_num_satellites;
    field_map["unix_time"] = t.unix_time;
    field_map["horizontal_accuracy"] = t.horizontal_accuracy;
    field_map["imu_accel_x"] = t.imu_accel_x;
    field_map["imu_accel_y"] = t.imu_accel_y;
    field_map["imu_accel_z"] = t.imu_accel_z;
    field_map["imu_gyro_x"] = t.imu_gyro_x;
    field_map["imu_gyro_y"] = t.imu_gyro_y;
    field_map["imu_gyro_z"] = t.imu_gyro_z;
    field_map["imu_orientation_x"] = t.imu_orientation_x;
    field_map["imu_orientation_y"] = t.imu_orientation_y;
    field_map["imu_orientation_z"] = t.imu_orientation_z;
    field_map["accel_x"] = t.accel_x;
    field_map["accel_y"] = t.accel_y;
    field_map["accel_z"] = t.accel_z;
    field_map["battery_volt"] = t.battery_volt;
    field_map["pressure_pt3"] = t.pressure_pt3;
    field_map["pressure_pt4"] = t.pressure_pt4;
    field_map["rtd_temperature"] = t.rtd_temperature;
    field_map["motor_position"] = t.motor_position;

    // Debug print each field.

    // for (const auto &entry : field_map)
    // {
    //     std::cout << entry.first << ": ";
    //     std::visit([](auto &&value)
    //                { std::cout << value; }, entry.second);
    //     std::cout << std::endl;
    // }

    // Write each field to the database.
    // We use std::visit to dispatch the correct type for each field.
    for (const auto &map_pair : field_map)
    {
        std::visit([&](auto &&value)
                   { influxdb->write(influxdb::Point{"telemetry"}
                                         .addField(map_pair.first, value)
                                         .setTimestamp(std::chrono::system_clock::now())); }, map_pair.second);
    }
}