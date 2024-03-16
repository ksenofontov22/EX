#include "ex.h"

Joystick joy;
Graphics gfx;
Cursor crs1;
Shortcut iconSapper, iconGears;
PowerSave pws;
Terminal trm;
UserTerminal <5> userTrm;
TimeNTP timentp;
Keyboard key;

/*void user_gfx_text()
{
    //gfx.print("EX", 239, 10); //u8g2.sendBuffer();
    String inputWord = "";
    key.keyboard();
    //Serial.println(inputWord);
    //inf.message(inputWord, 4500);
    //Serial.println("EX"); 
}

void user_terminal()
{
    userTrm.tick();
}*/

void setup()
{  
    //userTrm.attach(1, user_gfx_text, 10);
    //user_gfx_text();
    gfx.initializationSystem();
    //timentp.setupWifi("free", "");
    //timentp.setupTime();

}

void loop()
{  
    trm.terminal(); 
}