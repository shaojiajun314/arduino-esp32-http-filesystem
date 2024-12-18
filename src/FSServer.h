#include "FS.h"
#include <SPI.h>
#include <WiFi.h> 
#include <SPIFFS.h>
#include <ESP32WebServer.h>
#include <cassert>

class FSServer {
private:
  IPAddress localIP;
public:
  ~FSServer();
  void begin(const char* ssid, const char* password);
  void runSPIFFS(int port, const char* root);
  void handleClient();
};