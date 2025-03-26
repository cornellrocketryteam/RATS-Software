#include <iostream>
#include <InfluxDB/InfluxDBFactory.h>
#include <telemetry.hpp>
#include <map>
#include <variant>

#include "usb.hpp"
#include "cat.hpp"
#include "util.hpp"
#include "database.hpp"

void start_usb();
void start_database();
void start_cat();

int main()
{
    std::cout << "Starting Program" << std::endl;
    start_cat();
    // start_database();
}

void start_cat()
{
    if (!read_cat())
    {
        std::cout << "CAT function failed, ";
    };

    std::cout << "Ran CAT function" << std::endl;
}

void start_usb()
{
    if (!read_usb())
    {
        std::cout << "USB function failed, ";
    };

    std::cout << "Ran USB function" << std::endl;
}

void start_database()
{
    // Example write
    auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

    while (true)
    {
        Telemetry t = generate_dummy_telemetry();
        writeRadioTelemetry(t, influxdb);
        std::cout << "Wrote to database" << std::endl;
    }

    // influxdb->write(influxdb::Point{"test"}
    //                     .addField("value", 10)
    //                     .addTag("host", "localhost"));
}
