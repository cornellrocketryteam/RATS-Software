/**
 * @file usb.cpp
 *
 * @brief USB telemetry queue with bounded storage and partial-write handling.
 */

#include "usb.hpp"
#include "tusb.h"
#include <algorithm>

#if defined(RATS_VERBOSE) || defined(RATS_TIME) || defined(UBLOX_VERBOSE)
#error "Disable USB text diagnostics when forwarding packed binary telemetry."
#endif

namespace rats {

    bool UsbForwarder::begin() {
        head_ = count_ = offset_ = 0;
        return tusb_init();
    }

    void UsbForwarder::enqueue(const Telemetry &packet) {
        if (!tud_cdc_connected() || count_ == packets_.size())
            return;
        packets_[(head_ + count_) % packets_.size()] = packet;
        ++count_;
    }

    void UsbForwarder::service() {
        tud_task();
        if (!tud_cdc_connected()) {
            head_ = count_ = offset_ = 0;
            return;
        }
        if (count_ == 0)
            return;

        const auto bytes = reinterpret_cast<const std::uint8_t *>(&packets_[head_]);
        const auto amount = std::min<std::size_t>(sizeof(Telemetry) - offset_, tud_cdc_write_available());
        offset_ += tud_cdc_write(bytes + offset_, static_cast<std::uint32_t>(amount));
        tud_cdc_write_flush();
        if (offset_ == sizeof(Telemetry)) {
            head_ = (head_ + 1) % packets_.size();
            --count_;
            offset_ = 0;
        }
    }

} // namespace rats
