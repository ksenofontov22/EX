/*
  Contains function settings for working with the display.

  [!] Required u8g2 library
  [!] bmp to xbmp image converter https://www.online-utility.org/image/convert/to/XBM
                                  https://windows87.github.io/xbm-viewer-converter/
  [!] midi to arduino tones converter https://arduinomidi.netlify.app/
*/

#include <iostream>
#include <Arduino.h>
#include <U8g2lib.h> 
#include "ex.h"
#include "ex_xbm.h"

//version library
const int8_t VERSION_LIB[] = {1, 0};

Graphics _gfx; Timer _delayCursor, _trm0; Application _app; Joystick _joy; Shortcut _myConsole;
Cursor _crs; PowerSave _pwsDeep; Interface _mess; Button _ok, _no;

/* Prototype function */
void clearCommandTerminal(); void testApp();

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

/* graphics chip setup */
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

const int8_t PIN_BACKLIGHT_LCD = 25;   // gp 
const int8_t PIN_BUZZER = 26;          // gp 

/* backlight */
void Graphics::controlBacklight(bool state)
{
    pinMode(PIN_BACKLIGHT_LCD, OUTPUT);

    if (state == true)
    {
        digitalWrite(PIN_BACKLIGHT_LCD, 1); // on
    }
    else
    {
        digitalWrite(PIN_BACKLIGHT_LCD, 0); // off
    }
}

/* graphics */
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
    u8g2.setContrast(143);
    //setting the resolution of the analog-to-digital converter
    analogReadResolution(RESOLUTION_ADC);
    //display backlight
    pinMode(PIN_BACKLIGHT_LCD, OUTPUT);
    digitalWrite(PIN_BACKLIGHT_LCD, true);
    //PIN mode
    pinMode(PIN_BUTTON_ENTER, INPUT);
    pinMode(PIN_BUTTON_EX, INPUT);
    pinMode(PIN_BUTTON_A, INPUT);
    pinMode(PIN_BUTTON_B, INPUT);

    //platform logo output
    image_width = windows_width;
    image_height = windows_height;
    //--
    u8g2.clearBuffer();
    u8g2.drawXBMP(((W_LCD - image_width)/2), ((H_LCD - image_height)/2) - 7, image_width, image_height, windows_bits); //88 88
    _gfx.print(10, "the experience system", 65, ((H_LCD/2) + (image_height/2) + 7), 10, 6);
    _gfx.print(6, (String)VERSION_LIB[0] + "." + (String)VERSION_LIB[1] , 0, H_LCD, 10, 4);
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

/* print */
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

bool Interface::dialogueMessage(String label, String text, void (*f1)(), void (*f2)()){}

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

/* BUTTON */
/* Button return boolean state */
bool Button::button(String text, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
  uint8_t sizeText = text.length();

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

/* shortcut */
bool Shortcut::shortcut(const uint8_t *bitMap, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  u8g2.drawXBMP(x, y, 32, 32, bitMap);
  u8g2.drawXBMP(x, y + 24, 8, 8, icon_bits);

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

bool Shortcut::shortcut(String name, const uint8_t *bitMap, uint8_t x, uint8_t y, void (*f)(void), int xCursor, int yCursor)
{
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  u8g2.drawXBMP(x, y, 32, 32, bitMap);
  u8g2.drawXBMP(x, y + 24, 8, 8, icon_bits);

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

/* Joystic */
/* system button control */
bool Joystick::pressKeyENTER()
{
    if (digitalRead(PIN_BUTTON_ENTER) == true)
    {
        return true;
    }
    else
        return false;
}

bool Joystick::pressKeyEX()
{
    if (digitalRead(PIN_BUTTON_EX) == true)
    {
        return true;
    }
    else
        return false;
}

bool Joystick::pressKeyA()
{
    if (digitalRead(PIN_BUTTON_A) == true)
    {
        return true;
    }
    else
        return false;
}

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
        if (COOR_Y0 >= 160) COOR_Y0 = 0;
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
        if (COOR_Y0 <= 0) COOR_Y0 = 160;
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

int Joystick::calculatePositionX0() // 0x
{
    RAW_DATA_X0 = analogRead(PIN_STICK_0X);

    if ((RAW_DATA_X0 < (DEF_RES_X0 - 600)) /*&& (RAW_DATA_X0 > (DEF_RES_X0 - 1200))*/)
    {
        COOR_X0 -= 2;
        if (COOR_X0 <= 0) COOR_X0 = 256;
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
        if (COOR_X0 >= 256) COOR_X0 = 0;
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

/* Updating Stick coordinates */
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

/* */
void Joystick::resetPositionXY()
{
    COOR_X0 = 128; COOR_X1 = 128; COOR_Y0 = 80; COOR_Y1 = 80;
}

/* Updating Stick coordinates */
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

/* Calculate position index */
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
void Timer::timer(void (*f)(void), int interval)
{
    unsigned long currTime = millis();
    if (currTime - prevTime >= interval)
    {
        prevTime = currTime;
        f();
    }
}

/* Powersave mode */
/* The function checks whether the joystick or button is pressed at a certain moment */
bool isTouched()
{
  if ((_joy.calculateIndexY0() == 0) && (_joy.calculateIndexX0() == 0)) return false;

  return true;
}
/* Shows a notification about the start of sleep mode */
void sleepModeScreen()
{
    _mess.popUpMessage("PwSM", "Light sleep.\nBye, bye my User!\nUse the Joystick to wake up!\0\0", 1000);
}
/* */
void powerSaveDeepSleep()
{
    if (isTouched() == true)
    {
        screenTiming = TIMER;
    }
    
    if (_joy.posY0 >= 150) BUFFER_STRING = "Light powersave mode";
    
    if ((TIMER - screenTiming > 60000) && (_joy.posY0 >= 150))
    {
        screenTiming = TIMER;

        while (isTouched() == false)
        {
            sleepModeScreen();
            esp_light_sleep_start();
        }
    }
    
    if ((TIMER - screenTiming > 60000) && (_joy.posY0 < 150))
    {
        screenTiming = TIMER;

        while (isTouched() == false)
        {
            esp_deep_sleep_start();
        }
    }
}
/* Turns off the backlight and turns on an infinite loop
   with the text to pause until the joysticks are pressed or moved */
/* Light sleep */
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
/* Deep sleep */
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
void songEngine(uint arr[][2], uint noteCount)
{
  for (uint i = 0; i < noteCount; i++)
  {
    tone(PIN_BUZZER, arr[i][0], arr[i][1]);
    delay(120);
    noTone(PIN_BUZZER);
  }
}
/*  */
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
/*  */
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

/* Application */
void Application::window(String name, void (*f1)(void), void (*f2)(void))
{
    f1; //calculate

    //while (true)
    //{
        //u8g2.clearBuffer();

        _gfx.print(name, 5, 9, 8, 5);
        u8g2.drawFrame(0, 10, 256, 141);

        f2; //render

        //u8g2.sendBuffer();
    //}
}

/* TASK-FUNCTION */
void clearBufferString()
{
    BUFFER_STRING = "";
}
/* System tray */
void systemTray()
{
    u8g2.drawHLine(0, 150, 256);
    _gfx.print(BUFFER_STRING, 5, 159, 8, 5); _trm0.timer(clearBufferString, 100);
    
}
/* System cursor */
void systemCursor()
{
    _joy.updatePositionXY(25);
    _crs.cursor(true, _joy.posX0, _joy.posY0);
}
/* System RawADC */
void systemRawADC()
{
    String text = "Coord X: " + (String)_joy.RAW_DATA_X0 + "Coord Y: " + (String)_joy.RAW_DATA_Y0;
    BUFFER_STRING = text; 
}
/* System viewList */
void systemViewList()
{

}

/* NULL function */
void null(){}

void ff()
{
    _mess.popUpMessage("!", "Ohhh no :(\nTask-function not defined!\0", 5000);
    _joy.resetPositionXY();
}

void ff2()
{
    _mess.popUpMessage("COM port", "A - Ok, B - Cancel" , "Are you sure you want\nto close the task?\0", 5000);
    //_mess.dialogueMessage("COM port", "Are you sure you want\nto close the task?\0");
    _joy.resetPositionXY();
}

/* APP */
/* Desctop */
void desctop()
{
    _gfx.print("Move the cursor\nto the Pong game\nshortcut", 5, 10, 8, 5);
    _myConsole.shortcut("My Console", icon_mytablet_bits, 5, 30, ff, _joy.posX0, _joy.posY0);
    _myConsole.shortcut("Serial port", icon_com_port_bits, 5, 65, ff2, _joy.posX0, _joy.posY0);
    _myConsole.shortcut("Test Application", icon_tech_info_bits, 5, 100, testApp, _joy.posX0, _joy.posY0);
}

/* TERMINAL */
/* command type */
struct App
{
    char const *text;       //command
    char const *name;       //name task-function

    void (*f)(void);        //task-function

    bool active;            //activ status task-function
    int priority;           //execution priority
    const uint8_t *bitMap;  //icon task-function
    bool state;             //0-task-function any 1-Application
};

/* enumeration of objects - commands */
App commands[]
{
    {"clearcomm", "Clear command",  clearCommandTerminal, false,   0, NULL, 0},
    {"desctop",   "Desctop",        desctop,              true,    0, NULL, 1},
    {"deepsleep", "Deep sleep PWS-mode", powerSaveDeepSleep, true, 0, NULL, 0},
    {"rawadc",    "Raw data ADC",   systemRawADC,         false,   0, NULL, 0},
    {"clearbuffer","Clear Buffer",  clearBufferString,    false,   0, NULL, 0},

    {"test","Test",  testApp,    false,   0, NULL, 2},

    {"systray",   "Tray",           systemTray,           true,    0, NULL, 0},
    {"syscursor", "Cursor",         systemCursor,         true,    0, NULL, 0},
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

void testApp()
{
    commands[1].active = false;
    commands[5].active = true;

    _app.window("Test Application", null, null);
    
    if (_joy.pressKeyEX() == true)
    {
        commands[1].active = true;
        commands[5].active = false;
    }
}