#include "stepper.h"

class Mover {
public:
    void init();
    void move(float rockElev, float rockLat, float rockLong);

private:
    stepper_t stepper;
    bool launched = false;
    bool recPacket = false;
    long launchTime = 0;
    int stepPos = 0;
};