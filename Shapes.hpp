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
  // Checks if the two rectangles overlap
  bool RectCollisionCheck(const Rectangle& rect) {
    //Serial.print(A rect x: );
    return  (rect.topLeftCorner.x < bottomRightCorner.x && rect.bottomRightCorner.x > topLeftCorner.x &&
            rect.topLeftCorner.y > bottomRightCorner.y && rect.bottomRightCorner.y < topLeftCorner.y);
  }
};