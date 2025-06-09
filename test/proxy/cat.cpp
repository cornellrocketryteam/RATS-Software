#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <InfluxDB/InfluxDBFactory.h>
#include <InfluxDB/InfluxDBBuilder.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <filesystem>
#include <termios.h>

#include "telemetry.hpp"
#include "database.hpp"
#include "util.hpp"

const int BUFFER_SIZE = 10000; 

// All Picos have the same vendor ID
const std::string PICO_VENDOR_ID = "2e8a";


void loop(int fd, std::unique_ptr<influxdb::InfluxDB> &influxdb);


int open_serial(const std::string &port) {
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) throw std::runtime_error("open failed: " + std::string(strerror(errno)));


    // *** START: DTR HANDSHAKE CODE ***
    // This tells the Pico we are ready
    int status;
    ioctl(fd, TIOCMGET, &status); // Get current modem status bits
    status |= TIOCM_DTR;         // Set the DTR bit
    ioctl(fd, TIOCMSET, &status); // Apply the new status
    // *** END: DTR HANDSHAKE CODE ***

    struct termios tty;
    tcgetattr(fd, &tty);

    // put in raw mode (turn off all echo, canonical, and CR/NL conversions)
    cfmakeraw(&tty);

    // set baud rate
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // enable the receiver and set local mode
    tty.c_cflag |= CLOCAL | CREAD;

    // apply immediately
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        throw std::runtime_error("tcsetattr failed: " + std::string(strerror(errno)));

    return fd;
}



int loop_byte_by_byte(int fd, char *buffer, int buffer_size, float timeout_ms)
{
    // Set up a buffer for one byte
    int num_bytes = 0;
    while (num_bytes < buffer_size)
    {
        // Set up the file descriptor set.
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // Set up the timeout.
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = 1;

        // Wait for data on fd with the specified timeout.
        int ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            perror("select");
            break;
        }
        if (ret == 0)
        {
            // Timeout reached; no new byte arrived in the given time.
            // printf("Timeout reached, no data for %d ms. Breaking out of loop.\n", timeout_ms);
            break;
        }

        // If data is available, read one byte.
        ssize_t n = read(fd, (buffer + num_bytes), 1);
        if (n < 0)
        {
            perror("read");
            break;
        }
        if (n == 0)
        {
            // End-of-file or stream closed.
            printf("EOF reached, breaking out of loop.\n");
            break;
        }

        // Process the byte (here we simply print it).
        // For binary data, you might want to store it in a buffer.
        // printf("Read byte: '%c' (0x%02x)\n", byte, (unsigned char)byte);
        num_bytes++;
    }

    return num_bytes;
}


/*
* Find the tty port of the pico. Assumes only one pico is connected.
*/
bool is_pico(const std::string &tty) {
    std::string base = "/sys/class/tty/" + tty + "/device/../";
    std::ifstream vid(base + "idVendor");

    return vid && std::string((std::istreambuf_iterator<char>(vid)), {}).contains(PICO_VENDOR_ID);
}

std::string find_pico_port() {
    for (auto &entry : std::filesystem::directory_iterator("/dev")) {
        auto name = entry.path().filename().string();
        if (name.rfind("ttyACM", 0) == 0 && is_pico(name)) {
            std::cout << "Found pico at " << name << std::endl;
            return "/dev/" + name;
        }
    }
    throw std::runtime_error("Pico not found");
}


bool read_cat()
{
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

   
    int fd = open_serial(find_pico_port());
    loop(fd, influxdb);
    close(fd);
    return true;
}


void loop(int fd, std::unique_ptr<influxdb::InfluxDB> &influxdb)
{
    // TODO: Make reading and writing happen in different threads
    char buffer[BUFFER_SIZE];
    static int count = 0;
    while (true)
    {
        const float WAIT_MS = 1.f;
        
        int bytes_read = loop_byte_by_byte(fd, buffer, BUFFER_SIZE, WAIT_MS);
    
        if (bytes_read == 0) {
            continue;
        }

        std::cout << "Read " << bytes_read << " bytes\n" << std::endl;
        std::cout << "Read attempt count: " << count++ << std::endl;

        // Process only if we got a complete Telemetry object
        if (bytes_read == sizeof(Telemetry))
        {
            Telemetry *t = reinterpret_cast<Telemetry *>(buffer);
            print_telemetry(t);
            writeRadioTelemetry(*t, influxdb, count);
        }

        printf("\n\n");
    }
}
