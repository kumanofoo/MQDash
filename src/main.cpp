#include <M5Unified.h>
#include <esp_sntp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Wlan.h"
#include "Config.h"
#include "LRUHost.h"
#include "DashboardScreen.h"
#include "credential_qr.h"
#include "url_qr.h"

// NTP
static const char* NTP_TIMEZONE = "JST-9";
static const char* NTP_SERVER1 = "0.pool.ntp.org";
static const char* NTP_SERVER2 = "1.pool.ntp.org";
static const char* NTP_SERVER3 = "2.pool.ntp.org";

// MQTT
WiFiClient EspClient;
PubSubClient MQTTClient(EspClient);
const char* DeviceIdPrefix = "M5StickCPlus2";
String DeviceID = "";
String TemperatureTopic = "";
String PingTopic = "";
float ThresholdH = 30.0;
float ThresholdL = -4.0;

// Dashboard
DashboardScreen Dashboard;
LRUHost HostList(DashboardScreen::HOST_COUNT);

void setBluescreen() {
  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.setTextColor(WHITE, BLUE);
  M5.Lcd.setFont(&fonts::Font2);
  M5.Lcd.setTextSize(1);
}

void mqttCallback(char *topic_str, byte *payload, unsigned int length) {
  String topic(topic_str);
  Serial.printf("topic: %s\n", topic.c_str());
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    setBluescreen();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("JSON Parse Failed: ");
    M5.Lcd.println(error.c_str());
    return;
  }

  if (topic.equals(TemperatureTopic)) {
    float lowest = doc["lowest"];
    float highest = doc["highest"];
    const char *report_dt = doc["report_datetime"];
    Serial.println(report_dt);
    Serial.printf("L: %.1f\n", lowest);
    Serial.printf("H: %.1f\n", highest);
    Dashboard.setTemperatureL(lowest);
    Dashboard.setTemperatureH(highest);
    if (lowest <= ThresholdL) {
      if (highest <= 0) {
        Dashboard.setBackground(TFT_NAVY);
      }
      else {
        Dashboard.setBackground(TFT_RED);
      }
    }
    else if (highest >= ThresholdH) {
      if (lowest >= 30.0) {
        Dashboard.setBackground(TFT_YELLOW);
      }
      else {
        Dashboard.setBackground(TFT_ORANGE);
      }
    }
    else {
      Dashboard.setBackground(DashboardScreen::BG_DEFAULT);
    }
    M5.Speaker.tone(2000, 20);
  }

  if (topic.equals(PingTopic)) {
    Serial.print("rttmon: ");
    Serial.println(message);
    JsonArray arr = doc["anomalies"].as<JsonArray>();
    if (arr.isNull()) return;
    float lossValue[DashboardScreen::HOST_COUNT] = {0.0};
    for (JsonObject item: arr) {
      JsonObject anomaly = item["anomaly"];
      Serial.print("anomaly: ");
      Serial.println(anomaly);
      if (anomaly.isNull()) continue;
      float loss = anomaly["PacketLoss"] | -1.0;
      Serial.print("PacketLoss: ");
      Serial.println(loss);
      if (loss < 0.0) continue;
      String host = item["host"] | "";
      Serial.print("host: ");
      Serial.println(host);
      if (host.length() == 0) continue;
      uint8_t id = HostList.getHostId(host);
      Serial.print("id: ");
      Serial.println(id);
      lossValue[id] = loss;
    }
    Serial.printf("%.1f, %.1f, %.1f, %.1f\n", lossValue[0], lossValue[1], lossValue[2], lossValue[3]);
    for (int i = 0; i < DashboardScreen::HOST_COUNT; i++) {
      Dashboard.addPingLoss(i, lossValue[i]);
    }
  }
  
  Dashboard.update();
}

void mqttReconnect() {
  setBluescreen();
  M5.Lcd.setCursor(0, 0);
  while (!MQTTClient.connected()) {
    M5.Lcd.print("Connecting to MQTT...");
    Serial.print("Connecting to MQTT...");
    if (MQTTClient.connect(DeviceID.c_str())) {
      M5.Lcd.println("Connected!");
      Serial.println("Connected!");
      MQTTClient.subscribe(TemperatureTopic.c_str());
      MQTTClient.subscribe(PingTopic.c_str());
    } else {
      delay(5000);
    }
  }
}

bool sntp_sync_status_complete(false);
void sntpCallback(struct timeval *tv) {
  sntp_sync_status_t sntp_sync_status = sntp_get_sync_status();
  if (sntp_sync_status == SNTP_SYNC_STATUS_COMPLETED)
    sntp_sync_status_complete = true;
}

bool rtcUpdate() {
  if (!sntp_sync_status_complete) return false;
  sntp_sync_status_complete = false;
  time_t t = time(nullptr) + 1;
  while (t > time(nullptr)); // Wait for next seconds
  M5.Rtc.setDateTime(localtime(&t));

  auto dt = M5.Rtc.getDateTime();
  Serial.printf("Synchronized datetime: %04d/%02d/%02d %02d:%02d\n",
                dt.date.year, dt.date.month, dt.date.date,
                dt.time.hours, dt.time.minutes);
  return true;
}

// --------------------------------------------------
// setup()
// --------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("===== Serial ready =====");
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Lcd.setRotation(3);
  setBluescreen();
  M5.Speaker.tone(2000, 200);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("MQDash %s\n", VERSION);
  M5.Lcd.print("Press <M5> to Wi-Fi Setup.");

  bool forceAP = false;
  for (int i = 0; i < 200; i++) {
    M5.update(); // Update status of button
    if (M5.BtnA.isPressed()) {
      forceAP = true;
      break;
    }
    delay(10);
  }

  // -------------
  // Setting Wi-Fi
  // -------------
  setBluescreen();
  M5.Lcd.setCursor(0, 0);
  Wlan wlan;
  if (!wlan.credentialExists() || forceAP) {
    M5.Speaker.tone(2000, 200);
    int h = M5.Lcd.height();
    M5.Display.drawPng(CREDENTIAL_QR_PNG, CREDENTIAL_QR_PNG_LEN, 0, 0);
    M5.Lcd.setCursor(40, 0);
    M5.Lcd.printf("SSID: %s\n", AP_SSID);
    M5.Lcd.setCursor(40, 15);
    M5.Lcd.printf("PASS: %s\n", AP_PASS);
    
    M5.Display.drawPng(URL_QR_PNG, URL_QR_PNG_LEN, 0, h - 38);
    M5.Lcd.setCursor(40, h - 15);
    M5.Lcd.printf("192.168.4.1\n");
    startConfigServer(AP_SSID, AP_PASS);
  }

  if (wlan.connect()) {
    String mac_addr = WiFi.macAddress();
    DeviceID += DeviceIdPrefix;
    DeviceID += "_";
    DeviceID += mac_addr;
    M5.Lcd.println(":-)");
    Serial.println(":-)");
    Serial.println(DeviceID);
  }
  else {
    M5.Lcd.println("Wi-Fi failed."); 
    while (true);
  }
  delay(2000);

  // -----------
  // Setting RTC
  // -----------
  setBluescreen();
  M5.Lcd.setCursor(0, 0);
  if (!M5.Rtc.isEnabled()) {
    M5.Lcd.println("RTC not found");
    while (true);
  }
  else {
    M5.Lcd.print("Connecting to NTP...");
    Serial.print("Connecting to NTP...");
    configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    sntp_sync_status_complete = false;
    sntp_set_time_sync_notification_cb(sntpCallback);
    while (!rtcUpdate()) {
      M5.Lcd.print(".");
      Serial.print('.');
      delay(100);
    }
    M5.Lcd.println("Connected!");
    Serial.println("Connected!");
  }
  delay(2000);

  // ------------
  // Setting MQTT
  // ------------
  setBluescreen();
  M5.Lcd.setCursor(0, 0);
  Preferences prefs;
  bool checkParam(true);
  
  prefs.begin(PREF_NAMESPACE, true); // read mode
  String broker = prefs.getString(PREF_BROKER, "");
  String port_str = prefs.getString(PREF_PORT_STR, "");
  TemperatureTopic = prefs.getString(PREF_TEMP_TOPIC, "");
  PingTopic = prefs.getString(PREF_PING_TOPIC, "");
  String ThresholdHStr = prefs.getString(PREF_THRESHOLD_H_STR, "");
  String ThresholdLStr = prefs.getString(PREF_THRESHOLD_L_STR, "");
  prefs.end();
  
  if (broker.length() == 0) {
    M5.Lcd.println("No MQTT broker");
    checkParam = false;
  }
  uint16_t port(0);
  if (port_str.length() == 0) {
    M5.Lcd.println("No MQTT broker port");
    checkParam = false;
  }
  else {
    port = port_str.toInt();
    if (port == 0) {
      M5.Lcd.printf("Invalid port %s\n", port_str.c_str());
      checkParam = false;
    }
  }
  if (TemperatureTopic.length() == 0) {
    M5.Lcd.println("No temperature topic");
    checkParam = false;
  }
  if (PingTopic.length() == 0) {
    M5.Lcd.println("No ping topic");
    checkParam = false;
  }
  char *endPtr = nullptr;
  const char *cstr = ThresholdHStr.c_str();
  ThresholdH = strtof(cstr, &endPtr); 
  if (endPtr == cstr || *endPtr != '\0') {
    M5.Lcd.println("No highest temperature threshold");
    checkParam = false;
  }
  endPtr = nullptr;
  cstr = ThresholdLStr.c_str();
  ThresholdL = strtof(cstr, &endPtr); 
  if (endPtr == cstr || *endPtr != '\0') {
    M5.Lcd.println("No lowest temperature threshold");
    checkParam = false;
  }
  
  if (!checkParam)  {
    M5.Speaker.tone(2000, 500);
    while (true);
  }
  
  MQTTClient.setServer(broker.c_str(), port);
  MQTTClient.setCallback(mqttCallback);
  if (!MQTTClient.connected()) {
    mqttReconnect();
  }
  delay(2000);
  
  // --------------------
  // Initializing Screen
  // --------------------
  Dashboard.begin();
  Dashboard.setTemperatureH(-99.0);
  Dashboard.setTemperatureL(99.0);
  uint8_t h = DashboardScreen::HISTORY_LEN;
  for (int i = 0; i < h; i++) {
    Dashboard.addPingLoss(0, 100/(h-1) * (h-i-1));
    Dashboard.addPingLoss(1, random(0, 60));
    Dashboard.addPingLoss(2, random(0, 80));
    Dashboard.addPingLoss(3, random(0, 100));
  }
  Dashboard.update();
}

// --------------------------------------------------
// loop()
// --------------------------------------------------
void loop() {
  static const uint16_t INTERVAL_MS = 10;
  static const uint16_t SECONDS_COUNT = 1000/INTERVAL_MS;
  static int16_t second_timer = SECONDS_COUNT;
  
  M5.update();

  if (!MQTTClient.connected()) {
    mqttReconnect();
  }
  MQTTClient.loop();
  Dashboard.loop();
  rtcUpdate();
  
  second_timer -= 1;
  if (second_timer == 0) {
    second_timer = SECONDS_COUNT;
    Dashboard.updateTime();
  }

  delay(INTERVAL_MS);
}
