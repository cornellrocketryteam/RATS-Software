#ifndef DATABASE_HPP
#define DATABASE_HPP
#include <InfluxDB/InfluxDBFactory.h>
#include <telemetry.hpp>

void writeRadioTelemetry(const Telemetry &t, std::unique_ptr<influxdb::InfluxDB> &influxdb);
#endif // DATABASE_HPP