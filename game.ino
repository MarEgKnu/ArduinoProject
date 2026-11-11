#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Define the TFT pins
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
#define vert A0
#define horz A1
#define SEL_PIN 6

int x = 85; 
int y = 85;

unsigned long lastUpdate = 0;
const int updateInterval = 16;dasd

// Initialize the display
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

const byte heart[8][8] = {
    {0, 1, 1, 0, 0, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
};
const byte shield[10][2] = {
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

void drawHeart(int x, int y) {

    for (int row = 0; row < 8; row++) {

        for (int col = 0; col < 8; col++) {

            if (heart[row][col] == 1) {

                tft.fillRect(
                    x + col * 5,
                    y + row * 5,
                    5,
                    5,
                    ILI9341_RED
                );

            }
        }
    }
}

void drawShield(int x, int y) {

    for (int row = 0; row < 10; row++) {

        for (int col = 0; col < 2; col++) {

            if (shield[row][col] == 1) {

                tft.fillRect(
                    x + col * 5,
                    y + row * 5,
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

    int horizontal = analogRead(horz);
    int vertical = analogRead(vert);

    if (horizontal < 400) x = 190;
    if (horizontal > 600) x = 85;

    if (vertical < 400) y+=5;
    if (vertical > 600) y-=5;

    //x = constrain(x, 0, tft.width() - 40);
    //y = constrain(y, 0, tft.height() - 35);

if(oldX != x || oldY!= y){
    tft.fillRect(oldX, oldY, 7, 60, ILI9341_BLACK);
}
    drawHeart(120, 100);
    //drawShield(90, 100);
    tft.fillRect(x, 85, 7, 60, ILI9341_WHITE);
  }
}
