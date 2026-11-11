#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include "Sprite.hpp"

enum class direction : uint8_t {
    Left,
    Right,
    Up,
    Down
};

// Define the TFT pins
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
#define vert A12
#define horz A11
#define SEL_PIN 6

const int pixel_multiplier = 5; // How many pixels is one x and y coordinate in our code?

const int screenSizeX = 320;
const int screenSizeY = 240;

const int heartSizeX = 8;
const int heartSizeY = 8;

// Midpoint of the player is in the middle of the screen
const int heartMidpointX = screenSizeX / 2;
const int heartMidpointY = screenSizeY / 2;

// Supposed to appear in the middle of the screen
const int heartDrawPosX = heartMidpointX - (heartSizeX * pixel_multiplier / 2);
const int heartDrawPosY = heartMidpointY - (heartSizeY * pixel_multiplier / 2);

const int shieldOffsetFromPlayer = 2;

const int shieldSizeX = 10;
const int shieldSizeY = 2;

int x = 85; 
int y = 85;

int W = 7;
int H = 60;

unsigned long lastUpdate = 0;
const int updateInterval = 16;

// Initialize the display
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, 11, 13, TFT_RST, 12);

const Sprite<heartSizeX, heartSizeY> heart = {
    .data =  {
        {0, 1, 1, 0, 0, 1, 1, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
    }
};

const byte shield[shieldSizeX][shieldSizeY] = {
    {1, 1},
    {1, 1},
    {1, 1},
    {1, 1},
    {1, 1},
    {1, 1},
    {1, 1},
    {1, 1},
    {1, 1},
};

template<uint8_t XDim, uint8_t YDim>
void drawSprite(int x, int y, const Sprite<XDim,YDim>& sprite) {
    for (int row = 0; row < XDim; row++) {
        for (int col = 0; col < YDim; col++) {
            int colorID;
            switch(sprite.data[row][col]) {
                case uint8_t(SpriteColor::Black): {
                    colorID = ILI9341_BLACK;
                    break;
                }
                case uint8_t(SpriteColor::Red): {
                    colorID = ILI9341_RED;
                    break;
                }
                case uint8_t(SpriteColor::White): {
                    colorID = ILI9341_WHITE;
                    break;
                }
                default:{
                    colorID = ILI9341_BLACK;
                    break;
                }   
            }
            tft.fillRect(
                x + col * pixel_multiplier,
                y + row * pixel_multiplier,
                pixel_multiplier,
                pixel_multiplier,
                colorID
            );
        }
    }
}

void drawShield(int x, int y) {

    for (int row = 0; row < shieldSizeX; row++) {

        for (int col = 0; col < shieldSizeY; col++) {

            if (shield[row][col] == 1) {

                tft.fillRect(
                    x + col * pixel_multiplier,
                    y + row * pixel_multiplier,
                    5,
                    5,
                    ILI9341_WHITE
                );

            }
        }
    }
}

void setup() {
    Serial.begin(9600);

    pinMode(vert, INPUT);
    pinMode(horz, INPUT);
    pinMode(SEL_PIN, INPUT_PULLUP);

    // Start the display
    tft.begin();

    // Set screen orientation
    tft.setRotation(1);

    tft.fillScreen(ILI9341_BLACK);


}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    int oldX = x;
    int oldY = y;

    int oldW = W;
    int oldH = H;

    int horizontal = analogRead(horz);
    int vertical = analogRead(vert);

    if (horizontal < 400){
        x = 190;
        y = 85;
        W = 7;
        H = 60;
    } 
    if (horizontal > 600){
        x = 85;
        y = 85;
        W = 7;
        H = 60;
    }

    if (vertical > 600){
        x = 110;
        y = 160;
        W = 60;
        H = 7;
    }
    if (vertical < 400){
        x = 110;
        y = 65;
        W = 60;
        H = 7; 
    }

    //x = constrain(x, 0, tft.width() - 40);
    //y = constrain(y, 0, tft.height() - 35);

    if(oldX != x || oldY!= y){
        tft.fillRect(oldX, oldY, oldW, oldH, ILI9341_BLACK);
    }
    drawSprite(heartDrawPosX, heartDrawPosY, heart);
    //drawShield(90, 100);
    tft.fillRect(x, y, W, H, ILI9341_WHITE);
  }
}