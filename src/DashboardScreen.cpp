#include <M5Unified.h>
#include <time.h>
#include "DashboardScreen.h"

void DashboardScreen::begin() {
  M5.begin();
  M5.Lcd.setRotation(3);  // Landscape (USB port is the left.) 240x135 
  M5.Lcd.fillScreen(bgColor);

  memset(pingLoss, 0, sizeof(pingLoss));
  memset(historySize, 0, sizeof(historySize));

  screenH = M5.Lcd.height();
  screenW = M5.Lcd.width();

  // set weather area
  auto top = 0;
  auto left = 0;
  auto width = screenW*3/4;     // 240*3/4 = 180px
  auto height = screenH - top;  // 135px
  weatherArea = Area {top, left, width, height};

  // set time area
  top = 0;
  left = screenW*3/4;      // 240/4 * 3 = 180px
  width = screenW/4;       // 240/4 = 60px
  height = 21;
  timeArea = Area {top, left, width, height};
  
  // set graph area
  top = 21;
  left = screenW*3/4;      // 240/4 * 3 = 180px
  width = screenW/4;       // 240/4 = 60px
  height = screenH - top;  // 135 - 21 = 124px
  graphArea = Area {top, left, width, height};
}

void DashboardScreen::setTime(const char *t) {
  strncpy(now, t, sizeof(now));
}

void DashboardScreen::setTemperatureH(float tempC) {
  temperatureH = tempC;
  sprintf(temperatureLastUpdated, "%02u:%02u", hours, minutes);
}

void DashboardScreen::setTemperatureL(float tempC) {
  temperatureL = tempC;
  sprintf(temperatureLastUpdated, "%02u:%02u", hours, minutes);
}

void DashboardScreen::setBackground(uint16_t color) {
  bgColor = color;
}

void DashboardScreen::updateTime() {
  auto dt  = M5.Rtc.getDateTime();
  if (dt.time.hours == hours && dt.time.minutes == minutes) return;

  hours = dt.time.hours;
  minutes = dt.time.minutes;
  drawTime();
}

void DashboardScreen::drawTime() {
  // clear time area
  M5.Lcd.fillRect(timeArea.top, timeArea.left, timeArea.width, timeArea.height, bgColor);

  // draw time
  M5.Lcd.setTextColor(TFT_WHITE, bgColor);
  M5.Lcd.setFont(&fonts::Font0);
  M5.Lcd.setTextSize(2); // x2
  M5.Lcd.setCursor(timeArea.left, timeArea.top);
  M5.Lcd.printf("%02u:%02u", hours, minutes);
}

void DashboardScreen::addPingLoss(uint8_t host, uint8_t lossPercent) {
  if (host >= HOST_COUNT) return;
  if (lossPercent > 100) lossPercent = 100;

  if (historySize[host] < HISTORY_LEN) {
    pingLoss[host][historySize[host]++] = lossPercent;
  } else {
    memmove(&pingLoss[host][0], &pingLoss[host][1], HISTORY_LEN - 1);
    pingLoss[host][HISTORY_LEN - 1] = lossPercent;
  }
}

void DashboardScreen::loop() {
  static int y_1 = 1;
  static int y_0 = 0;
  static int position[100] = {
    1,1,1,1,1,1,1,1,1,1,2,2,2,3,3,3,4,4,5,5,6,6,7,8,8,9,10,11,11,12,13,14,15,15,16,17,18,19,20,
    21,23,24,25,26,27,28,30,31,32,34,35,36,38,39,41,42,44,45,47,48,50,52,53,55,57,58,60,62,64,66,
    68,69,71,73,75,77,79,82,84,86,88,90,92,95,97,99,101,104,106,109,111,113,116,118,121,123,126,129,131,133,
  };
  M5.Lcd.drawPixel(graphArea.left-2, position[y_0], bgColor);
  M5.Lcd.drawPixel(graphArea.left-2, position[y_1], TFT_WHITE);
  y_1 = (y_1 + 1) % 100;
  y_0 = (y_0 + 1) % 100;
}

void DashboardScreen::update() {
  M5.Lcd.fillScreen(bgColor);
  drawWeather();
  drawGraphs();
  drawTime();
}

void DashboardScreen::drawWeather() {
  // Left side: weather + time

  // clear weather area
  M5.Lcd.fillRect(weatherArea.top, weatherArea.left, weatherArea.width, weatherArea.height, bgColor);

  M5.Lcd.setTextColor(TFT_DARKGRAY, bgColor);
  M5.Lcd.setFont(&fonts::Font0);
  M5.Lcd.setTextSize(2); // x2
  M5.Lcd.setCursor(4, 0);
  M5.Lcd.printf("Upd: %s", temperatureLastUpdated);

  uint16_t tempHighTop = 16; // time font size
  uint16_t tempHeight = (weatherArea.height - tempHighTop)/2;
  uint16_t tempLowTop = tempHighTop + tempHeight;
  uint16_t tempRight = weatherArea.width/2;
  char str[8+1];
  
  M5.Lcd.setTextSize(1); // x1

  M5.Lcd.setTextColor(0xFC40, bgColor);
  snprintf(str, sizeof(str), "%.0f", temperatureH);
  M5.Lcd.drawRightString(str, tempRight, tempLowTop - 52, &fonts::Font7);
  
  M5.Lcd.setTextColor(0x06BF, bgColor);
  snprintf(str, sizeof(str), "%.0f", temperatureL);
  M5.Lcd.drawRightString(str, tempRight, tempLowTop + 3, &fonts::Font7);
}

void DashboardScreen::drawGraphs() {
  // clear graph area
  M5.Lcd.fillRect(graphArea.top, graphArea.left, graphArea.width, graphArea.height, bgColor);

  // Right side: Graph area
  int hostWidth = graphArea.width / HOST_COUNT;  // 60/4 = 15px

  for (uint8_t h = 0; h < HOST_COUNT; h++) {
    drawHostGraph(h, graphArea.left + h*hostWidth, graphArea.top, hostWidth - 2, graphArea.height);
  }
}

void DashboardScreen::drawHostGraph(uint8_t host, int x, int y, int w, int h) {
  uint8_t alpha_rank = 3;
  uint8_t count = historySize[host];
  if (count == 0) return;

  int boxH = h / HISTORY_LEN;  // 135/12 = 11 ... 3

  for (uint8_t i = 0; i < count; i++) {
    uint8_t loss = pingLoss[host][count - i - 1];

    int by = y + i * boxH;
    
    uint16_t color;
    if (loss < 10) color = TFT_GREEN;
    else if (loss < 20) color = TFT_ORANGE;
    else if (loss < 40) color = TFT_RED;
    else if (loss < 60) color = TFT_PURPLE;
    else color = TFT_BLACK;

    // Display past data dimmed
    uint8_t alpha = (i / alpha_rank);
    for (uint8_t a = 0; a < alpha; a++) {
      color = (color & 0xF7DE) >> 1;
    }
    M5.Lcd.fillRoundRect(x, by, w, boxH - 1, 2, color);
  }
}
