#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>

#define PREF_NAMESPACE "config"

const char PREF_SSID[] = "ssid";
const char PREF_PASS[] = "pass";
const char PREF_BROKER[] = "broker";
const char PREF_PORT_STR[] = "port";
const char PREF_TEMP_TOPIC[] = "temp_topic";
const char PREF_PING_TOPIC[] = "ping_topic";
const char PREF_THRESHOLD_H_STR[] = "threshold_h";
const char PREF_THRESHOLD_L_STR[] = "threshold_l";
const char PASS_MASK[] = "********";

extern void startConfigServer(const char *ap_ssid, const char *ap_pass);

const char FORM_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wi-Fi / MQTT Settings</title>
  <style>
    :root {
      --bg-color: #f4f6f8;
      --card-color: #ffffff;
      --accent-color: #0078d7;
      --text-color: #333;
      --border-color: #ccc;
    }
    body {
      margin: 0;
      padding: 0;
      background-color: var(--bg-color);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
      color: var(--text-color);
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      padding: 16px;
    }
    h1 {
      font-size: 1.6rem;
      text-align: center;
      margin-bottom: 16px;
    }
    .card {
      background-color: var(--card-color);
      border-radius: 12px;
      padding: 20px;
      box-shadow: 0 4px 10px rgba(0,0,0,0.08);
      margin-bottom: 16px;
    }
    .section-header {
      font-size: 1.3rem;
      font-weight: bold;
      margin-bottom: 16px;
      padding-bottom: 8px;
      border-bottom: 2px solid var(--accent-color);
      cursor: pointer;
      user-select: none;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .section-header:hover {
      color: var(--accent-color);
    }
    .toggle-icon {
      font-size: 1.2rem;
      transition: transform 0.3s;
    }
    .toggle-icon.collapsed {
      transform: rotate(-90deg);
    }
    .section-content {
      max-height: 1000px;
      overflow: hidden;
      transition: max-height 0.3s ease-out;
    }
    .section-content.collapsed {
      max-height: 0;
    }
    .form-group {
      margin-bottom: 18px;
    }
    label {
      display: block;
      font-size: 1.1rem;
      margin-bottom: 6px;
      font-weight: 600;
    }
    input {
      width: 100%;
      font-size: 1.2rem;
      padding: 12px 14px;
      border-radius: 8px;
      border: 1px solid var(--border-color);
      box-sizing: border-box;
    }
    input:focus {
      outline: none;
      border-color: var(--accent-color);
    }
    .hint {
      font-size: 0.9rem;
      color: #666;
      margin-top: 4px;
    }
    .button {
      width: 100%;
      font-size: 1.3rem;
      padding: 14px;
      border-radius: 10px;
      border: none;
      background-color: var(--accent-color);
      color: #fff;
      font-weight: bold;
      cursor: pointer;
      margin-top: 10px;
    }
    .button:active {
      opacity: 0.85;
    }
    footer {
      text-align: center;
      font-size: 0.85rem;
      color: #777;
      margin-top: 12px;
    }
    @media (min-width: 768px) {
      h1 { font-size: 1.8rem; }
      input { font-size: 1.25rem; }
      .button { font-size: 1.4rem; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Wi-Fi / MQTT Settings</h1>
    <form method="POST" action="/save">
      <div class="card">
        <div class="section-header" onclick="toggleSection('wifi')">
          <span>Wi-Fi Settings</span>
          <span class="toggle-icon" id="wifi-icon">▼</span>
        </div>
        <div class="section-content" id="wifi-content">
          <div class="form-group">
            <label for="ssid">Wi-Fi SSID</label>
            <input type="text" id="ssid" name="ssid" value="%SSID%">
            <div class="hint">Network name of the Wi-Fi access point</div>
          </div>
          <div class="form-group">
            <label for="password">Wi-Fi Password</label>
            <input type="password" id="password" name="password" value="%PASSWORD%">
            <div class="hint">Leave blank for open networks</div>
          </div>
        </div>
      </div>
      <div class="card">
        <div class="section-header" onclick="toggleSection('mqtt')">
          <span>MQTT Settings</span>
          <span class="toggle-icon" id="mqtt-icon">▼</span>
        </div>
        <div class="section-content" id="mqtt-content">
          <div class="form-group">
            <label for="broker">MQTT Broker IP Address</label>
            <input type="text" id="broker" name="broker" value="%BROKER%" placeholder="e.g. 192.168.1.10">
          </div>
          <div class="form-group">
            <label for="port">MQTT Port</label>
            <input type="number" id="port" name="port" value="%PORT%">
          </div>
          <div class="form-group">
            <label for="temp-topic">Temperature Topic</label>
            <input type="text" id="temp-topic" name="temp_topic" value="%TEMP_TOPIC%" placeholder="e.g. monitor/temperature">
          </div>
          <div class="form-group">
            <label for="ping-topic">Ping Topic</label>
            <input type="text" id="ping-topic" name="ping_topic" value="%PING_TOPIC%" placeholder="e.g. monitor/rttmon">
          </div>
        </div>
      </div>
      <div class="card">
        <div class="section-header" onclick="toggleSection('temp')">
          <span>Temperature Settings</span>
          <span class="toggle-icon" id="temp-icon">▼</span>
        </div>
        <div class="section-content" id="temp-content">
          <div class="form-group">
            <label for="highest-threshold">Highest Temperature Threshold</label>
            <input type="text" id="highest-threshold" name="threshold_h" value="%THRESHOLD_H%" placeholder="e.g. 30.0">
          </div>
          <div class="form-group">
            <label for="lowest-threshold">Lowest Temperature Threshold</label>
            <input type="text" id="lowest-threshold" name="threshold_l" value="%THRESHOLD_L%" placeholder="e.g. -4.0">
          </div>
        </div>
      </div>
      <button class="button" type="submit">Save & Reboot</button>
    </form>
    <footer>M5StickC Plus2 Configuration Page</footer>
  </div>
  <script>
    function toggleSection(section) {
      const content = document.getElementById(section + '-content');
      const icon = document.getElementById(section + '-icon');
      content.classList.toggle('collapsed');
      icon.classList.toggle('collapsed');
    }
  </script>
</body>
</html>
)rawliteral";
#endif // CONFIG_H
