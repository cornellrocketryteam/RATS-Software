#include <usb.hpp>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <cstring>       // for memcpy
#include "telemetry.hpp" // Ensure this header is available for Telemetry definition

const uint16_t vendor_id = 0x2e8a;
const uint16_t product_id = 0x000a;

bool read_usb()
{
    libusb_context *ctx = nullptr;
    libusb_device_handle *handle = nullptr;
    int r = libusb_init(&ctx);
    if (r < 0)
    {
        std::cerr << "Failed to initialize libusb: " << r << std::endl;
        return false;
    }

    // Optional: set debug level
    libusb_set_debug(ctx, 3);

    handle = libusb_open_device_with_vid_pid(ctx, vendor_id, product_id);
    if (!handle)
    {
        std::cerr << "Cannot open device." << std::endl;
        libusb_exit(ctx);
        return false;
    }

    int interface_number = 1; // Change from 0 to 1

    // Detach kernel driver if active on interface 1
    if (libusb_kernel_driver_active(handle, interface_number) == 1)
    {
        int detach_result = libusb_detach_kernel_driver(handle, interface_number);
        if (detach_result < 0)
        {
            std::cerr << "Cannot detach kernel driver: " << detach_result << std::endl;
            libusb_close(handle);
            libusb_exit(ctx);
            return false;
        }
    }

    if (libusb_claim_interface(handle, interface_number) < 0)
    {
        std::cerr << "Cannot claim interface." << std::endl;
        libusb_close(handle);
        libusb_exit(ctx);
        return false;
    }

    // Prepare a buffer for data. Increase the size if expecting multiple Telemetry objects.
    const int buffer_size = 1024;
    unsigned char data[buffer_size];
    int actual_length = 0;
    // Adjust endpoint_address as needed for your device
    unsigned char endpoint_address = 0x82;
    const int transfer_time_ms = 3'000; // 3 seconds

    r = libusb_bulk_transfer(handle, endpoint_address, data, buffer_size, &actual_length, transfer_time_ms);
    if (r == 0)
    {
        std::cout << "Data read successfully, " << actual_length << " bytes." << std::endl;

        // Determine how many complete Telemetry objects were received.
        size_t telemetry_count = actual_length / sizeof(Telemetry);
        std::cout << "Received " << telemetry_count << " Telemetry object(s)." << std::endl;

        for (size_t i = 0; i < telemetry_count; i++)
        {
            Telemetry telemetry;
            // Copy each Telemetry object from the buffer.
            std::memcpy(&telemetry, data + i * sizeof(Telemetry), sizeof(Telemetry));

            // Example processing: print one of the fields
            std::cout << "Telemetry " << i << " - Altitude: " << telemetry.altitude << std::endl;
            // You can add additional processing or pass the telemetry object to other functions here.
        }
    }
    else
    {
        std::cerr << "Error in bulk transfer: " << r << std::endl;
    }

    // Clean up
    libusb_release_interface(handle, interface_number);
    libusb_close(handle);
    libusb_exit(ctx);

    return true;
}
