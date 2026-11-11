#pragma once
// sprite colors
enum class SpriteColor {
  Black = 0,
  Red = 1,
  White = 2
};
template<uint8_t XDim, uint8_t YDim>
struct Sprite {
  uint8_t data[XDim][YDim];
};