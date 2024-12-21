#include "FS.h"
#include <SPI.h>
#include <WiFi.h> 
#include <SPIFFS.h>
#include <ESP32WebServer.h>
#include <cassert>

#define EVENT_TYPE int
#define DOWNLOAD_EVENT 1
#define VIEW_PATH_EVENT 2
#define UPLOAD_EVENT 3
#define DELETE_EVENT 4

typedef void (*EVENT_CALLBACK)(const String&, EVENT_TYPE);

class FSServer {
private:
  IPAddress localIP;
public:
  ~FSServer();
  void begin(const char* ssid, const char* password);
  void runSPIFFS(int port, const char* root);
  void setCallback(EVENT_CALLBACK);
  void handleClient();
};