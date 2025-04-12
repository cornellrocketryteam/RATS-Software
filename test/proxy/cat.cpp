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
#include <unistd.h>
#include <errno.h>

#include "telemetry.hpp"
#include "database.hpp"
#include "util.hpp"

void drain_pipeline(FILE *fp);
void loop(FILE *fp, std::unique_ptr<influxdb::InfluxDB> &influxdb);
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


    // Open a pipe to "cat /dev/ttyACM0"
    FILE *fp = popen("cat /dev/ttyACM0", "r");
    if (!fp)
    {
        std::cerr << "Failed to open /dev/ttyACM0 using cat." << std::endl;
        return false;
    }

    drain_pipeline(fp);
    loop(fp, influxdb);
    pclose(fp);
    return true;
}

int loop_byte_by_byte(FILE *fp, char *buffer, float timeout_ms)
{
    int fd = fileno(fp);

    // Set up a buffer for one byte

    int num_bytes = 0;
    while (true)
    {
        // Set up the file descriptor set.
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // Set up the timeout.
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = ((int)(timeout_ms * 10) % (1000*10))/10 * 1000;

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

void loop(FILE *fp, std::unique_ptr<influxdb::InfluxDB> &influxdb)
{
    const int WAIT_MS = 1;
    // TODO: Make reading and writing happen in different threads
    const int BUFFER_SIZE = sizeof(Telemetry);
    char buffer[BUFFER_SIZE + 4];
    static int count = 0;
    while (!feof(fp))
    {
        // size_t bytes_read = fread(buffer, 1, sizeof(buffer), fp);
        // if (bytes_read == 0)
        //     break; // End-of-file or error

        const float WAIT_MS = .5;
        size_t bytes_read = loop_byte_by_byte(fp, buffer, WAIT_MS);
    
        if (bytes_read == 0) continue;

        printf("Read %d bytes\n", bytes_read);
        printf("Count: %d\n", count++);

        // Process only if we got a complete Telemetry object
        if (bytes_read == sizeof(Telemetry))
        {
            Telemetry *t = reinterpret_cast<Telemetry *>(buffer + 4);
            print_telemetry(t);
            writeRadioTelemetry(*t, influxdb);
        }

        printf("\n\n");
    }
}

void drain_pipeline(FILE *fp)
{
    // Get the underlying file descriptor and set it to non-blocking
    int fd = fileno(fp);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    // Drain any buffered old data
    char drain_buf[256];
    while (true)
    {
        ssize_t n = read(fd, drain_buf, sizeof(drain_buf));
        if (n < 0)
        {
            // No more data available when errno is EAGAIN or EWOULDBLOCK
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("Finished draining\n");
                break;
            }
            else
            {
                std::cerr << "Error during drain: " << strerror(errno) << std::endl;
                break;
            }
        }
        else if (n == 0)
        {
            printf("Done with drain\n");
            break;
        }
        else
        {
            printf("Drained %zd bytes\n", n);
            // Continue draining; do not break here.
        }
    }

    // Restore the file descriptor to its original blocking mode
    fcntl(fd, F_SETFL, flags);
    printf("\n\n");
}

// bool read_cat()
// {

//     const char *port = "/dev/ttyACM0";
//     int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
//     if (fd < 0)
//     {
//         std::cerr << "Error opening " << port << std::endl;
//         return 1;
//     }

//     struct termios tty;
//     memset(&tty, 0, sizeof tty);
//     if (tcgetattr(fd, &tty) != 0)
//     {
//         std::cerr << "Error from tcgetattr" << std::endl;
//         close(fd);
//         return 1;
//     }

//     // Set baud rate to 115200
//     cfsetospeed(&tty, B115200);
//     cfsetispeed(&tty, B115200);

//     // Configure 8N1 mode (8 data bits, no parity, 1 stop bit)
//     tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
//     tty.c_cflag |= (CLOCAL | CREAD);
//     tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);

//     // Set raw input (non-canonical mode) and disable flow control
//     tty.c_lflag = 0;
//     tty.c_oflag = 0;
//     tty.c_iflag &= ~(IXON | IXOFF | IXANY);

//     // Set read timeout (VTIME is in deciseconds)
//     tty.c_cc[VMIN] = 0;
//     tty.c_cc[VTIME] = 5; // 0.5 seconds

//     if (tcsetattr(fd, TCSANOW, &tty) != 0)
//     {
//         std::cerr << "Error from tcsetattr" << std::endl;
//         close(fd);
//         return 1;
//     }

//     // Flush any buffered data from before we started reading.
//     tcflush(fd, TCIFLUSH);

//     const int BUFFER_SIZE = sizeof(Telemetry);
//     char buffer[BUFFER_SIZE];

//     while (true)
//     {
//         ssize_t n = read(fd, buffer, BUFFER_SIZE);
//         if (n < 0)
//         {
//             std::cerr << "Error reading from serial port" << std::endl;
//             break;
//         }
//         if (n == 0)
//         {
//             // No new data available; you might sleep a bit to avoid busy-waiting.
//             usleep(100000); // 100ms
//             continue;
//         }
//         printf("Read %zd bytes\n", n);
//         if (n == sizeof(Telemetry))
//         {
//             Telemetry *t = reinterpret_cast<Telemetry *>(buffer);
//             print_telemetry(t);
//         }
//     }

//     close(fd);
//     return 0;
// }
