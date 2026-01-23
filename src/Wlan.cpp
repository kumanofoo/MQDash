#include <M5Unified.h>
#include <WiFi.h>
#include "Config.h"
#include "Wlan.h"

void showInfo(const char* mode, const String& ssid,
              const String& pass, const IPAddress& ip) {
  M5.Lcd.printf("Mode: %s\n", mode);
  M5.Lcd.printf("SSID: %s\n", ssid.c_str());
  M5.Lcd.printf("IP: %s\n", ip.toString().c_str());
}

Wlan::Wlan() {
  Preferences prefs;
  prefs.begin(PREF_NAMESPACE, true); // read-only mode
  staSSID = prefs.getString(PREF_SSID, "");
  staPASS = prefs.getString(PREF_PASS, "");
  prefs.end();
}

bool Wlan::connect() {
  if (staSSID.length() == 0 || staPASS.length() == 0) {
    return false;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(staSSID.c_str(), staPASS.c_str());
  uint32_t start = millis();
  while ((WiFi.status() != WL_CONNECTED) && (millis() - start < 15000)) {
    delay(500);
    M5.Lcd.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    myIP = WiFi.localIP();
    show();
    return true;
  } else {
    return false;
  }
}

bool Wlan::credentialExists() {
  return (staSSID.length() != 0 && staPASS.length() != 0);
}

void Wlan::show() {
  showInfo("STA", staSSID, staPASS, myIP);
}
