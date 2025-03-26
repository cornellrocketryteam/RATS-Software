/**
 * @file radio.cpp
 * @author sj728
 *
 * @brief Test to collect all metadata from radio module (RFD900x)
 */

#include <cstdio>
#include <cstring>
#include <chrono>
#include <vector>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "pins.hpp"
#include "tusb.h"
#include "sd.hpp"
#include "telemetry.hpp"
#include "radio.hpp"
#include "ublox_mx.hpp"
#include "ublox_nav_pvt.hpp"
#include "hardware/i2c.h"

#define LED 25

#define RFM_TX 4
#define RFM_RX 5
#define UART_PORT uart1

// Timeout Constants
#define RESPONSE_TIMEOUT_MS 1000 // Time to wait after last received character
#define MAX_RESPONSE_SIZE 1024   // Maximum expected response size

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

void printTelemetry(const Telemetry *t);
Telemetry generate_dummy_telemetry();

const char *command_enter_seq = "+++";
const char *save_values_seq = "AT&W\r";
const char *reboot_seq = "ATZ\r";
const int rfm_baudrate = 115200;

void print_command(const char *str)
{
    printf("Sending Command: ");

    // Keep going until null terminator is reached
    while (*str)
    {
        if (*str != '\r')
        {
            putchar(*str);
        }
        str++;
    }

    printf("\n");
}

/* UART pins are initialized by:
    1) Initializating UART pins with desired baudrate
    2) Setting what the funtion does
    3) Setting the UART format, which in order is:
        - Data Size
        - # of end bits
        - Whether parity bit will exist or not
    4) Setting whether UART will have TX/RX flow control
    5) Setting if FIFO will be enabled
*/
void init_uart()
{
    // By default, the radio module has baudrate value rfm_baudrate, so we
    // set that value on the local UART port
    uart_init(UART_PORT, rfm_baudrate);

    gpio_set_function(RFM_TX, GPIO_FUNC_UART);
    gpio_set_function(RFM_RX, GPIO_FUNC_UART);

    // Set UART format to 8 data bits, 1 stop bit, no parity
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);

    // Set flow control to false for both
    uart_set_hw_flow(UART_PORT, false, false);

    uart_set_fifo_enabled(UART_PORT, true);
}

const int UART_BUFFER_SIZE = 256;
int idx = 0;
char buffer[UART_BUFFER_SIZE];

void read_uart()
{

    // while (uart_is_readable(UART_PORT)) {
    //     if (uart_is_readable(UART_PORT)) {
    //         char ch = uart_getc(UART_PORT);
    //         printf("%c", ch);
    //     // x++;
    //     }
    // }
    while (uart_is_readable(UART_PORT))
    {
        char c = uart_getc(UART_PORT);
        // Simple example: store in buffer, write to file when newline or buffer full
        if (c == '\n' || idx >= (UART_BUFFER_SIZE - 1))
        {
            buffer[idx] = '\0'; // Null terminate
            idx = 0;

            printf("Received: %s\n", buffer);
            idx = 0;
        }
        else
        {
            buffer[idx++] = c;
        }
    }
}

void get_result(uint8_t value)
{
    printf("Starting...\n");

    uart_write_blocking(UART_PORT, (const uint8_t *)command_enter_seq, strlen(command_enter_seq));
    printf("Wrote \"%s\"\n", command_enter_seq);
    sleep_ms(1000);

    char command[20];
    sprintf(command, "ATI%d\r", value);
    uart_write_blocking(UART_PORT, (const uint8_t *)command, strlen(command));
    printf("Wrote \"%s\"\n", command);
    sleep_ms(1000);

    while (true)
    {
        if (uart_is_readable(UART_PORT))
        {
            char ch = uart_getc(UART_PORT);
            printf("%c", ch);
        }
    }
}

// Function to send a command and wait for its complete response
bool send_command_and_wait_response(const char *cmd, char *response_buffer, size_t buffer_size)
{

    // Clear the response buffer
    // TODO: Experiment if printf() a buffer is significantly
    // faster than printf() a char as it comes in
    memset(response_buffer, 0, buffer_size);
    size_t response_index = 0;

    // Send the command via UART
    print_command(cmd); // Log the command being sent
    uart_write_blocking(UART_PORT, (const uint8_t *)cmd, strlen(cmd));

    if (cmd == command_enter_seq)
    {
        printf("Command mode starting...\n");
        sleep_ms(2000);
    }

    if (uart_is_readable_within_us(UART_PORT, 1000000))
    {
        while (uart_is_readable_within_us(UART_PORT, 1000))
        {
            // while (uart_is_readable(UART_PORT)) {
            char c = uart_getc(UART_PORT);

            // Append the character to the response buffer if there's space
            if (response_index < (buffer_size - 1))
            {
                response_buffer[response_index++] = c;
                response_buffer[response_index] = '\0'; // Null-terminate
            }
            else
            {
                printf("Response buffer overflow.\n");
                return false;
            }
            printf("%c", c);
        }
    }

    printf("\nComplete Response Received.\n\n");

    return true;
}

void get_all_results()
{
    // Buffer to store responses
    char response[MAX_RESPONSE_SIZE];
    send_command_and_wait_response(command_enter_seq, response, sizeof(response));

    std::vector<const char *> commands = {
        "ATI\r",
        "ATI2\r",
        "ATI3\r",
        "ATI4\r",
        "ATI5\r",
        "ATI6\r",
        "ATI7\r",
        "ATI8\r",
        "ATI9\r"};
    for (const char *cmd : commands)
    {
        send_command_and_wait_response(cmd, response, sizeof(response));
    }
}

int t_main()
{
    stdio_init_all();

    init_uart();

    gpio_init(LED);

    gpio_set_dir(LED, GPIO_OUT);

    // Needed for computer to see Raspberry Pico output properly
    while (!tud_cdc_connected())
    {
        sleep_ms(500);
    }

    sleep_ms(4000);
    get_all_results();

    printf("Done.\n");

    // Give enough time for all printf's to finish
    sleep_ms(500);
    return 0;
}

void read_radio()
{
    Radio radio;
    if (!radio.start())
    {
        printf("Radio failed to start\n");
        return;
    }

    while (true)
    {
        std::vector<Telemetry> telemetry_packets;

        bool success = radio.read(telemetry_packets);

        if (success)
        {

            const int num_elements = 1;
            for (const Telemetry &telemetry : telemetry_packets)
            {
                // printf("Timestamp: %u\n", telemetry.timestamp);
                printTelemetry(&telemetry);
            }
            printf("\n");
        }
        // sleep_ms(1000);
        // printf("End of line\n");
    }
}

extern "C" void tud_cdc_task(void)
{
    if (tud_cdc_connected())
    {
        Telemetry telemetry = generate_dummy_telemetry();
        tud_cdc_write(&telemetry, sizeof(telemetry));
        tud_cdc_write_flush(); // ensure it's sent
    }
}

void read_dummy()
{
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    const int num_elements = 1;
    const int SLEEP_TIME_MS = 1000;
    while (true)
    {
        Telemetry telemetry = generate_dummy_telemetry();
        // printTelemetry(&telemetry);
        // tud_cdc_task();

        fwrite(&telemetry, sizeof(Telemetry), num_elements, stdout);
        fflush(stdout);

        // Toggle the LED state: if it's on, turn it off; if off, turn it on.
        bool current_led_state = gpio_get(LED);
        gpio_put(LED, !current_led_state);

        sleep_ms(SLEEP_TIME_MS);
    }
}
int main()
{
    stdio_init_all();

    // while (!tud_cdc_connected())
    // {
    //     sleep_ms(500);
    // }
    // printf("Connected to computer\n");

    // read_radio();
    read_dummy();

    return 0;
}

void printTelemetry(const Telemetry *t)
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
