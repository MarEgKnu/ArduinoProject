#pragma once
#include "Shapes.hpp"

const int pixel_multiplier = 3;  // How many pixels is one x and y coordinate in our sprite?
// sprite colors
enum class SoftwareColor {
  Black = 0,
  Red = 1,
  White = 2,
  Green = 3,
  Orange = 4
};

int SoftwareColorToHardwareColor(SoftwareColor color) {
  switch(color) {
    case SoftwareColor::Black: {
        return ILI9341_BLACK;
    }
    case SoftwareColor::Red: {
        return ILI9341_RED;
        break;
    }
    case SoftwareColor::White: {
        return ILI9341_WHITE;
        break;
    }
    case SoftwareColor::Green: {
        return ILI9341_GREEN;
        break;
    }
     case SoftwareColor::Orange: {
        return ILI9341_ORANGE;
        break;
    }
    default:{
        return ILI9341_BLACK;
        break;
    }   
  }
}


template<uint8_t XDim, uint8_t YDim>
struct Sprite {
  uint8_t data[YDim][XDim];

  Rectangle GetBoundingBox(int x, int y) {
    return Rectangle{.topLeftCorner = {x, y}, .bottomRightCorner = {x + XDim * pixel_multiplier, y + YDim * pixel_multiplier} };
  }

  Sprite GetMirroredSprite() const {
    Sprite<XDim, YDim> sprite{};
    for(int x = 0; x < YDim;x++) {
      for(int y = 0; y < XDim;y++) {
        sprite.data[XDim - 1 - x][YDim - 1 - y] = data[x][y];
      }
    }
    return sprite;
  }
  Sprite<YDim, XDim> GetRotatedSprite() const {
    Sprite<YDim, XDim> sprite{};
    for(int x = 0; x < YDim;x++) {
      for(int y = 0; y < XDim;y++) {
        sprite.data[y][x] = data[x][y];
      }
    }
    return sprite;
  }

  Sprite Rotate90DegreesRight() const {
    return GetRotatedSprite();
  }
  Sprite Rotate180Degrees() const {
    return GetMirroredSprite();
  }
  Sprite Rotate90DegreesLeft() const {
    return Rotate90DegreesRight().GetMirroredSprite();
  }

};