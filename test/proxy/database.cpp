#include "database.hpp"
#include <iostream>
#include <variant>
#include <map>

// Define a variant type that can hold all of the telemetry field types.
using FieldType = std::variant<uint16_t, uint32_t, int32_t, uint8_t, float>;

void writeRadioTelemetry(const Telemetry &t, std::unique_ptr<influxdb::InfluxDB> &influxdb)
{
    std::cout << "Writing Telemetry data... " << std::endl;

    auto point = influxdb::Point{"Ground Radio"};
    initPoint(point, t);


    influxdb->write(std::move(point));

    std::cout<<"Data successfully written\n\n";
}


bool getEvent(const Telemetry &t, int event_i)
{
    // Check if the event index is within bounds
    if (event_i < 0 || event_i >= sizeof(t.events) * 8)
    {
        std::cerr << "Event index out of bounds" << std::endl;
        return false;
    }

    // Check if the event at the given bit index is set
    return t.events & (1 << event_i);
}

bool getMetadata(const Telemetry &t, int meta_i)
{
    // Check if the metadata index is within bounds
    if (meta_i < 0 || meta_i >= sizeof(t.metadata) * 8)
    {
        std::cerr << "Metadata index out of bounds" << std::endl;
        return false;
    }

    // Check if the metadata at the given bit index is set
    return t.metadata & (1 << meta_i);
}

std::string getFlightState(const Telemetry &t) {
    const int FLIGHT_BIT = 13;
    const int MASK = 0b111;

    // Only read bits[13-15]
    int value = (t.events & (1 << FLIGHT_BIT)) & MASK;
    std::string flight_state;
    switch (value) {
        case 0b000:
            flight_state = "Startup";
            break;
        case 0b001:
            flight_state = "Standby";
            break;
        case 0b010:
            flight_state = "Ascent";
            break;
        case 0b011:
            flight_state = "Drogue Deployed";
            break;
        case 0b100:
            flight_state = "Main Deployed";
            break;
        case 0b101:
            flight_state = "Fault";
            break;
        default:
            flight_state = "Unknown";
            break;
    }

    return flight_state;
}

void initPoint(influxdb::Point& point, const Telemetry &t) {

    int event_i = 0;
    int meta_i = 0;
    point.addField("metadata", t.metadata)
        .addField("ms_since_boot", t.timestamp)
        .addField("events", t.events)
        .addField("altimeter_altitude", t.altitude)
        .addField("altimeter_temperature", t.alt_temp)
        .addField("gps_latitude", t.gps_latitude)
        .addField("gps_longitude", t.gps_longitude)
        .addField("gps_satellites_in_view", t.gps_num_satellites)
        .addField("gps_unix_time", t.gps_utc_time)
        .addField("imu_acceleration_x", t.imu_accel_x)
        .addField("imu_acceleration_y", t.imu_accel_y)
        .addField("imu_acceleration_z", t.imu_accel_z)
        .addField("imu_gyro_x", t.imu_gyro_x)
        .addField("imu_gyro_y", t.imu_gyro_y)
        .addField("imu_gyro_z", t.imu_gyro_z)
        .addField("imu_orientation_x", t.imu_orientation_x)
        .addField("imu_orientation_y", t.imu_orientation_y)
        .addField("imu_orientation_z", t.imu_orientation_z)
        .addField("accelerometer_acceleration_x", t.accel_x)
        .addField("accelerometer_acceleration_y", t.accel_y)
        .addField("accelerometer_acceleration_z", t.accel_z)
        .addField("battery_volt", t.voltage)
        .addField("pressure_pt3", t.pressure_pt3)
        .addField("pressure_pt4", t.pressure_pt4)
        .addField("rtd_temperature", t.alt_temp)
        .addField("blims_motor_state", t.motor_position)
      
        // Add the events to the point
        .addField("altitude_armed", getEvent(t, event_i++))
        .addField("altimeter_init_failed", getEvent(t, event_i++))
        .addField("altimeter_reading_failed", getEvent(t, event_i++)) 
        .addField("gps_init_failed", getEvent(t, event_i++))
        .addField("gps_reading_failed", getEvent(t, event_i++)) 
        .addField("imu_init_failed", getEvent(t, event_i++)) 
        .addField("imu_reading_failed", getEvent(t, event_i++)) 
        .addField("accelerometer_init_failed", getEvent(t, event_i++)) 
        .addField("accelerometer_reading_failed", getEvent(t, event_i++)) 
        .addField("adc_init_failed", getEvent(t, event_i++)) 
        .addField("adc_reading_failed", getEvent(t, event_i++)) 
        .addField("fram_init_failed", getEvent(t, event_i++)) 
        .addField("fram_read_failed", getEvent(t, event_i++)) 
        .addField("fram_write_failed", getEvent(t, event_i++)) 
        .addField("sd_init_failed", getEvent(t, event_i++)) 
        .addField("sd_write_failed", getEvent(t, event_i++)) 
        .addField("mav_was_actuated", getEvent(t, event_i++)) 
        .addField("sv_was_actuated", getEvent(t, event_i++)) 
        .addField("main_deploy_wait_end", getEvent(t, event_i++)) 
        .addField("main_log_shutoff", getEvent(t, event_i++)) 
        .addField("cycle_overflow", getEvent(t, event_i++)) 
        .addField("unknown_command_received", getEvent(t, event_i++)) 
        .addField("launch_command_received", getEvent(t, event_i++)) 
        .addField("mav_command_received", getEvent(t, event_i++)) 
        .addField("sv_command_received", getEvent(t, event_i++)) 
        .addField("safe_command_received", getEvent(t, event_i++)) 
        .addField("reset_card_command_received", getEvent(t, event_i++)) 
        .addField("reset_fram_command_received", getEvent(t, event_i++)) 
        .addField("state_change_command_received", getEvent(t, event_i++)) 
        .addField("umbilical_disconnected", getEvent(t, event_i++))

        // Add metadata to the point
        .addField("alt_armed", getMetadata(t, meta_i++))
        .addField("alt_valid", getMetadata(t, meta_i++))
        .addField("gps_valid", getMetadata(t, meta_i++))
        .addField("imu_valid", getMetadata(t, meta_i++))
        .addField("acc_valid", getMetadata(t, meta_i++))
        .addField("umbilical_connection", getMetadata(t, meta_i++))
        .addField("adc_valid", getMetadata(t, meta_i++))
        .addField("fram_valid", getMetadata(t, meta_i++))
        .addField("sd_valid", getMetadata(t, meta_i++))
        .addField("gps_msg_fresh", getMetadata(t, meta_i++))
        .addField("rocket_was_safed", getMetadata(t, meta_i++))
        .addField("mav_state", getMetadata(t, meta_i++))
        .addField("sv2_state", getMetadata(t, meta_i++))
        .addField("flight_mode", getFlightState(t))

        .setTimestamp(std::chrono::system_clock::now());
}

