#include <iostream>
#include <InfluxDB/InfluxDBFactory.h>
#include <InfluxDB/InfluxDBBuilder.h>
#include <telemetry.hpp>
#include <map>
#include <variant>
#include <chrono>
#include <thread>

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
    // std::string ground_server_IP = "localhost"; // or "127.0.0.1"
    // std::string port_number = "8086";
    // std::string username = "admin";
    // std::string password = "your_password";
    // std::string db = "telemetry";
    
    // // Format with basic auth included in the URL
    // std::string full_url = "http://" + username + ":" + password + "@" + ground_server_IP + ":" + port_number + "?db=" + db;
    
    // auto influxdb = influxdb::InfluxDBBuilder::http(full_url)
    //                     .setTimeout(std::chrono::seconds{20})
    //                     .connect();



    std::string ground_server_IP = "192.168.1.200";
    std::string port_number = "8086";
    std::string full_url = ground_server_IP + ":" + port_number + "?db=";
    std::string db = "telemetry";
    std::string auth_token =
        "QHt2sPHm6KgRPties04eY_xfAqeuwUuOtuVH1AIBsXoPVWhqhGQlQLR-d1yngmLRL936pR8itzuallB__PGKvg==";
    int TIMEOUT_S = 20;

    auto influxdb = influxdb::InfluxDBBuilder::http(full_url + db)
                        .setTimeout(std::chrono::seconds{TIMEOUT_S})
                        .setAuthToken(auth_token)
                        .connect();
                

    std::cout << "Connecting to InfluxDB at " << ground_server_IP << ":" << port_number << std::endl;


    const int BATCH_SIZE = 10;
    const int SLEEP_TIME_MS = 500; // in milliseconds
    while (true)
    {
        for (int i = 0; i < BATCH_SIZE; i++) {
            Telemetry t = generate_dummy_telemetry();
            int count = 0;
            writeRadioTelemetry(t, influxdb, count);
            std::cout << "Wrote to database" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds( SLEEP_TIME_MS));
    }

    // influxdb->write(influxdb::Point{"test"}
    //                     .addField("value", 10)
    //                     .addTag("host", "localhost"));
}
