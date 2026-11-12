#pragma once
#include "Vector.hpp"

class Rectangle {
public:
  Vector16 topLeftCorner;
  Vector16 bottomRightCorner;
  // Checks if the given x,y point is colliding with the rectangle 
  bool CollisionCheck(Vector16 point) {
    return topLeftCorner.x <= point.x && point.x <= bottomRightCorner.x &&
           topLeftCorner.y <= point.y && point.y <= bottomRightCorner.y;
  }
};