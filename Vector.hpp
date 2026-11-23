#pragma once

struct Vector16 {
  int16_t x;
  int16_t y;
  bool operator== (const Vector16& rhs) const {
    return x == rhs.x && y == rhs.y;
  }
  bool operator!= (const Vector16& rhs) const {
    return !(*this == rhs);
  }
};