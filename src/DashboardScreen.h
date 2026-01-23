#ifndef DASHBOARD_SCREEN_H
#define DASHBOARD_SCREEN_H

class DashboardScreen {
public:
  static const uint8_t HOST_COUNT = 4;
  static const uint8_t HISTORY_LEN = 12; // 60 min / 5 min
  //                                 0brrrrrggggggbbbbb
  static const uint16_t BG_DEFAULT = 0b0011100011100111;
  
  void begin();
  void setTime(const char *t);
  void setTemperatureH(float tempC);
  void setTemperatureL(float tempC);
  void addPingLoss(uint8_t host, uint8_t lossPercent);
  void update();
  void loop();
  void setBackground(uint16_t color);

private:
  uint16_t screenW = 0;
  uint16_t screenH = 0;
  char now[5+1] = ""; // '1', '2', ':', '3', '4', '\0'
  char temperatureLastUpdated[5+1] = ""; // '1', '2', ':', '3', '4', '\0'
  float temperatureH = 0.0f;
  float temperatureL = 0.0f;
  uint16_t bgColor = BG_DEFAULT;

  uint8_t pingLoss[HOST_COUNT][HISTORY_LEN];
  uint8_t historySize[HOST_COUNT];

  void drawWeather();
  void drawGraphs();
  void drawHostGraph(uint8_t host, int x, int y, int w, int h);
};

#endif // DASHBOARD_SCREEN_H
