#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Sprite.hpp"
#include "Shapes.hpp"

enum class Direction : uint8_t {
  None,
  Left,
  Right,
  Up,
  Down
};

enum class BulletDirection : uint8_t {
  Left = 0,
  Right = 1,
  Up = 2,
  Down = 3
};

enum class BulletType : uint8_t {
    Normal,
    SlowHighDamage,
    FastLowDamage,
    NoBlock
};

struct ShieldState {
  bool isActive = false;
  Rectangle currentPosition{};
};

struct Bullet {
  Vector16 position;
  BulletDirection direction;
  BulletType type;
  bool isActive;
};

// Define the TFT pins
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define vert A12
#define horz A11
#define SEL_PIN 6

const int pixel_multiplier = 3;  // How many pixels is one x and y coordinate in our code?

const int screenSizeX = 320;
const int screenSizeY = 240;

const int playAreaX = 240;
const int playAreaY = 240;

const int playAreaStartX = 40;
const int playAreaEndX = 200;

const int playAreaStartY = 0;
const int playAreaEndY = 240;

const int heartSizeX = 8;
const int heartSizeY = 7;

const int bulletSizeX = 5;
const int bulletSizeY = 5;

// Midpoint of the player is in the middle of the screen
const int heartMidpointX = screenSizeX / 2;
const int heartMidpointY = screenSizeY / 2;

// Supposed to appear in the middle of the screen
const int heartDrawPosX = heartMidpointX - (heartSizeX * pixel_multiplier / 2);
const int heartDrawPosY = heartMidpointY - (heartSizeY * pixel_multiplier / 2);

int bulletDrawPosX;
int bulletDrawPosY;

int oldBulletPosX;
int oldBulletPosY;

// How far the shield should be from the player object.
const int shieldOffsetFromPlayer = 6;

const int shieldSizeX = 2;
const int shieldSizeY = 10;

unsigned long lastUpdate = 0;
const int updateInterval = 16;


//bidimensional array to keep track of active bullets. [x][0] is bullet position, [x][1] is bullet direction, [x][2] is if the bullet is active or not
Bullet bullets[8];

// last new bullet
unsigned long lastBullet;

// last bullet position update
unsigned long lastBulletUpdate;

// speed of bullet spawn and bullet speed (we use interrupt to tweak this)
int bulletRate = 750;
int bulletSpeed = 100;

// self explanatory
int bulletDirection;


//Gamestate variables

// current shield position. i set start value to 999 so it doesnt default to 0 (blocks left bullets without doing anything at start)
int shieldPos = 999;

// how many hits the player can take. can be changed from menu eventually
int health = 1;

int counter;

int bulletArray = 0;

// The current position (and bounding box) of the player shield, and a bool determining if it is currently active or not
ShieldState currentShieldState;
Direction lastJoystickDirection = Direction::None;


// Initialize the display
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, 11, 13, TFT_RST, 12);

const Sprite<heartSizeX, heartSizeY> heart = {
  .data = {
    { 0, 1, 1, 0, 0, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0 },
  }
};

const Sprite<bulletSizeX, bulletSizeY> bulletNormal = {
  .data = {
    { 0, 0, 2, 0, 0 },
    { 0, 2, 2, 2, 0 },
    { 2, 2, 2, 2, 2 },
    { 0, 2, 2, 2, 0 },
    { 0, 0, 2, 0, 0 },
  }
};

const Sprite<bulletSizeX, bulletSizeY> bulletNoBlock = {
  .data = {
    { 0, 0, 3, 0, 0 },
    { 0, 3, 3, 3, 0 },
    { 3, 3, 3, 3, 3 },
    { 0, 3, 3, 3, 0 },
    { 0, 0, 3, 0, 0 },
  }
};

const Sprite<bulletSizeX, bulletSizeY> bulletSlowHighDamage = {
  .data = {
    { 0, 0, 1, 0, 0 },
    { 0, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 0, 0, 1, 0, 0 },
  }
};
const Sprite<bulletSizeX, bulletSizeY> bulletFastLowDamage = {
  .data = {
    { 0, 0, 4, 0, 0 },
    { 0, 4, 4, 4, 0 },
    { 4, 4, 4, 4, 4 },
    { 0, 4, 4, 4, 0 },
    { 0, 0, 4, 0, 0 },
  }
};

const Sprite<bulletSizeX, bulletSizeY>& GetBulletTypeSprite(BulletType type) {
    switch(type) {
        case BulletType::Normal:
            return bulletNormal;
        case BulletType::NoBlock:
            return bulletNoBlock;
        case BulletType::SlowHighDamage:
            return bulletSlowHighDamage;
        case BulletType::FastLowDamage:
            return bulletFastLowDamage;
    }
}

Sprite<shieldSizeX, shieldSizeY> shieldVertical = {
  .data = {
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
  }
};
Sprite<shieldSizeY, shieldSizeX> shieldHorizontal = shieldVertical.GetRotatedSprite();

Direction getJoystickDirection() {
  int horizontal = analogRead(horz);
  int vertical = analogRead(vert);

  if (horizontal < 400) {
    return Direction::Right;
  } else if (horizontal > 600) {
    return Direction::Left;
  }

  else if (vertical > 600) {
    return Direction::Down;
  } else if (vertical < 400) {
    return Direction::Up;
  } else {
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
    hrwdColor);
}

template<uint8_t XDim, uint8_t YDim>
void drawSprite(int x, int y, const Sprite<XDim, YDim>& sprite) {
  for (int row = 0; row < YDim; row++) {
    for (int col = 0; col < XDim; col++) {
      int colorID = SoftwareColorToHardwareColor(SoftwareColor(sprite.data[row][col]));
      tft.fillRect(
        x + col * pixel_multiplier,
        y + row * pixel_multiplier,
        pixel_multiplier,
        pixel_multiplier,
        colorID);
    }
  }
}

void updateBulletPosition(Bullet& bullet) {
    switch(bullet.direction) {
        case BulletDirection::Left:
            bullet.position.x++;
            break;
        case BulletDirection::Right:
            bullet.position.x--;
            break;
        case BulletDirection::Up:
            bullet.position.y++;
            break;
        case BulletDirection::Down:
            bullet.position.y--;
            break;
    }
}

Bullet InitBullet(BulletDirection direction, BulletType type) {
    Vector16 position;
    switch(direction) {
        case BulletDirection::Left:
            position.x = playAreaStartX;
            position.y = heartMidpointY - (bulletSizeY * pixel_multiplier / 2);
            break;
        case BulletDirection::Right:
            position.x = playAreaEndX - bulletSizeX; // offset since it draws towards the right
            position.y = heartMidpointY - (bulletSizeY * pixel_multiplier / 2);
            break;
        case BulletDirection::Up:
            position.x = heartMidpointX - (bulletSizeX * pixel_multiplier / 2);
            position.y = playAreaStartY;
            break;
        case BulletDirection::Down:
            position.x = heartMidpointX - (bulletSizeX * pixel_multiplier / 2);
            position.y = playAreaEndY - bulletSizeY;
            break;
    }
    return Bullet{position, direction, type, true};
}

void counterDisplay(int number) {
  tft.fillRect(240, 0, 40, 40, ILI9341_BLACK);
  tft.setCursor(240, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print(number);
}

void setup() {
  Serial.begin(9600);

  // rng for bullet direction and bullet type
  randomSeed(analogRead(A3));

  pinMode(vert, INPUT);
  pinMode(horz, INPUT);
  pinMode(SEL_PIN, INPUT_PULLUP);

  // Start the display
  tft.begin();

  // Set screen orientation
  tft.setRotation(1);

  tft.fillScreen(ILI9341_WHITE);
  tft.fillRect(heartMidpointX - (playAreaX / 2), heartMidpointY - (playAreaY / 2), playAreaX, playAreaY, ILI9341_BLACK);
  tft.fillRect(20, 30, 10, 10, ILI9341_RED);
  tft.fillRect(280, 30, 10, 10, ILI9341_RED);
}

void loop() {

  while (health > 0) {

    if (millis() - lastUpdate >= updateInterval) {
      lastUpdate = millis();

      // Get direction of the joystick
      Direction joystickDirection = getJoystickDirection();

      if (joystickDirection != lastJoystickDirection) {
        lastJoystickDirection = joystickDirection;
        ShieldState oldShieldState = currentShieldState;
        // If the previous shield state was active, remove it from the display
        if (oldShieldState.isActive) {
          drawRectangle(currentShieldState.currentPosition, SoftwareColor::Black);
        }


        switch (joystickDirection) {
          // Set the shield bounding box, calculate the exact coordinates where it should appear and draw it on the screen
          case Direction::Left:
            {
              int x = (heartDrawPosX)-shieldOffsetFromPlayer * pixel_multiplier;
              int y = (heartDrawPosY)-shieldSizeY / 2;
              currentShieldState.currentPosition = shieldVertical.GetBoundingBox(x, y);
              currentShieldState.isActive = true;
              drawSprite(x, y, shieldVertical);
              shieldPos = int(BulletDirection::Left);
              break;
            }
          case Direction::Right:
            {
              int x = (heartDrawPosX + heartSizeX * pixel_multiplier) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeX * pixel_multiplier;
              int y = (heartDrawPosY)-shieldSizeY / 2;
              currentShieldState.currentPosition = shieldVertical.GetBoundingBox(x, y);
              currentShieldState.isActive = true;
              drawSprite(x, y, shieldVertical);
              shieldPos = int(BulletDirection::Right);
              break;
            }
          case Direction::Up:
            {
              int x = (heartDrawPosX)-shieldSizeX * pixel_multiplier / 2;
              int y = (heartDrawPosY)-shieldOffsetFromPlayer * pixel_multiplier;
              currentShieldState.currentPosition = shieldHorizontal.GetBoundingBox(x, y);
              currentShieldState.isActive = true;
              drawSprite(x, y, shieldHorizontal);
              shieldPos = int(BulletDirection::Up);
              break;
            }
          case Direction::Down:
            {
              int x = (heartDrawPosX)-shieldSizeX * pixel_multiplier / 2;
              int y = (heartDrawPosY + heartSizeY * pixel_multiplier) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeX * pixel_multiplier;
              currentShieldState.currentPosition = shieldHorizontal.GetBoundingBox(x, y);
              currentShieldState.isActive = true;
              drawSprite(x, y, shieldHorizontal);
              shieldPos = int(BulletDirection::Down);
              break;
            }
          case Direction::None:
            {
              currentShieldState.isActive = false;
              shieldPos = 999; // placeholder value
              break;
            }
        }
      }
      static bool test = false;

      // //spawns new bullet after millis exceeds bulletrate. assign direction, type and array to it
      /*if (millis() - lastBullet >= bulletRate) {
        BulletDirection bulletDirection = BulletDirection(random(0, 4));
        bullets[bulletArray] = InitBullet(bulletDirection, BulletType::Normal);
                                                                      // 1 = active. 0 = inactive (skips the for loop below)
        bulletArray = (bulletArray + 1) % 8;
        lastBullet = millis();
      }*/
      if (!test) {
        test = true
        BulletDirection bulletDirection = BulletDirection(random(0, 4));
        bullets[bulletArray] = InitBullet(bulletDirection, BulletType::Normal);
                                                                      // 1 = active. 0 = inactive (skips the for loop below)
        bulletArray = (bulletArray + 1) % 8;
        lastBullet = millis();
      }
      //loop responsible for updating bullet positions
      if (millis() - lastBulletUpdate >= bulletSpeed) {
        for (int i = 0; i < 8; i++) {
            // Remove all current bullet draws first thing
            const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullets[i].type);
            Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullets[i].position.x,bullets[i].position.y);   
            drawRectangle(bulletBoundingBox, SoftwareColor::Black);
        }
        for (int i = 0; i < 8; i++) {
            if (!bullets[i].isActive) continue;
            updateBulletPosition(bullets[i]);
            const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullets[i].type);
            Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullets[i].position.x,bullets[i].position.y);            
           
            bool collidesWithShield = bulletBoundingBox.RectCollisionCheck(currentShieldState.currentPosition);
            if(collidesWithShield) {
                Serial.println("I WAS HERE");
                bullets[i].isActive = false;  // deactivate bullet
                counter++;
                counterDisplay(counter);
                continue;
            }
            Rectangle playerBoundingBox = bulletSprite.GetBoundingBox(heartDrawPosX, heartDrawPosY);        
            bool collidesWithPlayer = bulletBoundingBox.RectCollisionCheck(playerBoundingBox);
            if(collidesWithPlayer) {
                Serial.println("I WAS HERE");
                health--;
                tft.fillScreen(ILI9341_BLACK);
                break;
            }

          //erase old bullet position
          //tft.fillRect(oldBulletPosX, oldBulletPosY, bulletSizeX * pixel_multiplier, bulletSizeY * pixel_multiplier, ILI9341_BLACK);

          
        }
        // Issue draw calls on active bullets
        for (int i = 0; i < 8; i++) {
            if (!bullets[i].isActive) continue;
            drawSprite(bullets[i].position.x, bullets[i].position.y, bulletNormal);
        }
        
        lastBulletUpdate = millis();
      }

      // Draw the player
      drawSprite(heartDrawPosX, heartDrawPosY, heart);
    }
  }

  tft.setCursor(110, 110);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.print("Dead");
}