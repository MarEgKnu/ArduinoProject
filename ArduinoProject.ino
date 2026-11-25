#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Sprite.hpp"
#include "Shapes.hpp"
#include "SizedArray.hpp"
#include "EntityContainer.hpp"
#include <EEPROM.h>
#include <TimerOne.h>

enum menuItem {
  Play,
  Difficulty,
  Leaderboard,
};

enum difficulty {
  Easy,
  Medium,
  Hard,
};

enum class Direction : uint8_t {
  None = 4,
  Left = 0,
  Right = 1,
  Up = 2,
  Down = 3
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
  Direction direction = Direction::None;
  Rectangle lastPosition = { -1, -1 };
  Rectangle currentPosition{ 0, 0 };
};

struct Bullet {
  Vector16 lastPosition = { -1, -1 };  // this value should be negative when it is just been spawned in, and thus does not have a last position
  Vector16 position = { 0, 0 };
  BulletDirection direction = BulletDirection::Down;
  BulletType type = BulletType::Normal;
  bool isActive = false;
};

struct GameState {
  Adafruit_ILI9341 tft(TFT_CS, TFT_DC, 11, 13, TFT_RST, 12);
  // last general update
  unsigned long lastUpdate = 0;
  // last new bullet spawned
  unsigned long lastBullet = 0;
  // speed of bullet spawn and bullet speed (we use interrupt to tweak this)
  int bulletRate = 750;
  unsigned long lastFrame = 0;
  //booleans to keep track of which screens we're in currently - mainMenu is initally true because its the first menu we want to display
  bool menuScreen = true;
  bool mainMenu = true;
  bool difficultyMenu = false;
  bool lbScreen = false;
  // self explanatory
  bool healthHasChanged = false;
  volatile int32_t speedMultiplier = 1;
  

}

const uint8_t bulletWeights[] = {
  50, // Normal
  15, // SlowHighDamage
  15, // FastLowDamage
  20 // NoBlock
};
const uint8_t bulletDamage[] = {
  2, // Normal
  4, // SlowHighDamage
  1, // FastLowDamage
  2 // NoBlock
};
const uint8_t bulletSpeed[] = {
  5, // Normal
  3, // SlowHighDamage
  10, // FastLowDamage
  5 // NoBlock
};

const uint8_t bulletWeightSize = sizeof(bulletWeights) / sizeof(bulletWeights[0]);

const uint8_t totalBulletWeights = [&]() {
  int totalWeight = 0;
  for(int i = 0; i < bulletWeightSize;i++) {
    totalWeight += bulletWeights[i];
  }
  return totalWeight;
}();



// Define the TFT pins
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define vert A12
#define horz A11
#define SEL_PIN 22

const int screenSizeX = 320;
const int screenSizeY = 240;

const int playAreaX = 240;
const int playAreaY = 240;

const int playAreaStartX = 40;
const int playAreaEndX = 280;

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


// How far the shield should be from the player object.
const int shieldOffsetFromPlayer = 6;

const int shieldSizeX = 2;
const int shieldSizeY = 10;

const int updateInterval = 32;

// Num frames per second
const int frameRate = 30;
const int msPerFrame = 1000 / frameRate;

//Gamestate variables


const int speedMultIncrease = 0.1;
const int speedIncreaseTime = 4 * 1000 * 1000; // in microseconds


// cursor position
int cursorPosition = 0;

int lastCursorPosition = -1;

Direction lastMenuDirection = Direction::None;

// menu screen items
const char* menuOptions[3]{
  "Play",
  "Difficulty",
  "Leaderboard"
};

// difficulty screen items
const char* difficultyOptions[3]{
  "Easy",
  "Medium",
  "Hard"
};

// length of menu array
int menuOptionsL = sizeof(menuOptions) / sizeof(menuOptions[0]);

// length of difficulty options
int difficultyOptionsL = sizeof(difficultyOptions) / sizeof(difficultyOptions[0]);

//container to keep track of active bullets.
EntityContainer<Bullet, 8> bullets = {};
// array of bullets to be deleted (removed from the screen)
SizedArray<Bullet, 8> deletedBullets = {};

// how many hits the player can take. can be changed from menu eventually
int health = 10;

uint32_t currentScore = 0;     // current score for player
bool scoreHasChanged = false;  // flag for when the score changes, and drawing a new counter is needed

// The current position (and bounding box) of the player shield, and a bool determining if it is currently active or not
ShieldState currentShieldState{};
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
  switch (type) {
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

void onTimerTrigger() {
  if(!menuScreen) {
    speedMultiplier += speedMultIncrease;
  }
}

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

BulletType generateBulletType() {
  int weight = random(0, totalBulletWeights - 1);
  uint8_t weightTest = bulletWeights[uint8_t(BulletType::Normal)];
  if(weight < weightTest) {
    return BulletType::Normal;
  }
  weightTest += bulletWeights[uint8_t(BulletType::SlowHighDamage)];
  if(weight < weightTest) {
    return BulletType::SlowHighDamage;
  }
  weightTest += bulletWeights[uint8_t(BulletType::FastLowDamage)];
  if(weight < weightTest) {
    return BulletType::FastLowDamage;
  }
  else {
    return BulletType::NoBlock;
  }
}

void onPlayerHit(const Bullet& bullet, uint8_t bulletIndex) {
    if(bullet.type != BulletType::NoBlock) {
      uint8_t damage = bulletDamage[uint8_t(bullet.type)];
      health -= damage;
      healthHasChanged = true;
    }
    else {
      setNewScore(currentScore + 1);
    }
    deletedBullets.PushBack(bullet);
    bullets.DeleteUnsafe(bulletIndex);
}
void onShieldHit(const Bullet& bullet, uint8_t bulletIndex) {
    if(bullet.type == BulletType::NoBlock) {
      uint8_t damage = bulletDamage[uint8_t(bullet.type)];
      health -= damage;
      healthHasChanged = true;
    }
    else {
      setNewScore(currentScore + 1);
    }
    deletedBullets.PushBack(bullet);
    bullets.DeleteUnsafe(bulletIndex);
}


void clearOldSpritesFromScreen() {
  // Remove bullets with a diffrent position than last update
  for (int i = 0; i < bullets.Size(); i++) {
    const auto& bullet = bullets.Get(i);
    if (bullet.lastPosition != bullet.position) {
      const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullet.type);
      Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullet.lastPosition.x, bullet.lastPosition.y);
      drawRectangle(bulletBoundingBox, SoftwareColor::Black);
    }
  }
  // Remove all bullets in the deletedBullets array and clear it
  for (uint8_t idx = 0; idx < deletedBullets.Size(); idx++) {
    const auto& bullet = deletedBullets[idx];
    const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullet.type);
    Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullet.lastPosition.x, bullet.lastPosition.y);
    drawRectangle(bulletBoundingBox, SoftwareColor::Black);
  }
  deletedBullets.Clear();
  // Remove old shield
  if (currentShieldState.lastPosition != currentShieldState.currentPosition) {
    drawRectangle(currentShieldState.lastPosition, SoftwareColor::Black);
  }
  // The player cannot move so no need to un-draw that
}

void processJoystickInput(Direction joystickDirection) {
  // save last shield position. Its ok if they are the same
  currentShieldState.lastPosition = currentShieldState.currentPosition;
  switch (joystickDirection) {
    // Set the shield bounding box, calculate the exact coordinates where it should appear and draw it on the screen
    case Direction::Left:
      {
        int x = (heartDrawPosX)-shieldOffsetFromPlayer * pixel_multiplier;
        int y = (heartDrawPosY)-shieldSizeY / 2;
        currentShieldState.currentPosition = shieldVertical.GetBoundingBox(x, y);
        break;
      }
    case Direction::Right:
      {
        int x = (heartDrawPosX + heartSizeX * pixel_multiplier) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeX * pixel_multiplier;
        int y = (heartDrawPosY)-shieldSizeY / 2;
        currentShieldState.currentPosition = shieldVertical.GetBoundingBox(x, y);
        break;
      }
    case Direction::Up:
      {
        int x = (heartDrawPosX)-shieldSizeX * pixel_multiplier / 2;
        int y = (heartDrawPosY)-shieldOffsetFromPlayer * pixel_multiplier;
        currentShieldState.currentPosition = shieldHorizontal.GetBoundingBox(x, y);
        break;
      }
    case Direction::Down:
      {
        int x = (heartDrawPosX)-shieldSizeX * pixel_multiplier / 2;
        int y = (heartDrawPosY + heartSizeY * pixel_multiplier) + shieldOffsetFromPlayer * pixel_multiplier - shieldSizeX * pixel_multiplier;
        currentShieldState.currentPosition = shieldHorizontal.GetBoundingBox(x, y);
        break;
      }
    case Direction::None:
      {
        break;
      }
  }
  currentShieldState.direction = joystickDirection;
}

void displayMenu(const char** options, int optionsLength) {
  Direction currentDir = getJoystickDirection();

  if (currentDir != lastMenuDirection) {

    if (currentDir == Direction::Up && cursorPosition > 0) {
      cursorPosition--;
    } else if (currentDir == Direction::Down && cursorPosition < optionsLength - 1) {
      cursorPosition++;
    }

    lastMenuDirection = currentDir;
  }

  if (cursorPosition != lastCursorPosition) {
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);

    // draw the menu items only once when the menu first opens
    if (lastCursorPosition == -1) {
      for (int a = 0; a < optionsLength; a++) {
        tft.setCursor(0, 50 + (a * 60));
        tft.print(options[a]);
      }
    }

    // only update the small cursor spot next to each item
    for (int a = 0; a < optionsLength; a++) {
      int textWidth = strlen(options[a]) * 12;  // 6px base width * textSize 2
      int yPos = 50 + (a * 60);

      if (cursorPosition == a) {
        tft.setCursor(textWidth, yPos);
        tft.print(" <");
      } else {
        tft.fillRect(textWidth, yPos, 24, 16, ILI9341_BLACK);
      }
    }
    lastCursorPosition = cursorPosition;
  }
}

// function to initialize play area
void playArea() {
  // play area
  tft.fillScreen(ILI9341_WHITE);
  tft.fillRect(heartMidpointX - (playAreaX / 2), heartMidpointY - (playAreaY / 2), playAreaX, playAreaY, ILI9341_BLACK);
  tft.fillRect(20, 30, 10, 10, ILI9341_RED);
  tft.fillRect(280, 30, 10, 10, ILI9341_RED);
  // draw inital health
  tft.setCursor(42, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("HP: ");
  tft.print(health);
}

void drawSpritesToScreen() {
  // draw bullets
  for (int i = 0; i < bullets.Size(); i++) {
    const auto& bullet = bullets.Get(i);
    const auto& bulletSprite = GetBulletTypeSprite(bullet.type);
    drawSprite(bullet.position.x, bullet.position.y, bulletSprite);
  }
  const auto& shield = currentShieldState;
  // draw shield
  switch (shield.direction) {
    case Direction::Left:
    case Direction::Right:
      drawSprite(shield.currentPosition.topLeftCorner.x, shield.currentPosition.topLeftCorner.y, shieldVertical);
      break;
    case Direction::Up:
    case Direction::Down:
      drawSprite(shield.currentPosition.topLeftCorner.x, shield.currentPosition.topLeftCorner.y, shieldHorizontal);
      break;
    case Direction::None:
      break;
  }
  // Draw the player
  drawSprite(heartDrawPosX, heartDrawPosY, heart);
}

void updateBulletPosition(Bullet& bullet) {
  // store last position
  bullet.lastPosition = bullet.position;
  // Disable interupt while it is reading the variable which it will modify
  noInterrupts();
  uint8_t speed = bulletSpeed[uint8_t(bullet.type)] * speedMultiplier;
  interrupts();
  switch (bullet.direction) {
    case BulletDirection::Left:
      bullet.position.x += speed;
      break;
    case BulletDirection::Right:
      bullet.position.x -= speed;
      break;
    case BulletDirection::Up:
      bullet.position.y += speed;
      break;
    case BulletDirection::Down:
      bullet.position.y -= speed;
      break;
  }
}

Bullet InitBullet(BulletDirection direction, BulletType type) {
  Vector16 position;
  switch (direction) {
    case BulletDirection::Left:
      position.x = playAreaStartX;
      position.y = heartMidpointY - (bulletSizeY * pixel_multiplier / 2);
      break;
    case BulletDirection::Right:
      position.x = playAreaEndX - bulletSizeX;  // offset since it draws towards the right
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
  Bullet bullet{};
  bullet.lastPosition = Vector16{ -1, -1 };
  bullet.position = position;
  bullet.direction = direction;
  bullet.type = type;
  bullet.isActive = true;
  return bullet;
}
void setNewScore(uint32_t newScore) {
  currentScore = newScore;
  scoreHasChanged = true;
}

void drawScoreCounterDisplay() {
  if (scoreHasChanged) {
    scoreHasChanged = false;
    tft.fillRect(240, 0, 40, 40, ILI9341_BLACK);
    tft.setCursor(240, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print(currentScore);
  }
}

// i just copy pasted the other function for score almost lo
void displayHealth() {
  if (healthHasChanged) {
    healthHasChanged = false;
    tft.fillRect(42, 0, 60, 40, ILI9341_BLACK);
    tft.setCursor(42, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("HP: ");
    tft.print(health);
  }
}
// function to handle exiting screens
void exitMenu(bool& menu) {
  menu = false;
  lastCursorPosition = -1;
  cursorPosition = 0;
  tft.fillRect(0, 0, 160, 240, ILI9341_BLACK);
}

void setup() {
  Serial.begin(9600);

  // rng for bullet direction and bullet type
  randomSeed(analogRead(A3));
  Timer1.initialize(speedIncreaseTime);
  Timer1.attachInterrupt(onTimerTrigger); // run the timer interrupt

  pinMode(vert, INPUT);
  pinMode(horz, INPUT);
  pinMode(SEL_PIN, INPUT_PULLUP);

  // get the saved health value (this depends on which difficulty you chose last). default is 3 (if a difficulty hasnt been chosed yet)
  EEPROM.get(0, health);
  if (health < 1 || health > 3) {
    health = 3;
  }
  // Start the display
  tft.begin();

  // Set screen orientation
  tft.setRotation(1);

  // draw logo
  drawSprite(192, 72, heart, 12);
}

void loop() {

  //menu screen
  while (menuScreen) {
    if (mainMenu) {
      displayMenu(menuOptions, menuOptionsL);
      if (digitalRead(SEL_PIN) == LOW && cursorPosition == Play) {
        playArea();
        menuScreen = false;
      } else if (digitalRead(SEL_PIN) == LOW && cursorPosition == Difficulty) {
        exitMenu(mainMenu);
        difficultyMenu = true;
      }
    }
    if (difficultyMenu) {
      displayMenu(difficultyOptions, difficultyOptionsL);
      if (digitalRead(SEL_PIN) == LOW && cursorPosition == Easy) {
        exitMenu(difficultyMenu);
        health = 3;
        EEPROM.put(0, health);
        mainMenu = true;
      } else if (digitalRead(SEL_PIN) == LOW && cursorPosition == Medium) {
        exitMenu(difficultyMenu);
        health = 2;
        EEPROM.put(0, health);
        mainMenu = true;
      } else if (digitalRead(SEL_PIN) == LOW && cursorPosition == Hard) {
        exitMenu(difficultyMenu);
        health = 1;
        EEPROM.put(0, health);
        mainMenu = true;
      }
    }
    //leaderboard
  }
  Timer1.restart(); // restart timer when game begins
  // game loop
  while (health > 0) {

    // Check if it's time for a game state update
    int miliseconds = millis();
    if (miliseconds - lastUpdate >= updateInterval) {
      lastUpdate = miliseconds;

      // Get direction of the joystick
      Direction joystickDirection = getJoystickDirection();

      if (joystickDirection != lastJoystickDirection) {
        lastJoystickDirection = joystickDirection;
        processJoystickInput(joystickDirection);
      }
      //spawns new bullet after millis exceeds bulletrate. assign direction, type and array to it
      if (miliseconds - lastBullet >= bulletRate) {
        BulletDirection bulletDirection = BulletDirection(random(0, 4));
        BulletType type = generateBulletType();
        // If not enough space, it will not insert it
        bullets.Add(InitBullet(bulletDirection, type));
        lastBullet = miliseconds;
      }
      for (int i = 0; i < bullets.Size(); i++) {
        auto& bullet = bullets.Get(i);
        updateBulletPosition(bullet);
        const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullet.type);
        Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullet.position.x, bullet.position.y);

        bool collidesWithShield = currentShieldState.direction != Direction::None && bulletBoundingBox.RectCollisionCheck(currentShieldState.currentPosition);
        if (collidesWithShield) {
          onShieldHit(bullet, i);
          continue;
        }
        Rectangle playerBoundingBox = bulletSprite.GetBoundingBox(heartDrawPosX, heartDrawPosY);
        bool collidesWithPlayer = bulletBoundingBox.RectCollisionCheck(playerBoundingBox);
        if (collidesWithPlayer) {
          onPlayerHit(bullet, i);
          continue;
        }
      }
    }
    // Render things to the screen if it's time for a frame update
    if (miliseconds - lastFrame >= msPerFrame) {
      lastFrame = miliseconds;
      // Clear old sprites
      clearOldSpritesFromScreen();
      // then draw the new ones
      drawSpritesToScreen();
      // Display score counter
      drawScoreCounterDisplay();
      // Draw the hp amount
      displayHealth();
    }
    if (health == 0) {
      tft.fillScreen(ILI9341_BLACK);
    }
  }

  tft.setCursor(90, 110);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.print("Dead :(");
}