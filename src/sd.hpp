/**
 * @file sd.hpp
 * @author sj728
 *
 * @brief SD card interface for the program
 */

#ifndef SD_HPP
#define SD_HPP

#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include "telemetry.hpp"

/**
 * Container for SD card-related functionality.
 */
class SD
{
public:
    /**
     * Begins the SD card interface by mounting the SD card.
     * @return True on successful mount, false on mount failure.
     */
    bool begin();

    /**
     * Logs telemetry to the SD card.
     * @return True on successful log, false on file open, write, or close failures.
     */
    bool log_telemetry(const Telemetry &telemetry);

    /**
     * Tracks the number of writes made to the current log file.
     */
    uint16_t writes_count = 0;

    /**
     * SD card representation.
     */
    sd_card_t *pSD;

    /**
     * File system representation.
     */
    FATFS fs;

    /**
     * Log file object representation.
     */
    FIL log_file;

};

#endif // SD_HPP