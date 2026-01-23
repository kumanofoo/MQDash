#ifndef WEB_SERVER_INSTANCE_H
#define WEB_SERVER_INSTANCE_H

#include <WebServer.h>

class WebServerInstance {
 public:
  static WebServer& getInstance() {
    static WebServer instance(80);
    return instance;
  }
 private:
  WebServerInstance() {}
  WebServerInstance(const WebServerInstance&) = delete;
  WebServerInstance& operator=(const WebServerInstance&) = delete;
};

#endif // WEB_SERVER_INSTANCE_H
