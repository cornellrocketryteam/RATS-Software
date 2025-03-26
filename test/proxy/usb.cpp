#include <usb.hpp>
#include <libusb-1.0/libusb.h>
#include <iostream>

const uint16_t vendor_id = 0x0;
const uint16_t product_id = 0x0;

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

    // Set verbosity level for debugging (optional)
    libusb_set_debug(ctx, 3);

    // Replace with your device's Vendor ID and Product ID

    handle = libusb_open_device_with_vid_pid(ctx, vendor_id, product_id);
    if (!handle)
    {
        std::cerr << "Cannot open device." << std::endl;
        libusb_exit(ctx);
        return 1;
    }

    // Claim the interface (usually interface 0)
    int interface_number = 0;
    if (libusb_claim_interface(handle, interface_number) < 0)
    {
        std::cerr << "Cannot claim interface." << std::endl;
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    // Prepare a buffer for data
    unsigned char data[64]; // Adjust size according to your needs
    int actual_length = 0;
    // Endpoint address (0x81 is typical for a bulk IN endpoint, but check your device)
    unsigned char endpoint_address = 0x81;

    // Perform the bulk transfer
    r = libusb_bulk_transfer(handle, endpoint_address, data, sizeof(data), &actual_length, 1000);
    if (r == 0)
    {
        std::cout << "Data read successfully, " << actual_length << " bytes:" << std::endl;
        for (int i = 0; i < actual_length; i++)
        {
            std::cout << std::hex << static_cast<int>(data[i]) << " ";
        }
        std::cout << std::dec << std::endl;
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
