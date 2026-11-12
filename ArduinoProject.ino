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

struct ShieldState {
    bool isActive = false;
    Rectangle currentPosition{};
};

// Define the TFT pins
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
#define vert A12
#define horz A11
#define SEL_PIN 6

const int pixel_multiplier = 3; // How many pixels is one x and y coordinate in our code?

const int screenSizeX = 320;
const int screenSizeY = 240;

const int playAreaX = 240;
const int playAreaY = 240;

const int heartSizeX = 8;
const int heartSizeY = 7;

// Midpoint of the player is in the middle of the screen
const int heartMidpointX = screenSizeX / 2;
const int heartMidpointY = screenSizeY / 2;

// Supposed to appear in the middle of the screen
const int heartDrawPosX = heartMidpointX - (heartSizeX * pixel_multiplier / 2);
const int heartDrawPosY = heartMidpointY - (heartSizeY * pixel_multiplier / 2);

// How far the shield should be from the player object.
const int shieldOffsetFromPlayer = 4;

const int shieldSizeX = 2;
const int shieldSizeY = 10;

unsigned long lastUpdate = 0;
const int updateInterval = 16;

//Gamestate variables

// The current position (and bounding box) of the player shield, and a bool determining if it is currently active or not
ShieldState currentShieldState;
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
        {2, 2},
    }
};
Sprite<shieldSizeY,shieldSizeX> shieldHorizontal = shieldVertical.GetRotatedSprite();

Direction getJoystickDirection() {
    int horizontal = analogRead(horz);
    int vertical = analogRead(vert);

    if (horizontal < 400){
        return Direction::Right;
    } 
    else if (horizontal > 600){
        return Direction::Left;
    }

    else if (vertical > 600){
       return Direction::Down;
    }
    else if (vertical < 400){
        return Direction::Up;
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
    tft.fillRect(
                xStart,
                yStart,
                xSize * pixel_multiplier,
                ySize * pixel_multiplier,
                hrwdColor
            );
}

template<uint8_t XDim, uint8_t YDim>
void drawSprite(int x, int y, const Sprite<XDim,YDim>& sprite) {
    for (int row = 0; row < YDim; row++) {
        for (int col = 0; col < XDim; col++) {
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

    tft.fillScreen(ILI9341_WHITE);
    tft.fillRect(heartMidpointX - (playAreaX/2), heartMidpointY - (playAreaY/2), playAreaX, playAreaY, ILI9341_BLACK);


}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // Get direction of the joystick
    Direction joystickDirection = getJoystickDirection();

    if(joystickDirection != lastJoystickDirection) {
        lastJoystickDirection = joystickDirection;
        ShieldState oldShieldState = currentShieldState;
        // If the previous shield state was active, remove it from the display
        if(oldShieldState.isActive) {
            Serial.println(int(joystickDirection));
            drawRectangle(currentShieldState.currentPosition, SoftwareColor::Black);
        }


        switch(joystickDirection) {
        // Set the shield bounding box, calculate the exact coordinates where it should appear and draw it on the screen
        case Direction::Left: {
            int x = (heartDrawPosX) - shieldOffsetFromPlayer * pixel_multiplier;
            int y = (heartDrawPosY) - shieldSizeY / 2;
            currentShieldState.currentPosition = shieldVertical.GetBoundingBox(x, y);
            currentShieldState.isActive = true;
            drawSprite(x, y, shieldVertical);
            break;
        }
        case Direction::Right: {
            int x = (heartDrawPosX + heartSizeX * pixel_multiplier) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeX * pixel_multiplier;
            int y = (heartDrawPosY) - shieldSizeY / 2;
            currentShieldState.currentPosition = shieldVertical.GetBoundingBox(x, y);
            currentShieldState.isActive = true;
            drawSprite(x, y, shieldVertical);
            break;
        }
        case Direction::Up: {
            int x = (heartDrawPosX) - shieldSizeX * pixel_multiplier / 2;
            int y = (heartDrawPosY) - shieldOffsetFromPlayer * pixel_multiplier;
            currentShieldState.currentPosition = shieldHorizontal.GetBoundingBox(x, y);
            currentShieldState.isActive = true;
            drawSprite(x, y, shieldHorizontal);
            break;
        }
        case Direction::Down: {
            int x = (heartDrawPosX) - shieldSizeX * pixel_multiplier / 2;
            int y = (heartDrawPosY + heartSizeY * pixel_multiplier) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeX * pixel_multiplier;
            currentShieldState.currentPosition = shieldHorizontal.GetBoundingBox(x, y);
            currentShieldState.isActive = true;
            drawSprite(x, y, shieldHorizontal);
            break;
        }
        case Direction::None: {
            currentShieldState.isActive = false;
            break;
        }
        }
    
    }

    // Draw the player 
    drawSprite(heartDrawPosX, heartDrawPosY, heart);
  }
}