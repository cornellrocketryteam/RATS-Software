/**
 * @file usb.hpp
 *
 * @brief Bounded forwarding of packed telemetry records to the host proxy.
 */

#ifndef USB_HPP
#define USB_HPP

#include "constants.hpp"
#include "telemetry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace rats {

    /**
     * Core 0 owns TinyUSB and this queue. Call service() regularly from that core;
     * do not also start the SDK's background USB stdio task.
     * Records retain the packed Telemetry layout expected by the host proxy.
     */
    class UsbForwarder {
    public:
        bool begin();

        /** Drops the incoming whole record when disconnected or the queue is full. */
        void enqueue(const Telemetry &packet);

        /** Preserves partial-write offsets and clears queued data on disconnect. */
        void service();

    private:
        std::array<Telemetry, constants::tracking::usbQueueCapacity> packets_ {};
        std::size_t head_ = 0;
        std::size_t count_ = 0;
        std::size_t offset_ = 0;
    };

} // namespace rats

#endif // USB_HPP
