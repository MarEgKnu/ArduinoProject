#pragma once
#include "Shapes.hpp"
// sprite colors
enum class SoftwareColor {
  Black = 0,
  Red = 1,
  White = 2
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
    default:{
        return ILI9341_BLACK;
        break;
    }   
  }
}


template<uint8_t XDim, uint8_t YDim>
struct Sprite {
  uint8_t data[XDim][YDim];

  Rectangle GetBoundingBox(int x, int y) {
    return Rectangle{.topLeftCorner = {x, y}, .bottomRightCorner = {x + XDim, y + YDim} };
  }

  Sprite GetMirroredSprite() const {
    Sprite<XDim, YDim> sprite{};
    for(int x = 0; x < XDim;x++) {
      for(int y = 0; y < YDim;y++) {
        sprite.data[XDim - 1 - x][YDim - 1 - y] = data[x][y];
      }
    }
    return sprite;
  }
  Sprite<YDim, XDim> GetRotatedSprite() const {
    Sprite<YDim, XDim> sprite{};
    for(int x = 0; x < XDim;x++) {
      for(int y = 0; y < YDim;y++) {
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