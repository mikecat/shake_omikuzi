#include <M5StickCPlus.h>
#include "images.h"

const int luckCandidates[][2] = {
  {0, 3}, {1, 3}, {2, 3}, {-1, 4}, {0, 4}
};
const int luckCandidateNum = (int)(sizeof(luckCandidates) / sizeof(luckCandidates[0]));

const float batteryMin = 3.0, batteryMax = 4.2;
const float batteryWarnRatio = 0.35;
bool lowBatteryWarned = false;

unsigned long prevTime = 0;
float prevX = 0, prevY = 0, prevZ = 0;
float nogSizeLpf = 0;
const float lpfAlpha = 0.5;
const float shakeOnThreshold = 0.5, shakeOffThreshold = 0.3;
bool shaked = false;
bool prevShaked = false;
int kuziPos = 0;
const int kuziPosMax = 10;
int luck[2] = {-1, -1};
unsigned long luckSelect = 0;

const int kuziWidth = 32;
int kuziX, kuziMaxY;
const uint16_t kuziColor = M5.Lcd.color565(240, 155, 89);
const uint16_t backgroundColor = BLACK;
const uint16_t characterColor = BLACK;

unsigned long frameNo = 0;

void drawLuck(int luckY) {
  for (int i = 0; i < 2; i++) {
    if (luck[i] < 0) {
      M5.Lcd.fillRect(kuziX, luckY + 32 * i, 32, 32, kuziColor);
    } else {
      M5.Lcd.drawXBitmap(kuziX, luckY + 32 * i, luckChars[luck[i]], 32, 32, characterColor, kuziColor);
    }
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(2);
  M5.Lcd.fillScreen(backgroundColor);
  M5.Imu.Init();
  prevTime = millis();
  kuziX = M5.Lcd.width() / 2 - kuziWidth / 2;
  kuziMaxY = M5.Lcd.height() - 32;
}

void loop() {
  M5.update();
  frameNo++;
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 20UL) {
    prevTime = currentTime;
    float accelX = 0, accelY = 0, accelZ = 0;
    M5.Imu.getAccelData(&accelX, &accelY, &accelZ);
    float nogX = accelX - prevX, nogY = accelY - prevY, nogZ = accelZ - prevZ;
    float gX = accelX - nogX, gY = accelY - nogY, gZ = accelZ - nogZ;
    prevX = accelX; prevY = accelY; prevZ = accelZ;
    float nogSize = sqrt(nogX * nogX + nogY * nogY + nogZ * nogZ);
    luckSelect += (unsigned long)(nogSize * 1000);
    nogSizeLpf = nogSizeLpf * (1 - lpfAlpha) + nogSize * lpfAlpha;
    float gHorzSize = sqrt(gX * gX + gZ * gZ);
    bool isDown = gY < -gHorzSize, isUp = gY > gHorzSize;
    if (shaked) {
      if (nogSizeLpf < shakeOffThreshold) shaked = false;
    } else {
      if (nogSizeLpf > shakeOnThreshold) shaked = true;
    }
    bool shakedNow = shaked && !prevShaked;
    prevShaked = shaked;
    bool appear = false;
    if (shakedNow) {
      appear = frameNo % 5 == 0;
      if (appear && kuziPos == 0) {
        luck[0] = luckCandidates[luckSelect % luckCandidateNum][0];
        luck[1] = luckCandidates[luckSelect % luckCandidateNum][1];
      }
      M5.Beep.tone(100, 50);
    }
    if (isDown && kuziPos < kuziPosMax && (kuziPos > 0 || appear)) {
      int kuziPrevY = kuziMaxY * kuziPos / kuziPosMax;
      kuziPos++;
      int kuziY = kuziMaxY * kuziPos / kuziPosMax;
      int kuziDeltaStartY = kuziPrevY - kuziWidth / 4 - kuziWidth * 2;
      int kuziCharacterY = kuziY - kuziWidth / 4 - kuziWidth * 2;
      M5.Lcd.fillRoundRect(kuziX, kuziY - kuziWidth / 2, kuziWidth, kuziWidth / 2, kuziWidth / 4, kuziColor);
      M5.Lcd.fillRect(kuziX, kuziDeltaStartY, kuziWidth, kuziCharacterY - kuziDeltaStartY, kuziColor);
      drawLuck(kuziCharacterY);
    }
    if (isUp && kuziPos > 0) {
      int kuziPrevY = kuziMaxY * kuziPos / kuziPosMax;
      kuziPos--;
      int kuziY = kuziMaxY * kuziPos / kuziPosMax;
      int kuziDeltaStartY = kuziY - kuziWidth / 4 - kuziWidth * 2;
      M5.Lcd.fillRect(kuziX, kuziDeltaStartY, kuziWidth, kuziPrevY - kuziDeltaStartY, backgroundColor);
      M5.Lcd.fillRoundRect(kuziX, kuziY - kuziWidth / 2, kuziWidth, kuziWidth / 2, kuziWidth / 4, kuziColor);
      drawLuck(kuziDeltaStartY);
    }
    float batteryRatio = (M5.Axp.GetBatVoltage() - batteryMin) / (batteryMax - batteryMin);
    if (batteryRatio < batteryWarnRatio) {
      if (!lowBatteryWarned) {
        M5.lcd.drawXBitmap(8, M5.Lcd.height() - 24, lowBatteryIcon[0], 32, 16, LIGHTGREY);
        M5.lcd.drawXBitmap(8, M5.Lcd.height() - 24, lowBatteryIcon[1], 32, 16, RED);
        lowBatteryWarned = true;
      }
    } else {
      if (lowBatteryWarned) {
        M5.Lcd.fillRect(8, M5.Lcd.height() - 24, 32, 16, backgroundColor);
        lowBatteryWarned = false;
      }
    }
  }
}
