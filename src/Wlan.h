#ifndef GETWIFI_H
#define GETWIFI_H

#include <Preferences.h>
#include <WebServer.h>

class Wlan {
public:
  String staSSID;
  String staPASS;
  IPAddress myIP;
  Wlan();
  bool connect();
  bool credentialExists();
  void show();
};

#endif // GETWIFI_H
