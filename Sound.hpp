#pragma once
const int BuzzerPin = 30;

#include "pitches.h"

void PlayBlockSound() {
  tone(BuzzerPin, NOTE_A4, 250);
}
void PlayerHitSound() {
  tone(BuzzerPin, NOTE_F2, 250);
}
void PlayerHitNoDamageSound() {
  tone(BuzzerPin, NOTE_D3, 250);
}