#include <M5Unified.h>
#include <WiFi.h>
#include <WebServerInstance.h>
#include "Config.h"

String Html = "";

void handleRoot() {
  WebServer& server = WebServerInstance::getInstance();
  server.send(200, "text/html", Html);
}

void handleSave() {
  Preferences prefs;

  WebServer& server = WebServerInstance::getInstance();
  
  prefs.begin(PREF_NAMESPACE, false); // read-write mode
  if (server.hasArg("ssid")) {
    prefs.putString(PREF_SSID, server.arg("ssid"));
  }
  if (server.hasArg("password")) {
    if (server.arg("password") != PASS_MASK) {
      prefs.putString(PREF_PASS, server.arg("password"));
    }
  }
  if (server.hasArg("broker")) {
    prefs.putString(PREF_BROKER, server.arg("broker"));
  }
  if (server.hasArg("port")) {
    prefs.putString(PREF_PORT_STR, server.arg("port"));
  }
  if (server.hasArg("temp_topic")) {
    prefs.putString(PREF_TEMP_TOPIC, server.arg("temp_topic"));
  }
  if (server.hasArg("ping_topic")) {
    prefs.putString(PREF_PING_TOPIC, server.arg("ping_topic"));
  }
  if (server.hasArg("threshold_h")) {
    prefs.putString(PREF_THRESHOLD_H_STR, server.arg("threshold_h"));
  }
  if (server.hasArg("threshold_l")) {
    prefs.putString(PREF_THRESHOLD_L_STR, server.arg("threshold_l"));
  }
  
  prefs.end();
  
  server.send(200, "text/plain", "Saved. Rebooting...");
  delay(1000);
  ESP.restart();
  //server.send(400, "text/plain", "Missing parameters");
}

void createHtml() {
  Html.clear();
  Html += FORM_HTML;
  Preferences prefs;
  prefs.begin(PREF_NAMESPACE, true); // read mode
  
  String ssid = prefs.getString(PREF_SSID, "");
  Html.replace("%SSID%", ssid.c_str());
  
  String pass = prefs.getString(PREF_PASS, "");
  if (pass.length() > 0) {
    pass.clear();
    pass += PASS_MASK;
  }
  Html.replace("%PASSWORD%", pass.c_str());
  
  String broker = prefs.getString(PREF_BROKER, "");
  Html.replace("%BROKER%", broker.c_str());
  
  String port_str = prefs.getString(PREF_PORT_STR, "");
  Html.replace("%PORT%", port_str.c_str());

  String temp_topic = prefs.getString(PREF_TEMP_TOPIC, "");
  Html.replace("%TEMP_TOPIC%", temp_topic.c_str());
  
  String ping_topic = prefs.getString(PREF_PING_TOPIC, "");
  Html.replace("%PING_TOPIC%", ping_topic.c_str());
  
  String highest = prefs.getString(PREF_THRESHOLD_H_STR, "");
  Html.replace("%THRESHOLD_H%", highest.c_str());

  String lowest = prefs.getString(PREF_THRESHOLD_L_STR, "");
  Html.replace("%THRESHOLD_L%", lowest.c_str());
}

void startConfigServer(const char *ap_ssid, const char *ap_pass) {
  WiFi.mode(WIFI_MODE_AP);
  if (!WiFi.softAP(ap_ssid, ap_pass)) {
    M5.Lcd.println("ap_mode: Failed");
    while(1);
  }
  delay(100); // ?????
  
  // Startup Web server
  createHtml();
  WebServer& server = WebServerInstance::getInstance();
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  while (1) {
    // In AP mode we need to service the web server
    if (WiFi.getMode() == WIFI_AP) {
      server.handleClient();
    }
    delay(10);
  }
}
