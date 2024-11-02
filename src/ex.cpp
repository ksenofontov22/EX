/*
  Contains function settings for working with the display.

  [!] Required u8g2 library
  [!] bmp to xbmp image converter https://www.online-utility.org/image/convert/to/XBM
                                  https://windows87.github.io/xbm-viewer-converter/
  [!] midi to arduino tones converter https://arduinomidi.netlify.app/
      ntp client                  https://github.com/arduino-libraries/NTPClient
*/

#include <iostream>
#include <Arduino.h>
#include <U8g2lib.h> 
#include "ex.h"
#include "ex_xbm.h"

#include <WiFi.h>
#include "NTPClient.h"
#include <WiFiUdp.h>


//version library
const int8_t VERSION_LIB[] = {1, 0};

Graphics _gfx; 
Timer _delayCursor, _trm0, _trm1, _stop, _timerUpdateClock; 
Application _app; 
Joystick _joy; 
Shortcut _myConsole, _wifi;
Cursor _crs; 
PowerSave _pwsDeep; 
Interface _mess; 
Button _ok, _no, _collapse, _expand, _close, _ledControl, _keys;
TimeNTP _timentp; Task _task;
Label _labelClock, _labelBattery, _labelWifi;

/* WIFI */
bool stateWifiSetup = false;
bool stateWifi = false;

/* LED control */
bool systemStateLedControl = true; bool flagStateLedControl = true;

/* Time NTP*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "1.asia.pool.ntp.org", 10800, 60000);

/* Prototype function */
void null();
void clearCommandTerminal(); void testApp(); void myDesctop();
void myWifiConnect(); void myWifiDisconnect(); void sustemLedControl(); void flagLedControl();
void keyboard();


enum StateOs
{
    /* State OS */
    _ON,
    _OFF,
    _PAUSED,
    _SLEEP,
    /* State game */
    _IN_GAME,
    _OFF_GAME,
    _CLOSE_GAME,
    _RESTART_GAME,
    _PAUSED_GAME

};

//for screensaver
unsigned long screenTiming{}, screenTiming2{}, TIMER{};

//buffer -->use enum?
String BUFFER_STRING{};
int BUFFER_INT{};
double BUFFER_DOUBLE{};

//for timer
unsigned long previousMillis{};
unsigned long prevTime_0{};
const long interval{300};

/* Graphics chip setup */
U8G2_ST75256_JLX256160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/5, /* dc=*/17, /* reset=*/16);

/* Liquid crystal display resolution. */
int H_LCD{160}, W_LCD{256};
/* Analog-to-digital converter resolution (Chip PICO 2040). */
const int8_t RESOLUTION_ADC{12};
/* Port data. */
const int8_t PIN_STICK_0X = 33; // adc 
const int8_t PIN_STICK_0Y = 32; // adc 

const int8_t PIN_STICK_1Y = 34; // adc 
const int8_t PIN_STICK_1X = 35; // adc 

const int8_t PIN_BUTTON_ENTER = 27;  // gp 
const int8_t PIN_BUTTON_EX    = 14;  // gp 
const int8_t PIN_BUTTON_A     = 12;  // gp
const int8_t PIN_BUTTON_B     = 13;  // gp

const int8_t PIN_BACKLIGHT_LCD = 0;    // gp 
const int8_t PIN_BUZZER  = 26;         // gp 
const int8_t PIN_BATTERY = 39;         // gp

/* Backlight */
bool Graphics::controlBacklight(bool state) //p-n-p transistor
{
    pinMode(PIN_BACKLIGHT_LCD, OUTPUT);

    if (state == true)
    {
        digitalWrite(PIN_BACKLIGHT_LCD, 0); // on
        return true;
    }
    else
    {
        digitalWrite(PIN_BACKLIGHT_LCD, 1); // off
        return false;
    }
}

/* Graphics */
/* graphics output objects */
void Graphics::initializationSystem()
{
    /* GPIO release from sleep */
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 1);
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 1);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 1); // Stick 0
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); // Stick 0
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 1); // EX button
    
    //setting the operating system state
    //setting display, contrast
    u8g2.begin(); Serial.begin(9600);
    u8g2.setContrast(143); //143//150
    //setting the resolution of the analog-to-digital converter
    analogReadResolution(RESOLUTION_ADC);
    //display backlight
    pinMode(PIN_BACKLIGHT_LCD, OUTPUT);
    //_gfx.controlBacklight(true);
    //PIN mode
    pinMode(PIN_BUTTON_ENTER, INPUT);
    pinMode(PIN_BUTTON_EX,    INPUT);
    pinMode(PIN_BUTTON_A,     INPUT);
    pinMode(PIN_BUTTON_B,     INPUT);
    pinMode(PIN_BATTERY,      INPUT);

    //platform logo output
    image_width = windows_width;
    image_height = windows_height;
    //--
    u8g2.clearBuffer();
    u8g2.drawXBMP(((W_LCD - image_width)/2), ((H_LCD - image_height)/2) - 7, image_width, image_height, windows_bits); //88 88
    _gfx.print(10, "the experience system", 65, ((H_LCD/2) + (image_height/2) + 7), 10, 6);
    _gfx.print(6, (String)VERSION_LIB[0] + "." + (String)VERSION_LIB[1] + BUFFER_STRING, 0, H_LCD, 10, 4);
    u8g2.sendBuffer();
    //--
    delay(2500);
}
/* data render (full frame) */
void Graphics::render(void (*f)(), int timeDelay)
{
    uint32_t time;

    time = millis() + timeDelay;

    do
    {
        u8g2.clearBuffer();
        f();
        u8g2.sendBuffer();
    } while (millis() < time);
}
/* data render (full frame) no time delay */
void Graphics::render(void (*f)())
{
      u8g2.clearBuffer();
      f();
      u8g2.sendBuffer();
}
/* data render two function (full frame) */
void Graphics::render(void (*f1)(), void (*f2)())
{
      u8g2.clearBuffer();
      f1();
      f2();
      u8g2.sendBuffer();
}
/* clearing the output buffer */
void Graphics::clear()
{
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

/* Print */
/* text output with parameters, add size font, add line interval (def: 10) and character interval (def: 6) */
void Graphics::print(int8_t sizeFont, String text, int x, int y, int8_t lii, int8_t chi) // text, x-position, y-position, line interval (8-10), character interval (4-6)
{
    int sizeText = text.length() + 1;
    int yy{0};

    //micro, 6, 7, 8, 10, 12, 13
    if (sizeFont == 5) u8g2.setFont(u8g2_font_micro_tr);
    else if (sizeFont == 6) u8g2.setFont(u8g2_font_4x6_tr);
    else if (sizeFont == 7) u8g2.setFont(u8g2_font_5x7_tr);
    else if (sizeFont == 8) u8g2.setFont(u8g2_font_5x8_tr);
    else if (sizeFont == 10) u8g2.setFont(u8g2_font_6x10_tr);
    else if (sizeFont == 12) u8g2.setFont(u8g2_font_6x12_tr);
    else if (sizeFont == 13) u8g2.setFont(u8g2_font_6x13_tr);
    else u8g2.setFont(u8g2_font_6x10_tr); //default

    for (int i = 0, xx = 0; i < sizeText, xx < (sizeText * chi); i++, xx += chi)
    {
        u8g2.setCursor(xx + x, yy + y);
        u8g2.print(text[i]);

        if (text[i] == '\n')
        {
            yy += lii; // 10
            xx = -chi; // 6
        }
    }
}
/* text output with parameters, add line interval (def: 10) and character interval (def: 6) */
void Graphics::print(String text, int x, int y, int8_t lii, int8_t chi) // text, x-position, y-position, line interval (8-10), character interval (4-6)
{
    int sizeText = text.length() + 1;
    int yy{0};

    u8g2.setFont(u8g2_font_6x10_tr);

    for (int i = 0, xx = 0; i < sizeText, xx < (sizeText * chi); i++, xx += chi)
    {
        u8g2.setCursor(xx + x, yy + y);
        u8g2.print(text[i]);

        if (text[i] == '\n')
        {
            yy += lii; // 10
            xx = -chi; // 6
        }
    }
}
/* text output with parameters */
void Graphics::print(String text, int x, int y) // text, x-position, y-position, line interval (8-10), character interval (4-6)
{
    int8_t lii{10}, chi{6};
    int sizeText = text.length() + 1;
    int yy{0};

    u8g2.setFont(u8g2_font_6x10_tr);

    for (int i = 0, xx = 0; i < sizeText, xx < (sizeText * chi); i++, xx += chi)
    {
        u8g2.setCursor(xx + x, yy + y);
        u8g2.print(text[i]);

        if (text[i] == '\n')
        {
            yy += lii; // 10
            xx = -chi; // 6
        }
    }
}
/* "wink text" output  */
bool Graphics::winkPrint(void (*f)(String, int, int), String text, int x, int y, int interval)
{
    unsigned long currTime = millis();
    if (currTime - prevTime_0 >= interval)
    {
        prevTime_0 = currTime;
        return 0;
    }
    else
    {
        f(text, x, y);
        return 1;
    }
}

/* Cursor */
/* displaying the cursor on the screen */
bool Cursor::cursor(bool stateCursor, int xCursor, int yCursor)
{
    if (stateCursor == true)
    {
        u8g2.setDrawColor(2);
        u8g2.setBitmapMode(1);
        u8g2.drawXBMP(xCursor, yCursor, cursor_w, cursor_h, cursor_bits);
        u8g2.setDrawColor(1);
        u8g2.setBitmapMode(0);
        return true;
    }
    else
        return false;
}

/* Interface */
/* displaying a message to the user */
void Interface::message(String text, int duration)
{
    uint8_t x{10}, y{34};
    
    /* Counting the number of lines in a message
       Line break is supported - '\n' */   
    int sizeText = text.length() + 1, line{};

    for (int i = 0; i < sizeText; i++)
    {
        if (text[i] == '\n')
        {
          line++;
        }
    }

    uint8_t correction = (line * 10)/2;
    
    u8g2.clearBuffer();
    for (int i = 0; i < W_LCD; i += 2)
    {
        for (int j = 0; j < (y - 10) - correction; j += 2)
        {
            u8g2.drawPixel(i, j);
        }

        for (int k = y + ((line) * 10) + 4 - correction /* correction */; k < H_LCD; k += 2)
        {
            u8g2.drawPixel(i, k);
        }
    }

    _gfx.print(text, x, y - correction, 10, 6);
    u8g2.sendBuffer();

    delay(duration);
}
/* displaying a message to the user */
void Interface::popUpMessage(String label, String text, uint tDelay)
{
    uint8_t sizeText = text.length();

    uint8_t countLine{1}, countChar{0}, maxChar{}, h_frame{}, border{5}, a{}; 

    for (int i = 0; i <= sizeText; i++)
    {
        if (text[i] != '\0')
        {
            ++countChar;

            if (text[i] == '\n')
            {
                countLine++;

                if ((text[i] == '\n') && (countChar > maxChar))
                {
                    maxChar = countChar;
                    countChar = 0;
                }
            }

            if ((text[i] == '\0') || (text[i+1] == '\0'))
            {
                if (countChar > maxChar)
                {
                    maxChar = countChar;
                    countChar = 0;
                }
            }
            if (countChar > maxChar) maxChar = countChar;
        }
    }
    
    h_frame = countLine * 10; a = h_frame/2;

    u8g2.clearBuffer();
    u8g2.drawFrame(((W_LCD/2)-(maxChar*6)/2) - border, (H_LCD/2) - a - border, (maxChar * 6) + (border * 2), h_frame + (border * 2));
    u8g2.drawFrame(((W_LCD/2)-(maxChar*6)/2) - (border + 3), (H_LCD/2) - a - (border + 3), (maxChar * 6) + ((border + 3) * 2), h_frame + ((border + 3) * 2));

    _gfx.print(label, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) - a - (border + 4));
    _gfx.print(text, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) - a + 10);
    u8g2.sendBuffer();

    delay(tDelay);

    //debug
    /*Serial.println(maxChar); Serial.println(sizeText);
    for (int i = 0; i <= sizeText; i++)
    {
        Serial.println(text[i], BIN);
    }*/
}
/* displaying a message to the user */
void Interface::popUpMessage(String label1, String label2, String text, uint tDelay)
{
    uint8_t sizeText = text.length();

    uint8_t countLine{1}, countChar{0}, maxChar{}, h_frame{}, border{5}, a{}; 

    for (int i = 0; i <= sizeText; i++)
    {
        if (text[i] != '\0')
        {
            ++countChar;

            if (text[i] == '\n')
            {
                countLine++;

                if ((text[i] == '\n') && (countChar > maxChar))
                {
                    maxChar = countChar;
                    countChar = 0;
                }
            }

            if ((text[i] == '\0') || (text[i+1] == '\0'))
            {
                if (countChar > maxChar)
                {
                    maxChar = countChar;
                    countChar = 0;
                }
            }
            if (countChar > maxChar) maxChar = countChar;
        }
    }
    
    h_frame = countLine * 10; a = h_frame/2;

    u8g2.clearBuffer();
    u8g2.drawFrame(((W_LCD/2)-(maxChar*6)/2) - border, (H_LCD/2) - a - border, (maxChar * 6) + (border * 2), h_frame + (border * 2));
    u8g2.drawFrame(((W_LCD/2)-(maxChar*6)/2) - (border + 3), (H_LCD/2) - a - (border + 3), (maxChar * 6) + ((border + 3) * 2), h_frame + ((border + 3) * 2));

    _gfx.print(label1, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) - a - (border + 4));
    _gfx.print(text, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) - a + 10);
    _gfx.print(label2, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) + a + (border + 11));

    u8g2.sendBuffer();

    delay(tDelay);

    //debug
    /*Serial.println(maxChar); Serial.println(sizeText);
    for (int i = 0; i <= sizeText; i++)
    {
        Serial.println(text[i], BIN);
    }*/
}
/* displaying a message to the user */
void Interface::popUpMessage(String label, String text)
{
    uint8_t sizeText = text.length();

    uint8_t countLine{1}, countChar{0}, maxChar{}, h_frame{}, border{5}, a{}; 

    for (int i = 0; i <= sizeText; i++)
    {
        if (text[i] != '\0')
        {
            ++countChar;

            if (text[i] == '\n')
            {
                countLine++;

                if ((text[i] == '\n') && (countChar > maxChar))
                {
                    maxChar = countChar;
                    countChar = 0;
                }
            }

            if ((text[i] == '\0') || (text[i+1] == '\0'))
            {
                if (countChar > maxChar)
                {
                    maxChar = countChar;
                    countChar = 0;
                }
            }
            if (countChar > maxChar) maxChar = countChar;
        }
    }
    
    h_frame = countLine * 10; a = h_frame/2;

    xBorder = 128 /* W_LCD/2 */ - (maxChar*6)/2;
    yBorder = 80  /* H_LCD/2 */ + a + (border + 11);

    u8g2.clearBuffer();
    u8g2.drawFrame(((W_LCD/2)-(maxChar*6)/2) - border, (H_LCD/2) - a - border, (maxChar * 6) + (border * 2), h_frame + (border * 2));
    u8g2.drawFrame(((W_LCD/2)-(maxChar*6)/2) - (border + 3), (H_LCD/2) - a - (border + 3), (maxChar * 6) + ((border + 3) * 2), h_frame + ((border + 3) * 2));

    _gfx.print(label, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) - a - (border + 4));
    _gfx.print(text, (W_LCD/2)-(maxChar*6)/2, (H_LCD/2) - a + 10);
    u8g2.sendBuffer();

    //Serial.println(max); Serial.println(countChar);
}
/* displaying the dialog to the user */
bool Interface::dialogueMessage(String label, String text, void (*f1)(), void (*f2)()){}
/* displaying the dialog to the user */
bool Interface::dialogueMessage(String label, String text)
{
    while (true)
    {
        uint8_t sizeText = text.length();

        uint8_t countLine{1}, countChar{0}, maxChar{}, h_frame{}, border{5}, a{};        
        
        for (int i = 0; i <= sizeText; i++)
        {
            if (text[i] != '\0')
            {
                ++countChar;

                if (text[i] == '\n')
                {
                    countLine++;

                    if ((text[i] == '\n') && (countChar > maxChar))
                    {
                        maxChar = countChar;
                        countChar = 0;
                    }
                }

                if ((text[i] == '\0') || (text[i + 1] == '\0'))
                {
                    if (countChar > maxChar)
                    {
                        maxChar = countChar;
                        countChar = 0;
                    }
                }
                if (countChar > maxChar)
                    maxChar = countChar;
            }
        }

        h_frame = countLine * 10;
        a = h_frame / 2;

        xBorder = 128 /* W_LCD/2 */ - (maxChar * 6) / 2;
        yBorder = 80  /* H_LCD/2 */ + a + (border + 11);

        u8g2.clearBuffer();
        u8g2.drawFrame(((W_LCD / 2) - (maxChar * 6) / 2) - border, (H_LCD / 2) - a - border, (maxChar * 6) + (border * 2), h_frame + (border * 2));
        u8g2.drawFrame(((W_LCD / 2) - (maxChar * 6) / 2) - (border + 3), (H_LCD / 2) - a - (border + 3), (maxChar * 6) + ((border + 3) * 2), h_frame + ((border + 3) * 2));

        _gfx.print(label, (W_LCD / 2) - (maxChar * 6) / 2, (H_LCD / 2) - a - (border + 4));
        _gfx.print(text, (W_LCD / 2) - (maxChar * 6) / 2, (H_LCD / 2) - a + 10);

        //_ok.button(" OK ", xBorder - 8, yBorder + 2, _joy.posX0, _joy.posY0);
        //_no.button(" NO ", xBorder + 20, yBorder + 2, _joy.posX0, _joy.posY0);

        if (_ok.button(" OK ", 128 - 26, yBorder + 2, _joy.posX0, _joy.posY0))
        {
            return true;
            break;
        }
        if (_no.button(" NO ", 130, yBorder + 2, _joy.posX0, _joy.posY0))
        {
            return false;
            break;
        }

        _joy.updatePositionXY(25);
        _crs.cursor(true, _joy.posX0, _joy.posY0);
        u8g2.sendBuffer();
    }
}

/* Button */
/* Button return boolean state */
bool Button::button(String text, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
  uint8_t sizeText = text.length();

  if ((xCursor >= x && xCursor <= (x + (sizeText * 5) + 4)) && (yCursor >= y - 8 && yCursor <= y + 2))
  {
    u8g2.setDrawColor(1);
    u8g2.drawBox(x, y - 8, (sizeText * 5) + 5, 10);

    if (Joystick::pressKeyENTER() == true)
    {
      f();
      return true;
    }
  }
  else
  {
    u8g2.setDrawColor(1);
    u8g2.drawFrame(x, y - 8, (sizeText * 5) + 5, 10);
  }

  u8g2.setCursor(x + 3, y);
  u8g2.setFont(u8g2_font_profont10_mr);
  u8g2.setFontMode(1);
  u8g2.setDrawColor(2);
  u8g2.print(text);
  u8g2.setFontMode(0);
  
  return false;
}
/* Button return boolean state */
bool Button::button(String text, uint8_t x, uint8_t y, uint8_t xCursor, uint8_t yCursor)
{
  uint8_t sizeText = text.length();

  if ((xCursor >= x && xCursor <= (x + (sizeText * 5) + 4)) && (yCursor >= y - 8 && yCursor <= y + 2))
  {
    u8g2.setDrawColor(1);
    u8g2.drawBox(x, y - 8, (sizeText * 5) + 5, 10);

    if (Joystick::pressKeyENTER() == true)
    {
      return true;
    }
  }
  else
  {
    u8g2.setDrawColor(1);
    u8g2.drawFrame(x, y - 8, (sizeText * 5) + 5, 10);
  }

  u8g2.setCursor(x + 3, y);
  u8g2.setFont(u8g2_font_profont10_mr);
  u8g2.setFontMode(1);
  u8g2.setDrawColor(2);
  u8g2.print(text);
  u8g2.setFontMode(0);
  
  return false;
}

bool Button::buttonForKeyboard(int sizeFont, char symbol, uint8_t x, uint8_t y, void (*f)(void), uint8_t xCursor, uint8_t yCursor)
{
  uint8_t sizeText = 1;

  if ((xCursor >= x && xCursor <= (x + (sizeText * 5) + 4)) && (yCursor >= y - 8 && yCursor <= y + 2))
  {
    u8g2.setDrawColor(1);
    u8g2.drawRBox(x, y - 8, (sizeText * 5) + 5, 10, 2);

    if (Joystick::pressKeyENTER() == true)
    {
      f();
      return true;
    }
  }
  else
  {
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(x, y - 8, (sizeText * 5) + 5, 10, 2);
  }

  u8g2.setCursor(x + 3, y);
  if (sizeFont == 5) u8g2.setFont(u8g2_font_micro_tr);
  else if (sizeFont == 6) u8g2.setFont(u8g2_font_4x6_tr);
  else if (sizeFont == 7) u8g2.setFont(u8g2_font_5x7_tr);
  else if (sizeFont == 8) u8g2.setFont(u8g2_font_5x8_tr);
  else if (sizeFont == 10) u8g2.setFont(u8g2_font_6x10_tr);
  else if (sizeFont == 12) u8g2.setFont(u8g2_font_6x12_tr);
  else if (sizeFont == 13) u8g2.setFont(u8g2_font_6x13_tr);
  else u8g2.setFont(u8g2_font_6x10_tr);
  //u8g2.setFont(u8g2_font_profont10_mr);
  u8g2.setFontMode(1);
  u8g2.setDrawColor(2);
  u8g2.print((String)symbol);
  u8g2.setFontMode(0);
  
  return false;
}

/* Shortcut */
/* displaying a shortcut to a task-function */
bool Shortcut::shortcut(const uint8_t *bitMap, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  u8g2.drawXBMP(x, y, 32, 32, bitMap);
  //u8g2.drawXBMP(x, y + 24, 8, 8, icon_bits);
  u8g2.drawXBMP(x, y + 21, 11, 11, shortcut_bits);

  if ((xCursor >= x && xCursor <= (x + 32)) && (yCursor >= y && yCursor <= (y + 32)))
  {
    u8g2.drawFrame(x, y, 32, 32);
    if (Joystick::pressKeyENTER() == true)
    {
      f();
      return true;
    }
  }

  return false;
}
/* displaying a shortcut to a task-function */
bool Shortcut::shortcut(String name, const uint8_t *bitMap, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  u8g2.drawXBMP(x, y, 32, 32, bitMap);
  //u8g2.drawXBMP(x, y + 24, 8, 8, icon_bits);
  u8g2.drawXBMP(x, y + 21, 11, 11, shortcut_bits);

  if ((xCursor >= x && xCursor <= (x + 32)) && (yCursor >= y && yCursor <= (y + 32)))
  {
    u8g2.drawFrame(x, y, 32, 32);
    
    BUFFER_STRING = name;
    
    if (Joystick::pressKeyENTER() == true)
    {
      f();
      return true;
    }
  }
  else
  {
    
  }

  return false;
}
/* displaying a shortcut to a task-function */
bool Shortcut::shortcutFrame(String name, uint8_t w, uint8_t h, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
    u8g2.setDrawColor(1);
    u8g2.setBitmapMode(0);

    //u8g2.drawXBMP(x, y + 24, 8, 8, icon_bits);

    if ((xCursor >= x && xCursor <= (x + w)) && (yCursor >= y && yCursor <= (y + h)))
    {
        u8g2.drawFrame(x, y, w, h);

        BUFFER_STRING = name;

        if (Joystick::pressKeyENTER() == true)
        {
            f();
            return true;
        }
    }
    else
    {
    }

    return false;
}

/* Label */
/*  */
bool Label::label(String text, uint8_t x, uint8_t y, void (*f)(void), uint8_t lii, uint8_t chi, int xCursor, int yCursor)
{
    /*u8g2.setDrawColor(1);
    u8g2.setBitmapMode(0);

    uint8_t w{},h{}; 

    uint8_t sizeText = text.length();
    w = sizeText * chi;
    h = lii;

    _gfx.print(text, x, y, lii, chi);

    if ((xCursor >= x && xCursor <= (x + w)) && (yCursor >= y && yCursor <= (y + h)))
    {
        u8g2.drawFrame(x, y, w, h);

        BUFFER_STRING = text;

        if (Joystick::pressKeyENTER() == true)
        {
            f();
            return true;
        }
    }
    else
    {
    }*/

    uint8_t sizeText = text.length();
    uint8_t yy{};

    if ((xCursor >= x && xCursor <= (x + (sizeText * chi))) && (yCursor >= y - (lii + 2) && yCursor <= y + 2))
    {
        u8g2.setDrawColor(1);
        u8g2.drawBox(x - 1, y - (lii), (sizeText * chi) + 2, lii + 1);

        BUFFER_STRING = text;

        if (Joystick::pressKeyENTER() == true)
        {
            f();
            return true;
        }
    }
    else
    {
        u8g2.setDrawColor(1);
    }

    u8g2.setCursor(x + 3, y);
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setFontMode(1);
    u8g2.setDrawColor(2);
    
    for (int i = 0, xx = 0; i < sizeText, xx < (sizeText * chi); i++, xx += chi)
    {
        u8g2.setCursor(xx + x, yy + y);
        u8g2.print(text[i]);

        if (text[i] == '\n')
        {
            yy += lii; // 10
            xx = -chi; // 6
        }
    }

    u8g2.setFontMode(0);

    return false;
}
/*  */
bool Label::label(String text, String description, uint8_t x, uint8_t y, void (*f)(void), uint8_t lii, uint8_t chi, int xCursor, int yCursor)
{
    uint8_t sizeText = text.length();
    uint8_t yy{};

    if ((xCursor >= x && xCursor <= (x + (sizeText * chi))) && (yCursor >= y - (lii + 2) && yCursor <= y + 2))
    {
        u8g2.setDrawColor(1);
        u8g2.drawBox(x - 1, y - (lii), (sizeText * chi) + 2, lii + 1);

        BUFFER_STRING = description;

        if (Joystick::pressKeyENTER() == true)
        {
            f();
            return true;
        }
    }
    else
    {
        u8g2.setDrawColor(1);
    }

    u8g2.setCursor(x + 3, y);
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setFontMode(1);
    u8g2.setDrawColor(2);
    
    for (int i = 0, xx = 0; i < sizeText, xx < (sizeText * chi); i++, xx += chi)
    {
        u8g2.setCursor(xx + x, yy + y);
        u8g2.print(text[i]);

        if (text[i] == '\n')
        {
            yy += lii; // 10
            xx = -chi; // 6
        }
    }

    u8g2.setFontMode(0);

    return false;
}

/* Joystic */
/* button control */
bool Joystick::pressKeyENTER()
{
    if (digitalRead(PIN_BUTTON_ENTER) == true)
    {
        return true;
    }
    else
        return false;
}
/* button control */
bool Joystick::pressKeyEX()
{
    if (digitalRead(PIN_BUTTON_EX) == true)
    {
        return true;
    }
    else
        return false;
}
/* button control */
bool Joystick::pressKeyA()
{
    if (digitalRead(PIN_BUTTON_A) == true)
    {
        return true;
    }
    else
        return false;
}
/* button control */
bool Joystick::pressKeyB()
{
    if (digitalRead(PIN_BUTTON_B) == true)
    {
        return true;
    }
    else
        return false;
}
/* calculate Stick position */
int Joystick::calculatePositionY0() // 0y
{
    RAW_DATA_Y0 = analogRead(PIN_STICK_0Y);

    if ((RAW_DATA_Y0 < (DEF_RES_Y0 - 600)) /*&& (RAW_DATA_Y0 > (DEF_RES_Y0 - 1100))*/)
    {
        COOR_Y0 += 2;
        if (COOR_Y0 >= 160) COOR_Y0 = 160;
        return COOR_Y0;
    }
    /*else if (RAW_DATA_Y0 < (DEF_RES_Y0 - 1100))
    {
        COOR_Y0 -= 2;
        if (COOR_Y0 <= 0) COOR_Y0 = 160;
        return COOR_Y0;
    }*/
    else if ((RAW_DATA_Y0 > (DEF_RES_Y0 + 600)) /*&& (RAW_DATA_Y0 < (DEF_RES_Y0 + 1100))*/)
    {
        COOR_Y0 -= 2;
        if (COOR_Y0 <= 0) COOR_Y0 = 0;
        return COOR_Y0;
    }
   /*else if (RAW_DATA_Y0 > (DEF_RES_Y0 + 1100))
    {
        COOR_Y0 += 2;
        if (COOR_Y0 >= 160) COOR_Y0 = 0;
        return COOR_Y0;
    }*/
    else
        return COOR_Y0;
}
/* calculate Stick position */
int Joystick::calculatePositionY1() // 1y
{
    RAW_DATA_Y1 = analogRead(PIN_STICK_1Y);

    if ((RAW_DATA_Y1 < (DEF_RES_Y1 - 500)) && (RAW_DATA_Y1 > (DEF_RES_Y1 - 1100)))
    {
        COOR_Y1 -= 1;
        if (COOR_Y1 <= 0) COOR_Y1 = 160;
        return COOR_Y1;
    }
    else if (RAW_DATA_Y1 < (DEF_RES_Y1 - 1100))
    {
        COOR_Y1 -= 2;
        if (COOR_Y1 <= 0) COOR_Y1 = 160;
        return COOR_Y1;
    }
    else if ((RAW_DATA_Y1 > (DEF_RES_Y1 + 500)) && (RAW_DATA_Y1 < (DEF_RES_Y1 + 1100)))
    {
        COOR_Y1 += 1;
        if (COOR_Y1 >= 160) COOR_Y1 = 0;
        return COOR_Y1;
    }
    else if (RAW_DATA_Y1 > (DEF_RES_Y1 + 1100))
    {
        COOR_Y1 += 2;
        if (COOR_Y1 >= 160) COOR_Y1 = 0;
        return COOR_Y1;
    }
    else
        return COOR_Y1;
}
/* calculate Stick position */
int Joystick::calculatePositionX0() // 0x
{
    RAW_DATA_X0 = analogRead(PIN_STICK_0X);

    if ((RAW_DATA_X0 < (DEF_RES_X0 - 600)) /*&& (RAW_DATA_X0 > (DEF_RES_X0 - 1200))*/)
    {
        COOR_X0 -= 2;
        if (COOR_X0 <= 0) COOR_X0 = 0;
        return COOR_X0; 

    }
    /*else if (RAW_DATA_X0 < (DEF_RES_X0 - 1100))
    {
        COOR_X0 += 2;
        if (COOR_X0 >= 256) COOR_X0 = 0;
        return COOR_X0;

    }*/
    else if ((RAW_DATA_X0 > (DEF_RES_X0 + 600)) /*&& (RAW_DATA_X0 < (DEF_RES_X0 + 1100))*/)
    {
        COOR_X0 += 2;
        if (COOR_X0 >= 256) COOR_X0 = 256;
        return COOR_X0; 
    }
    /*else if (RAW_DATA_X0 > (DEF_RES_X0 + 1100))
    {
        COOR_X0 -= 2;
        if (COOR_X0 <= 0) COOR_X0 = 256;
        return COOR_X0;
    }*/
    else
        return COOR_X0;
}
/* calculate Stick position */
int Joystick::calculatePositionX1() // 1x
{
    RAW_DATA_X1 = analogRead(PIN_STICK_1X);

    if ((RAW_DATA_X1 < (DEF_RES_X1 - 500)) && (RAW_DATA_X1 > (DEF_RES_X1 - 1100)))
    {
        COOR_X1 += 1;
        if (COOR_X1 >= 256) COOR_X1 = 0;
        return COOR_X1;
    }
    else if (RAW_DATA_X1 < (DEF_RES_X1 - 1100))
    {
        COOR_X1 += 2;
        if (COOR_X1 >= 256) COOR_X1 = 0;
        return COOR_X1;
    }
    else if ((RAW_DATA_X1 > (DEF_RES_X1 + 500)) && (RAW_DATA_X1 < (DEF_RES_X1 + 1100)))
    {
        COOR_X1 -= 1;
        if (COOR_X1 <= 0) COOR_X1 = 256;
        return COOR_X1;
    }
    else if (RAW_DATA_X1 > (DEF_RES_X1 + 1100))
    {
        COOR_X1 -= 2;
        if (COOR_X1 <= 0) COOR_X1 = 256;
        return COOR_X1;
    }
    else
        return COOR_X1;
}
/* updating Stick coordinates */
void Joystick::updatePositionXY()
{
    posX0 = calculatePositionX0(); //
    posX1 = calculatePositionX1();
    posY0 = calculatePositionY0(); //
    posY1 = calculatePositionY1();

    indexX0 = calculateIndexX0();
    indexX1 = calculateIndexX1();
    indexY0 = calculateIndexY0();
    indexY1 = calculateIndexY1();
}
/* resetting the cursor position */
void Joystick::resetPositionXY()
{
    COOR_X0 = 128; COOR_X1 = 128; COOR_Y0 = 80; COOR_Y1 = 80;
}
/* updating Stick coordinates */
void Joystick::updatePositionXY(uint delay)
{
    unsigned long currTime = millis();
    if (currTime - prevTime >= delay)
    {
        prevTime = currTime;

        posX0 = calculatePositionX0(); //
        posX1 = calculatePositionX1();
        posY0 = calculatePositionY0(); //
        posY1 = calculatePositionY1();

        indexX0 = calculateIndexX0();
        indexX1 = calculateIndexX1();
        indexY0 = calculateIndexY0();
        indexY1 = calculateIndexY1();
    }
}
/* calculate position index */
int8_t Joystick::calculateIndexY0() // obj 0y
{
    RAW_DATA_Y0 = analogRead(PIN_STICK_0Y);

    if ((RAW_DATA_Y0 < (DEF_RES_Y0 - 500)) && (RAW_DATA_Y0 > (DEF_RES_Y0 - 1100)))
    {
        return OBJ_Y0 = OBJ_Y0 - 1;
    }
    else if (RAW_DATA_Y0 < (DEF_RES_Y0 - 1100))
    {
        return OBJ_Y0 = OBJ_Y0 - 1; // 2
    }
    else if ((RAW_DATA_Y0 > (DEF_RES_Y0 + 500)) && (RAW_DATA_Y0 < (DEF_RES_Y0 + 1100)))
    {
        return OBJ_Y0 = OBJ_Y0 + 1;
    }
    else if (RAW_DATA_Y0 > (DEF_RES_Y0 + 1100))
    {
        return OBJ_Y0 = OBJ_Y0 + 1; // 2
    }
    else
        return OBJ_Y0 = 0;
}
/* calculate position index */
int8_t Joystick::calculateIndexY1() // obj 1y
{
    RAW_DATA_Y1 = analogRead(PIN_STICK_1Y);

    if ((RAW_DATA_Y1 < (DEF_RES_Y1 - 500)) && (RAW_DATA_Y1 > (DEF_RES_Y1 - 1100)))
    {
        return OBJ_Y1 = OBJ_Y1 - 1;
    }
    else if (RAW_DATA_Y1 < (DEF_RES_Y1 - 1100))
    {
        return OBJ_Y1 = OBJ_Y1 - 1; // 2
    }
    else if ((RAW_DATA_Y1 > (DEF_RES_Y1 + 500)) && (RAW_DATA_Y1 < (DEF_RES_Y1 + 1100)))
    {
        return OBJ_Y1 = OBJ_Y1 + 1;
    }
    else if (RAW_DATA_Y1 > (DEF_RES_Y1 + 1100))
    {
        return OBJ_Y1 = OBJ_Y1 + 1; // 2
    }
    else
        return OBJ_Y1 = 0;
}
/* calculate position index */
int8_t Joystick::calculateIndexX0() // obj 0x
{
    RAW_DATA_X0 = analogRead(PIN_STICK_0X);

    if ((RAW_DATA_X0 < (DEF_RES_X0 - 500)) && (RAW_DATA_X0 > (DEF_RES_X0 - 1100)))
    {
        return OBJ_X0 = OBJ_X0 - 1;
    }
    else if (RAW_DATA_X0 < (DEF_RES_X0 - 1100))
    {
        return OBJ_X0 = OBJ_X0 - 1; // 2
    }
    else if ((RAW_DATA_X0 > (DEF_RES_X0 + 500)) && (RAW_DATA_X0 < (DEF_RES_X0 + 1100)))
    {
        return OBJ_X0 = OBJ_X0 + 1;
    }
    else if (RAW_DATA_X0 > (DEF_RES_X0 + 1100))
    {
        return OBJ_X0 = OBJ_X0 + 1; // 2
    }
    else
        return OBJ_X0 = 0;
}
/* calculate position index */
int8_t Joystick::calculateIndexX1() // obj 1x
{
    RAW_DATA_X1 = analogRead(PIN_STICK_1X);

    if ((RAW_DATA_X1 < (DEF_RES_X1 - 500)) && (RAW_DATA_X1 > (DEF_RES_X1 - 1100)))
    {
        return OBJ_X1 = OBJ_X1 - 1;
    }
    else if (RAW_DATA_X1 < (DEF_RES_X1 - 1100))
    {
        return OBJ_X1 = OBJ_X1 - 1; // 2
    }
    else if ((RAW_DATA_X1 > (DEF_RES_X1 + 500)) && (RAW_DATA_X1 < (DEF_RES_X1 + 1100)))
    {
        return OBJ_X1 = OBJ_X1 + 1;
    }
    else if (RAW_DATA_X1 > (DEF_RES_X1 + 1100))
    {
        return OBJ_X1 = OBJ_X1 + 1; // 2
    }
    else
        return OBJ_X1 = 0;
}

/* Timer */
/* starting a task-function with an interval */
void Timer::timer(void (*f)(void), int interval)
{
    unsigned long currTime = millis();
    if (currTime - prevTime >= interval)
    {
        prevTime = currTime;
        f();
    }
}
/* starting a task-function with an interval */
void Timer::timer(int (*f)(void), int interval)
{
    unsigned long currTime = millis();
    if (currTime - prevTime >= interval)
    {
        prevTime = currTime;
        f();
    }
}
/* ---> remove support */
void Timer::stopwatch(void (*f)(void), int interval)
{
    unsigned long currTime = millis();
    if (currTime - prevTime >= interval)
    {
        prevTime = currTime;
        f();
    }
}

/* Powersave mode */
/* the function checks whether the joystick or button is pressed at a certain moment */
bool isTouched()
{
  if ((_joy.calculateIndexY0() == 0) && (_joy.calculateIndexX0() == 0)) return false;

  return true;
}
/* shows a notification about the start of sleep mode */
void sleepModeScreen()
{
    _mess.popUpMessage("PwSM", "Light sleep.\nBye, bye my User!\nUse the Joystick to wake up!\0\0", 1000);
}
/* a system-task for working in an energy-efficient mode */
void powerSaveDeepSleep()
{
    if (isTouched() == true)
    {
        screenTiming = TIMER;
    }
    
    if ((_joy.posY0 >= 150) && (_joy.posX0 <= 100)) BUFFER_STRING = "Light powersave mode";
    
    if ((TIMER - screenTiming > 60000) && (_joy.posY0 >= 150))
    {
        screenTiming = TIMER;

        while (isTouched() == false)
        {
            sleepModeScreen();
            _gfx.controlBacklight(false);
            esp_light_sleep_start(); 
        }
    }
    
    if ((TIMER - screenTiming > 60000) && (_joy.posY0 < 150))
    {
        screenTiming = TIMER;

        while (isTouched() == false)
        {
            _gfx.controlBacklight(false);
            esp_deep_sleep_start();

        }
    }
}
/* turns off the backlight and turns on an infinite loop
   with the text to pause until the joysticks are pressed or moved */
/* ---> remove support */
void PowerSave::sleepLight(bool state, uint timeUntil)
{
  if ((state == true))
  {
    if (isTouched() == true)
    {
      screenTiming = millis();
    }

    if (millis() - screenTiming > timeUntil)
    {
      screenTiming = millis();

      digitalWrite(PIN_BACKLIGHT_LCD, false);

      while (isTouched() == false)
      {
        /* Sleep */
        _gfx.render(sleepModeScreen, 500);
        //esp_deep_sleep_start();
        esp_light_sleep_start();
      }

      digitalWrite(PIN_BACKLIGHT_LCD, true);
    }
  }
}
/* ---> remove support */
void PowerSave::sleepDeep(bool state, uint timeUntil)
{
  if ((state == true))
  {
    if (isTouched() == true)
    {
      screenTiming = TIMER;
    }
    else screenTiming = screenTiming;


    if (TIMER - screenTiming > timeUntil)
    {
      screenTiming = TIMER;

      digitalWrite(PIN_BACKLIGHT_LCD, false);

      while (isTouched() == false)
      {
        /* Sleep */
        esp_deep_sleep_start();
      }

      digitalWrite(PIN_BACKLIGHT_LCD, true);
    }
  }
}

/* Song engine */
/* playing a melody */
void songEngine(uint arr[][2], uint noteCount)
{
  for (uint i = 0; i < noteCount; i++)
  {
    tone(PIN_BUZZER, arr[i][0], arr[i][1]);
    delay(120);
    noTone(PIN_BUZZER);
  }
}
/* list melody */
void Melody::songCore()
{
    switch (lM)
    {
    case listMelody::None:
        break;

    // Melody
    case listMelody::MakeGame:
        songEngine(songMakeGame, 7);
        lM = None;
        break;
    
    // Bang
    case listMelody::Bang1:
        songEngine(songBang1, 2);
        lM = None;
        break;

    case listMelody::Bang2:
        songEngine(songBang2, 2);
        lM = None;
        break;

    case listMelody::Bang3:
        songEngine(songBang3, 2);
        lM = None;
        break;

    case listMelody::Bang4:
        songEngine(songBang4, 2);
        lM = None;
        break;

    case listMelody::Bang5:
        songEngine(songBang5, 2);
        lM = None;
        break;

    // Tone
    case listMelody::Tone1:
        songEngine(songTone1, 1);
        lM = None;
        break;

    case listMelody::Tone2:
        songEngine(songTone2, 1);
        lM = None;
        break;

    case listMelody::Tone3:
        songEngine(songTone3, 1);
        lM = None;
        break;

    case listMelody::Tone4:
        songEngine(songTone4, 1);
        lM = None;
        break;

    case listMelody::Tone5:
        songEngine(songTone5, 1);
        lM = None;
        break;

    // UI
    // --

    default:
        break;
    }
}
/* list melody */
void Melody::song(listMelody num)
{
    switch (num)
    {
    case listMelody::None:
        lM = None;
        break;
    
    // Melody
    case listMelody::MakeGame:
        lM = MakeGame;
        break;

    // Bang
    case listMelody::Bang1:
        lM = Bang1;
        break;

    case listMelody::Bang2:
        lM = Bang2;
        break;

    case listMelody::Bang3:
        lM = Bang3;
        break;

    case listMelody::Bang4:
        lM = Bang4;
        break;

    case listMelody::Bang5:
        lM = Bang5;
        break;
    
    // Tones
    case listMelody::Tone1:
        lM = Tone1;
        break;

    case listMelody::Tone2:
        lM = Tone2;
        break;

    case listMelody::Tone3:
        lM = Tone3;
        break;

    case listMelody::Tone4:
        lM = Tone4;
        break;

    case listMelody::Tone5:
        lM = Tone5;
        break;

    // UI
    case listMelody::Ok:
        lM = Ok;
        break;

    case listMelody::Cancel:
        lM = Cancel;
        break;

    case listMelody::Error:
        lM = Error;
        break;

    case listMelody::Click:
        lM = Click;
        break;

    default:
        break;
    }
}

/* System task-function */
/* task-function. clear buffer */
void clearBufferString()
{
    BUFFER_STRING = "";
}
/* task-function. calculation of battery capacity */
int dataRawBattery{};
int systemUpdateBattery()
{
    dataRawBattery = analogRead(PIN_BATTERY);
    dataRawBattery = map(dataRawBattery, 1861, 2481, 0, 100);

    return dataRawBattery;
}
/* update NTP time */
void systemNTPTimeUpdate()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        timeClient.update();
    }
    else _mess.popUpMessage("!", "The Wi-Fi (internet) connection\nis not active.", 2500);
}
/* task-function. */
int _t{};
int systemBattery()
{
    if (_t >= 30000) _t = 30000;
    _timerUpdateClock.timer(systemUpdateBattery, _t); _t += 10000;

    return dataRawBattery;
}
/* task-function. system tray output */
void systemTray()
{
    u8g2.setDrawColor(1);
    u8g2.drawHLine(0, 150, 256);
    
    _gfx.print(BUFFER_STRING, 5, 159, 8, 5);
    _labelBattery.label((String)systemBattery(), "Click to LED on/off", 196, 159, flagLedControl, 8, 5, _joy.posX0, _joy.posY0);
    _labelClock.label((String)timeClient.getFormattedTime(), "Click to update time", 211, 159, systemNTPTimeUpdate, 8, 5, _joy.posX0, _joy.posY0);
    //_gfx.print((String)systemBattery(), 196, 159, 8, 5);


    _trm0.timer(clearBufferString, 100); //clear text-buffer
}
/* task-function. system cursor output */
void systemCursor()
{
    _joy.updatePositionXY(20);
    _crs.cursor(true, _joy.posX0, _joy.posY0);
}
/* task-function. system RawADC */
void systemRawADC()
{
    String text = "Coord X: " + (String)_joy.RAW_DATA_X0 + "Coord Y: " + (String)_joy.RAW_DATA_Y0;
    BUFFER_STRING = text; 
}
/* task-function. displaying a list of tasks */
void systemViewList()
{

}
/* task-function. stack, task, command */
void myConsole()
{
    _mess.popUpMessage("!", "Ohhh no :(\nTask-function not defined!\0", 5000);
    _joy.resetPositionXY();
}
/* task-function. serial port operation control */
void mySerialPort()
{
    _mess.popUpMessage("COM port", "A - Ok, B - Cancel" , "Are you sure you want\nto close the task?\0", 5000);
    //_mess.dialogueMessage("COM port", "Are you sure you want\nto close the task?\0");
    _joy.resetPositionXY();
}
/* task-function. system LED control */

void flagLedControl()
{
    for (int i = 0; i < 1; i++)
    {
        if (flagStateLedControl == true)
        {
            flagStateLedControl = false; delay(250); break;
        }
        else
            flagStateLedControl = true; delay(250);
    }
}

void sustemLedControl()
{

    //_ledControl.button("LED", 5, 140, flagLedControl, _joy.posX0, _joy.posY0);
    
    if (flagStateLedControl == true) _gfx.controlBacklight(true);
    else _gfx.controlBacklight(false);
}

/* NULL function */
void null(){}

/* Terminal */
/* command type */
struct App
{
    char const *text;       //command
    char const *name;       //name task-function

    void (*f)(void);        //task-function

    bool active;            //activ status task-function
    int indexTask;          //
    const uint8_t *bitMap;  //icon task-function
    uint8_t state;          //0-task-function any 1-desctop any 2-app
};
/* enumeration of objects - commands */
App commands[]
{
    //system task
    {"clearcomm",   "Clear command",       clearCommandTerminal, false,     0, NULL, 0},
    {"deepsleep",   "Deep sleep PWS-mode", powerSaveDeepSleep,   true,      1, NULL, 0},
    {"rawadc",      "Raw data ADC",        systemRawADC,         false,     2, NULL, 0},
    {"clearbuffer", "Clear Buffer",        clearBufferString,    false,     3, NULL, 0},

    //app-desctop
    {"mydesctop",   "My Desctop",          myDesctop,            true,    100, NULL,                  1},
    //app
    {"myconsole",   "My Console",          myConsole,            false,   101, iconMyConsole_bits,    2},
    {"myserialport","My Serial port",      mySerialPort,         false,   102, iconMySerialPort_bits, 2},
    {"testapp",     "Test Application",    testApp,              false,   103, iconMyNullApp_bits,    2},
    {"mywifi",      "My WiFi",             myWifiConnect,        false,   104, iconMyWiFiClient_bits, 2},
    
    

    //system graphics-task
    {"keyboard",      "Keyboard",            keyboard,             true,    298, NULL, 0},
    {"sysledcontrol", "LED control",         sustemLedControl,     true,    299, NULL, 0},
    {"systray",       "Tray",                systemTray,           true,    300, NULL, 0},
    {"syscursor",     "Cursor",              systemCursor,         true,    301, NULL, 0},
};
/* delete all commands */
void clearCommandTerminal()
{
  for (App &command : commands)
  {
    command.active = false;
  }
}
/* command stack */
void calcTerminal()
{
  for (App &command : commands)
  {
    if (command.active)
    {
      command.f();
    }
  }
}
/* pushing data onto the stack */
void Terminal::terminal()
{
  TIMER = millis();
  
  _gfx.render(calcTerminal);

  if (Serial.available() != 0)
  {
    char text[20]{};
    Serial.readBytesUntil('\n', text, sizeof(text));

    for (App &command : commands)
    {
      if (not strncmp(command.text, text, 20))
      {
        command.active = true;
      }
    }
  }
}
/* pushing data onto the stack and User task-function */
void Terminal::terminal(void(*f)())
{
  TIMER = millis();
  
  _gfx.render(calcTerminal, f);

  if (Serial.available() != 0)
  {
    char text[20]{};
    Serial.readBytesUntil('\n', text, sizeof(text));

    for (App &command : commands)
    {
      if (not strncmp(command.text, text, 20))
      {
        if (command.active == true) command.active = false;
        else command.active = true;
      }
    }
  }
}

/* Task management */
/* disable a task */
void Task::taskKill(int indexTask)
{
    for (App &command : commands)
    {
        if ((command.active == true) && (command.indexTask == indexTask))
        {
            command.active = false;
        }
    }
}
/* start a task */
void Task::taskRun(int indexTask)
{
    for (App &command : commands)
    {
        if ((command.active == false) && (command.indexTask == indexTask))
        {
            command.active = true;
        }
    }
}

/* Application */
/* window designer for the task-function */
void Application::window(String name, int indexTask, void (*f1)(void), void (*f2)(void))
{
    _task.taskKill(100); //kill Desctop
    _task.taskRun(indexTask);
    
    f1; //calc
    f2; //graphics
    
    //draw window
    {
        _gfx.print(name, 5, 9, 8, 5); u8g2.setDrawColor(1);
        u8g2.drawFrame(0, 10, 256, 141);
    }
    //draw button-state-window
    {
        if (_collapse.button(" COLLAPSE ", 162, 9, _joy.posX0, _joy.posY0)) {}
        
        if (_close.button(" CLOSE ", 216, 9, _joy.posX0, _joy.posY0))
        {
            _task.taskKill(indexTask);
            _task.taskRun(100); //run Desctop
        }
    }
}

/* App */
/* desctop */
void myDesctop()
{
    uint8_t border{4};
    uint8_t xx{border};
    uint8_t yy{15}; 

    uint8_t countTask{1};
    
    for (App &command : commands)
    {
        if (command.state == 2)
        {
            _myConsole.shortcut(command.name, command.bitMap, xx, yy, command.f, _joy.posX0, _joy.posY0);
            countTask++;

            xx += (32 + border);

            if (countTask > 7) 
            {
                xx = 4; yy += (32 + border); countTask = 0;
            }
        }
    }
    
    _gfx.print("My Desctop", 5, 8, 8, 5);
    u8g2.drawHLine(0, 10, 256);

    

    /*test led*/ //_gfx.controlBacklight(true);
}
/* test */
void testApp()
{
    _app.window("Test Application", 103, null, null);
}
/* wi-fi */
void myWifiDisconnect()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    //_mess.popUpMessage("!", "Wi-Fi is disabled!\0", 2000);
    
    _task.taskKill(104);
    stateWifiSetup = false; stateWifi = false;
}
/* wi-fi */
void myWifiConnect()
{
    _task.taskRun(104);
    
    {
        //add network selection
    }

    if (stateWifiSetup == false)
    {
        WiFi.begin("Allowed", "Serjant1985"); stateWifiSetup = true;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        BUFFER_STRING = "Wi-Fi...";
        stateWifi = false;
        _stop.stopwatch(myWifiDisconnect, 10000);
    }
    else
    {
        stateWifi = true;
        _labelWifi.label(WiFi.localIP().toString(), "Click to disconnect", 130, 159, myWifiDisconnect, 8, 5, _joy.posX0, _joy.posY0);
        //_gfx.print(WiFi.localIP().toString(), 130, 159, 8, 5);
        //_disconnect.button("X", 115, 158, myWifiDisconnect, _joy.posX0, _joy.posY0);
    }

    /* IPAddress ip = WiFi.localIP();
    sprintf(lcdBuffer, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], udpPort);*/
}


/*Keyboard*/

/*global variables of the keyboard*/
int allKeyboards[4][30] = { {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'y', 'v', 'w', 'x', 'y', 'z', ' ', ' ', ' ', ' '},
                            {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'Y', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' '},
                            {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                            {'.', ',', '!', '?', '+', '-', '*', '/', ':', ';', '{', '}', '(', ')', '[', ']', '%', '#', '@', '~', '_', '|', '№', '<', '>', '`', ' ', ' ', ' ', ' '} };
int recentKeyboard = 0, symbolsRow = 0;
String wordFromKeyboard = "", inputWord = "";
bool isKeyboardActive;
char SYMBOL = ' ';

/*Add symbol from key to word*/

void printKeyValue()
{
    wordFromKeyboard += SYMBOL;
    SYMBOL = ' ';
    delay(200);
}

/*Changes the visible symbols*/
void chageSymbolsRowLeft()
{
    if (symbolsRow >= 1) { symbolsRow--; }
    delay(150);
}
void chageSymbolsRowRight()
{
    if ((symbolsRow == 0 || symbolsRow == 1 || symbolsRow == 3) && symbolsRow < 2) { symbolsRow++; }
    delay(150);
}

/*Change the keyboard layout*/
void changeKeyboardType1()
{
    if (recentKeyboard == 0) { recentKeyboard = 1; }
    else { recentKeyboard = 0; }
    symbolsRow = 0;
    delay(190);
}
void changeKeyboardType2()
{
    recentKeyboard = 2;
    symbolsRow = 0;
    delay(150);
}
void changeKeyboardType3()
{
    recentKeyboard = 3;
    symbolsRow = 0;
    delay(150);
}

/*Function for delete button*/
void deleteSymbol()
{
    wordFromKeyboard.remove(wordFromKeyboard.length() - 1);
    delay(150);
}

/*Function for space button*/
void spaceSymbol()
{
    wordFromKeyboard += " ";
    delay(150);
}

/*Assigns the entered word to the one we return and ends the endless keyboard loop*/
void Enter()
{
    isKeyboardActive = 0;
    inputWord = wordFromKeyboard;
    wordFromKeyboard = "";
    _crs.cursor(false, _joy.posX0, _joy.posY0);
    delay(150);
}

/*Function for keyboard rendering*/
void showKeyboard()
{
    // add x and y to locate keyboard
    int x = 63;
    _keys.button("Aa", 0+x, 127, changeKeyboardType1, _joy.posX0, _joy.posY0);
    _keys.button("123", 18+x, 127, changeKeyboardType2, _joy.posX0, _joy.posY0);
    _keys.button("?!&", 41+x, 127, changeKeyboardType3, _joy.posX0, _joy.posY0);
    _keys.button("spc", 64+x, 127, spaceSymbol, _joy.posX0, _joy.posY0);
    _keys.button("ent", 87+x, 127, Enter, _joy.posX0, _joy.posY0);
    _keys.button("<--", 111+x, 127, deleteSymbol, _joy.posX0, _joy.posY0);
    //----------------------------------------------
    _keys.button("<", 0+x, 138, chageSymbolsRowLeft, _joy.posX0, _joy.posY0);
    //----------------------------------------------

    for (int j = 0; j <= 9; j++)
    {
        SYMBOL = allKeyboards[recentKeyboard][j + 10*symbolsRow];
        if (SYMBOL != ' ')
        {
            _keys.buttonForKeyboard(8, allKeyboards[recentKeyboard][j + 10*symbolsRow], 11+x + 11*j, 138, printKeyValue, _joy.posX0, _joy.posY0); 
        }
    }

    //----------------------------------------------
    _keys.button(">", 121+x, 138, chageSymbolsRowRight, _joy.posX0, _joy.posY0);
    //----------------------------------------------
    _gfx.print(wordFromKeyboard, 30+x, 107);
}

/*starts the endless cycle, returns entered word*/
void keyboard()
{
    isKeyboardActive = 1;
    while (isKeyboardActive)
    {
        showKeyboard(); break;
    }
    //return inputWord;
}











