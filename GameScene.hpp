#pragma once

void clearOldSpritesFromScreen(Adafruit_ILI9341& tft) {
  // Remove bullets with a diffrent position than last update
  for (int i = 0; i < bullets.Size(); i++) {
    const auto& bullet = bullets.Get(i);
    if (bullet.lastPosition != bullet.position) {
      const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullet.type);
      Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullet.lastPosition.x, bullet.lastPosition.y);
      drawRectangle(bulletBoundingBox, SoftwareColor::Black, tft);
    }
  }
  // Remove all bullets in the deletedBullets array and clear it
  for (uint8_t idx = 0; idx < deletedBullets.Size(); idx++) {
    const auto& bullet = deletedBullets[idx];
    const Sprite<bulletSizeX, bulletSizeY>& bulletSprite = GetBulletTypeSprite(bullet.type);
    Rectangle bulletBoundingBox = bulletSprite.GetBoundingBox(bullet.lastPosition.x, bullet.lastPosition.y);
    drawRectangle(bulletBoundingBox, SoftwareColor::Black, tft);
  }
  deletedBullets.Clear();
  // Remove old shield
  if (currentShieldState.lastPosition != currentShieldState.currentPosition) {
    drawRectangle(currentShieldState.lastPosition, SoftwareColor::Black);
  }
  // The player cannot move so no need to un-draw that
}