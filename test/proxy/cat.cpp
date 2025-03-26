#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <InfluxDB/InfluxDBFactory.h>

#include "telemetry.hpp"
#include "database.hpp"
#include "util.hpp"

bool read_cat()
{

    auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

    // Open a pipe to "cat /dev/ttyACM0"
    FILE *fp = popen("cat /dev/ttyACM0", "r");
    if (!fp)
    {
        std::cerr << "Failed to open /dev/ttyACM0 using cat." << std::endl;
        return false;
    }

    // TODO: Make reading and writing happen in different threads
    const int BUFFER_SIZE = sizeof(Telemetry);
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        // std::cout << buffer;
        // std::cout << "\n\n"
        //           << std::endl;

        Telemetry *t = (Telemetry *)buffer;
        print_telemetry(t);
        // writeRadioTelemetry(*t, influxdb);
    }

    pclose(fp);
    return true;
}
