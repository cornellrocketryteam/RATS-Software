#include <cstdio>
#include <iostream>
#include <cstdlib>

bool read_cat()
{
    // Open a pipe to "cat /dev/ttyACM0"
    FILE *fp = popen("cat /dev/ttyACM0", "r");
    if (!fp)
    {
        std::cerr << "Failed to open /dev/ttyACM0 using cat." << std::endl;
        return false;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        std::cout << buffer;
    }

    pclose(fp);
    return true;
}
