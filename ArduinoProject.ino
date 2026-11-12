#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include "Sprite.hpp"
#include "Shapes.hpp"

enum class Direction : uint8_t {
    None,
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

const int shieldOffsetFromPlayer = 4;

const int shieldSizeX = 10;
const int shieldSizeY = 2;

int x = 85; 
int y = 85;

int W = 7;
int H = 60;

unsigned long lastUpdate = 0;
const int updateInterval = 16;

//Gamestate variables

// The current position (and bounding box) of the player shield
Rectangle currentShieldPosition;
// Changes to true when the shield has been moved atleast once by the player
bool hasMovedShield = false;
Direction lastJoystickDirection = Direction::None;


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


Sprite<shieldSizeX,shieldSizeY> shieldVertical = {
    .data = {
        {2, 2},
        {2, 2},
        {2, 2},
        {2, 2},
        {2, 2},
        {2, 2},
        {2, 2},
        {2, 2},
        {2, 2},
    }
};
Sprite<shieldSizeY,shieldSizeX> shieldHorizontal = shieldVertical.GetRotatedSprite();

Direction getJoystickDirection() {
    int horizontal = analogRead(horz);
    int vertical = analogRead(vert);

    if (horizontal > 620 && vertical <= 1){
        Serial.println("left");
        return Direction::Left;
    } 
    else if (horizontal < 420 && vertical <= 1){
        Serial.println("Right");
       return Direction::Right;
    }

    else if (vertical < 400 && horizontal < 1020 ){
        Serial.println("Up");
        return Direction::Up;
    }
    else if (vertical > 600 && horizontal < 1020){
        Serial.println("Down");
        return Direction::Down;
    }
    else {
        return Direction::None;
    }
}

void drawRectangle(const Rectangle& rectangle, SoftwareColor color) {
    int xStart = rectangle.topLeftCorner.x;
    int yStart = rectangle.topLeftCorner.y;
    int xEnd = rectangle.bottomRightCorner.x;
    int yEnd = rectangle.bottomRightCorner.y;
    int xSize = xEnd - xStart;
    int ySize = yEnd - yStart;
    int hrwdColor = SoftwareColorToHardwareColor(color);
    //Serial.print(" xstart: ");
    //Serial.print(xStart);
    //Serial.print(" ystart: ");
    //Serial.print(yStart);
    //Serial.print(" xEnd: ");
    //Serial.print(xEnd);
    //Serial.print(" yEnd: ");
    //Serial.print(yEnd);
    //Serial.print(" xsize: ");
    //Serial.print(xSize);
    //Serial.print(" ysize: ");
    //Serial.println(ySize);
    tft.fillRect(
                xStart,
                yStart,
                xSize,
                ySize,
                hrwdColor
            );
}

template<uint8_t XDim, uint8_t YDim>
void drawSprite(int x, int y, const Sprite<XDim,YDim>& sprite) {
    for (int row = 0; row < XDim; row++) {
        for (int col = 0; col < YDim; col++) {
            int colorID = SoftwareColorToHardwareColor(SoftwareColor(sprite.data[row][col]));
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
    // Get direction of the joystick
    Direction joystickDirection = getJoystickDirection();



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
    if(joystickDirection != lastJoystickDirection) {
        lastJoystickDirection = joystickDirection;

        Rectangle oldShieldPos = currentShieldPosition;
        switch(joystickDirection) {
        // Set the shield bounding box, calculate the exact coordinates where it should appear and draw it on the screen
        case Direction::Left: {
            int x = (heartMidpointX - heartSizeX * pixel_multiplier / 2) - shieldOffsetFromPlayer * pixel_multiplier;
            int y = (heartMidpointY - heartSizeY * pixel_multiplier / 2) - shieldSizeY * pixel_multiplier / 2;
            currentShieldPosition = shieldVertical.GetBoundingBox(x, y);
            drawSprite(x, y, shieldVertical);
            break;
        }
        case Direction::Right: {
            int x = (heartMidpointX + heartSizeX * pixel_multiplier / 2) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeY * pixel_multiplier;
            int y = (heartMidpointY - heartSizeY * pixel_multiplier / 2) - shieldSizeY * pixel_multiplier / 2;
            currentShieldPosition = shieldVertical.GetBoundingBox(x, y);
            drawSprite(x, y, shieldVertical);
            break;
        }
        case Direction::Up: {
            int x = (heartMidpointX - heartSizeX * pixel_multiplier / 2) - shieldSizeY * pixel_multiplier / 2;
            int y = (heartMidpointY - heartSizeY * pixel_multiplier / 2) - shieldOffsetFromPlayer * pixel_multiplier;
            currentShieldPosition = shieldHorizontal.GetBoundingBox(x, y);
            drawSprite(x, y, shieldHorizontal);
            break;
        }
        case Direction::Down: {
            int x = (heartMidpointX - heartSizeX * pixel_multiplier / 2) - shieldSizeY * pixel_multiplier / 2;
            int y = (heartMidpointY + heartSizeY * pixel_multiplier / 2) + shieldOffsetFromPlayer * pixel_multiplier;
            currentShieldPosition = shieldHorizontal.GetBoundingBox(x, y);
            drawSprite(x, y, shieldHorizontal);
            break;
        }
        }
        // If player has moved shield before, clear the old shield
        
        drawRectangle(oldShieldPos, SoftwareColor::Black);
        
        hasMovedShield = true;
    }

    // Draw the player 
    drawSprite(heartDrawPosX, heartDrawPosY, heart);
    //drawShield(90, 100);
   // tft.fillRect(x, y, W, H, ILI9341_WHITE);
  }
}