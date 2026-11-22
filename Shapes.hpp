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

    Serial.print("this topLeftCorner.x: ");
    Serial.print(topLeftCorner.x);
    Serial.print(" this topLeftCorner.y: ");
    Serial.print(topLeftCorner.y);
    Serial.print(" this bottomRightCorner.x: ");
    Serial.print(bottomRightCorner.x);
    Serial.print(" this bottomRightCorner.y: ");
    Serial.print(bottomRightCorner.y);

    Serial.print(" rect topLeftCorner.x: ");
    Serial.print(rect.topLeftCorner.x);
    Serial.print(" rect topLeftCorner.y: ");
    Serial.print(rect.topLeftCorner.y);
    Serial.print(" rect bottomRightCorner.x: ");
    Serial.print(rect.bottomRightCorner.x);
    Serial.print(" rect bottomRightCorner.y: ");
    Serial.println(rect.bottomRightCorner.y);
    // adapted from https://www.geeksforgeeks.org/dsa/find-two-rectangles-overlap/
    // If one rectangle is on left side of other
    if (topLeftCorner.x > rect.bottomRightCorner.x || rect.topLeftCorner.x > bottomRightCorner.x)
        return false;

    // If one rectangle is above other
    if (topLeftCorner.y > rect.bottomRightCorner.y || rect.topLeftCorner.y > bottomRightCorner.y)
        return false;

    return true;
  }
};