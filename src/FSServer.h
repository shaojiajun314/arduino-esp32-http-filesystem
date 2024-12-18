#include "FS.h"
#include <SPI.h>
#include <WiFi.h>              // Built-in
#include <SPIFFS.h>
#include <ESP32WebServer.h>    // https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
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