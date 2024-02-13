#include "ex.h"

Joystick joy;
Graphics gfx;
Cursor crs1;
Shortcut iconSapper, iconGears;
PowerSave pws;
Terminal trm;
UserTerminal <5> userTrm;

void printSerial()
{
    gfx.print("EX", 239, 10);
    Serial.println("Test"); userTrm.tick();
}

void setup()
{
    userTrm.attach(0, printSerial, 0);
    userTrm.attach(1, NULL, 0);

    gfx.initializationSystem();
}

void loop()
{
    trm.terminal();
}