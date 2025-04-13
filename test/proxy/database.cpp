#include "database.hpp"
#include <iostream>
#include <variant>
#include <map>

// Define a variant type that can hold all of the telemetry field types.
using FieldType = std::variant<uint16_t, uint32_t, int32_t, uint8_t, float>;
void initPoint(influxdb::Point& point, const Telemetry &t);


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

void writeRadioTelemetry(const Telemetry &t, std::unique_ptr<influxdb::InfluxDB> &influxdb, int& count)
{
    std::cout << "Writing Telemetry data... " << std::endl;

    // auto point = influxdb::Point{"Fill Radio"};
    // initPoint(point, t);
    const std::string title = "Ground Radio";

    int event_i = 0;
    int meta_i = 0;

    // influxdb->batchOf(1);
    // Write the point to the database
    influxdb->write( influxdb::Point{title}
    .addField("metadata", t.metadata)
    .addField("ms_since_boot", t.ms_since_boot)
    .addField("events", t.events)
    .addField("altimeter_altitude", t.altitude)
    .addField("altimeter_temperature", t.temperature)
    .addField("gps_latitude", t.gps_latitude)
    .addField("gps_longitude", t.gps_longitude)
    .addField("gps_satellites_in_view", t.gps_num_satellites)
    .addField("gps_unix_time", t.unix_time)
    .addField("gps_horizontal_accuracy", t.horizontal_accuracy)
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
    .addField("battery_volt", t.battery_volt)
    .addField("pressure_pt3", t.pressure_pt3)
    .addField("pressure_pt4", t.pressure_pt4)
    .addField("rtd_temperature", t.rtd_temperature)
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

    .setTimestamp(std::chrono::system_clock::now()));
    // influxdb->flushBatch();

    std::cout<<"Data successfully written\n\n";
}



void initPoint(influxdb::Point& point, const Telemetry &t) {

    int event_i = 0;
    int meta_i = 0;
    point.addField("metadata", t.metadata)
        .addField("ms_since_boot", t.ms_since_boot)
        .addField("events", t.events)
        .addField("altimeter_altitude", t.altitude)
        .addField("altimeter_temperature", t.temperature)
        .addField("gps_latitude", t.gps_latitude)
        .addField("gps_longitude", t.gps_longitude)
        .addField("gps_satellites_in_view", t.gps_num_satellites)
        .addField("gps_unix_time", t.unix_time)
        .addField("gps_horizontal_accuracy", t.horizontal_accuracy)
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
        .addField("battery_volt", t.battery_volt)
        .addField("pressure_pt3", t.pressure_pt3)
        .addField("pressure_pt4", t.pressure_pt4)
        .addField("rtd_temperature", t.rtd_temperature)
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

void writeRadioTelemetryCasted(const Telemetry &t, std::unique_ptr<influxdb::InfluxDB> &influxdb, int& count)
{
    std::cout << "Writing Telemetry data... " << std::endl;

    const std::string measurement = "Ground_Radio"; // Using underscore instead of space
    auto timestamp = std::chrono::system_clock::now();

    int event_i = 0;
    int meta_i = 0;

    try {
        // Write the point to the database
        influxdb->write(influxdb::Point{measurement}
            .addField("metadata", static_cast<int64_t>(t.metadata))
            .addField("ms_since_boot", static_cast<int64_t>(t.ms_since_boot))
            .addField("events", static_cast<int64_t>(t.events))
            .addField("altimeter_altitude", static_cast<double>(t.altitude))
            .addField("altimeter_temperature", static_cast<double>(t.temperature))
            .addField("gps_latitude", static_cast<int64_t>(t.gps_latitude))
            .addField("gps_longitude", static_cast<int64_t>(t.gps_longitude))
            .addField("gps_satellites_in_view", static_cast<int64_t>(t.gps_num_satellites))
            .addField("gps_unix_time", static_cast<int64_t>(t.unix_time))
            .addField("gps_horizontal_accuracy", static_cast<int64_t>(t.horizontal_accuracy))
            .addField("imu_acceleration_x", static_cast<double>(t.imu_accel_x))
            .addField("imu_acceleration_y", static_cast<double>(t.imu_accel_y))
            .addField("imu_acceleration_z", static_cast<double>(t.imu_accel_z))
            .addField("imu_gyro_x", static_cast<double>(t.imu_gyro_x))
            .addField("imu_gyro_y", static_cast<double>(t.imu_gyro_y))
            .addField("imu_gyro_z", static_cast<double>(t.imu_gyro_z))
            .addField("imu_orientation_x", static_cast<double>(t.imu_orientation_x))
            .addField("imu_orientation_y", static_cast<double>(t.imu_orientation_y))
            .addField("imu_orientation_z", static_cast<double>(t.imu_orientation_z))
            .addField("accelerometer_acceleration_x", static_cast<double>(t.accel_x))
            .addField("accelerometer_acceleration_y", static_cast<double>(t.accel_y))
            .addField("accelerometer_acceleration_z", static_cast<double>(t.accel_z))
            .addField("battery_volt", static_cast<double>(t.battery_volt))
            .addField("pressure_pt3", static_cast<double>(t.pressure_pt3))
            .addField("pressure_pt4", static_cast<double>(t.pressure_pt4))
            .addField("rtd_temperature", static_cast<double>(t.rtd_temperature))
            .addField("blims_motor_state", static_cast<double>(t.motor_position))
          
            // Add the events to the point - cast bool to int
            .addField("altitude_armed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("altimeter_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("altimeter_reading_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("gps_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("gps_reading_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("imu_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("imu_reading_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("accelerometer_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("accelerometer_reading_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("adc_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("adc_reading_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("fram_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("fram_read_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("fram_write_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("sd_init_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("sd_write_failed", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("mav_was_actuated", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("sv_was_actuated", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("main_deploy_wait_end", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("main_log_shutoff", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("cycle_overflow", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("unknown_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("launch_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("mav_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("sv_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("safe_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("reset_card_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("reset_fram_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("state_change_command_received", static_cast<int64_t>(getEvent(t, event_i++)))
            .addField("umbilical_disconnected", static_cast<int64_t>(getEvent(t, event_i++)))
    
            // Add metadata to the point - cast bool to int
            .addField("alt_armed", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("alt_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("gps_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("imu_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("acc_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("umbilical_connection", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("adc_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("fram_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("sd_valid", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("gps_msg_fresh", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("rocket_was_safed", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("mav_state", static_cast<int64_t>(getMetadata(t, meta_i++)))
            .addField("sv2_state", static_cast<int64_t>(getMetadata(t, meta_i++)))
            
            // String values don't need casting
            .addField("flight_mode", getFlightState(t))
            .setTimestamp(timestamp));
        
        count++;
        std::cout << "Data successfully written (batch " << count << ")\n\n";
    } catch (const std::exception& e) {
        std::cerr << "Error writing to InfluxDB: " << e.what() << std::endl;
    }
}

void writeRadioTelemetryAsIndividualPoints(const Telemetry &t, std::unique_ptr<influxdb::InfluxDB> &influxdb, int& count)
{
    std::cout << "Writing Telemetry data as individual points... " << std::endl;
    const std::string measurement = "Ground_Radio"; // Using underscore instead of space
    
    try {
        // We'll use type casting to convert values to well-supported types
        // Integer values will be cast to int64_t
        // Float values will remain as float/double
        
        // Basic telemetry fields
        influxdb->write(influxdb::Point{measurement}.addField("metadata", static_cast<int64_t>(t.metadata)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("ms_since_boot", static_cast<int64_t>(t.ms_since_boot)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("events", static_cast<int64_t>(t.events)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("altimeter_altitude", static_cast<double>(t.altitude)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("altimeter_temperature", static_cast<double>(t.temperature)).setTimestamp(std::chrono::system_clock::now()));
        
        // GPS data
        influxdb->write(influxdb::Point{measurement}.addField("gps_latitude", static_cast<double>(t.gps_latitude)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("gps_longitude", static_cast<double>(t.gps_longitude)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("gps_satellites_in_view", static_cast<int64_t>(t.gps_num_satellites)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("gps_unix_time", static_cast<int64_t>(t.unix_time)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("gps_horizontal_accuracy", static_cast<double>(t.horizontal_accuracy)).setTimestamp(std::chrono::system_clock::now()));
        
        // IMU data
        influxdb->write(influxdb::Point{measurement}.addField("imu_acceleration_x", static_cast<double>(t.imu_accel_x)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_acceleration_y", static_cast<double>(t.imu_accel_y)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_acceleration_z", static_cast<double>(t.imu_accel_z)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_gyro_x", static_cast<double>(t.imu_gyro_x)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_gyro_y", static_cast<double>(t.imu_gyro_y)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_gyro_z", static_cast<double>(t.imu_gyro_z)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_orientation_x", static_cast<double>(t.imu_orientation_x)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_orientation_y", static_cast<double>(t.imu_orientation_y)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("imu_orientation_z", static_cast<double>(t.imu_orientation_z)).setTimestamp(std::chrono::system_clock::now()));
        
        // Accelerometer data
        influxdb->write(influxdb::Point{measurement}.addField("accelerometer_acceleration_x", static_cast<double>(t.accel_x)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("accelerometer_acceleration_y", static_cast<double>(t.accel_y)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("accelerometer_acceleration_z", static_cast<double>(t.accel_z)).setTimestamp(std::chrono::system_clock::now()));
        
        // Other sensor data
        influxdb->write(influxdb::Point{measurement}.addField("battery_volt", static_cast<double>(t.battery_volt)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("pressure_pt3", static_cast<double>(t.pressure_pt3)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("pressure_pt4", static_cast<double>(t.pressure_pt4)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("rtd_temperature", static_cast<double>(t.rtd_temperature)).setTimestamp(std::chrono::system_clock::now()));
        influxdb->write(influxdb::Point{measurement}.addField("blims_motor_state", static_cast<int64_t>(t.motor_position)).setTimestamp(std::chrono::system_clock::now()));
        
        // Event flags - all boolean values will be cast to integer 0 or 1
        for (int event_i = 0; event_i < 30; event_i++) {
            std::string event_name;
            switch (event_i) {
                case 0: event_name = "altitude_armed"; break;
                case 1: event_name = "altimeter_init_failed"; break;
                case 2: event_name = "altimeter_reading_failed"; break;
                case 3: event_name = "gps_init_failed"; break;
                case 4: event_name = "gps_reading_failed"; break;
                case 5: event_name = "imu_init_failed"; break;
                case 6: event_name = "imu_reading_failed"; break;
                case 7: event_name = "accelerometer_init_failed"; break;
                case 8: event_name = "accelerometer_reading_failed"; break;
                case 9: event_name = "adc_init_failed"; break;
                case 10: event_name = "adc_reading_failed"; break;
                case 11: event_name = "fram_init_failed"; break;
                case 12: event_name = "fram_read_failed"; break;
                case 13: event_name = "fram_write_failed"; break;
                case 14: event_name = "sd_init_failed"; break;
                case 15: event_name = "sd_write_failed"; break;
                case 16: event_name = "mav_was_actuated"; break;
                case 17: event_name = "sv_was_actuated"; break;
                case 18: event_name = "main_deploy_wait_end"; break;
                case 19: event_name = "main_log_shutoff"; break;
                case 20: event_name = "cycle_overflow"; break;
                case 21: event_name = "unknown_command_received"; break;
                case 22: event_name = "launch_command_received"; break;
                case 23: event_name = "mav_command_received"; break;
                case 24: event_name = "sv_command_received"; break;
                case 25: event_name = "safe_command_received"; break;
                case 26: event_name = "reset_card_command_received"; break;
                case 27: event_name = "reset_fram_command_received"; break;
                case 28: event_name = "state_change_command_received"; break;
                case 29: event_name = "umbilical_disconnected"; break;
                default: event_name = "event_" + std::to_string(event_i); break;
            }
            
            // Convert boolean to int64_t (0 or 1)
            int64_t event_value = getEvent(t, event_i) ? 1 : 0;
            influxdb->write(influxdb::Point{measurement}.addField(event_name, event_value).setTimestamp(std::chrono::system_clock::now()));
        }
        
        // Metadata flags - also cast to integers
        for (int meta_i = 0; meta_i < 13; meta_i++) {
            std::string meta_name;
            switch (meta_i) {
                case 0: meta_name = "alt_armed"; break;
                case 1: meta_name = "alt_valid"; break;
                case 2: meta_name = "gps_valid"; break;
                case 3: meta_name = "imu_valid"; break;
                case 4: meta_name = "acc_valid"; break;
                case 5: meta_name = "umbilical_connection"; break;
                case 6: meta_name = "adc_valid"; break;
                case 7: meta_name = "fram_valid"; break;
                case 8: meta_name = "sd_valid"; break;
                case 9: meta_name = "gps_msg_fresh"; break;
                case 10: meta_name = "rocket_was_safed"; break;
                case 11: meta_name = "mav_state"; break;
                case 12: meta_name = "sv2_state"; break;
                default: meta_name = "meta_" + std::to_string(meta_i); break;
            }
            
            int64_t meta_value = getMetadata(t, meta_i) ? 1 : 0;
            influxdb->write(influxdb::Point{measurement}.addField(meta_name, meta_value).setTimestamp(std::chrono::system_clock::now()));
        }
        
        // Flight mode as a string value
        influxdb->write(influxdb::Point{measurement}.addField("flight_mode", getFlightState(t)).setTimestamp(std::chrono::system_clock::now()));
            
        count++;
        std::cout << "Successfully wrote " << count << " batches of telemetry data points" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error writing to InfluxDB: " << e.what() << std::endl;
    }
}