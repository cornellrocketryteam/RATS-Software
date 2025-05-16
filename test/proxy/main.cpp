#include <iostream>
#include <InfluxDB/InfluxDBFactory.h>
#include <InfluxDB/InfluxDBBuilder.h>
#include <telemetry.hpp>
#include <map>
#include <variant>
#include <chrono>
#include <thread>

#include "cat.hpp"
#include "util.hpp"
#include "database.hpp"

int main()
{
    std::cout << "Starting Program" << std::endl;
    read_cat();
}

