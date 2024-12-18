#include "FSServer.h"
#include <URLCode.h>


String renderHTML(String body) {
	return "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"UTF-8\"><title>file system</title></head><body><h1>Directory listing for /</h1><hr><ul>" + body + "</ul><hr></body></html>";
};

String renderA(String data, String url) {
	return "<li><a href=\"" + url + "\">" + data + "</a></li>";
};

ESP32WebServer* server = nullptr;
String root = "";

FSServer::~FSServer() {
	if (server != nullptr) {
		delete server;
	}
};

void FSServer::begin(const char* ssid, const char* password) {
	int status = WiFi.begin(ssid, password);
	while (
		!(
		status == WL_CONNECTED || status == WL_CONNECT_FAILED
		)
	) {
		Serial.println("WiFi connecting...");
		delay(100);
		status = WiFi.status();
		Serial.print("WiFi status: ");
		Serial.print(status);
	}
	if (status == WL_CONNECTED) {
		localIP = WiFi.localIP();
		Serial.print("WiFi connected ip: ");
		Serial.println(localIP);
		Serial.print("WiFi dns: ");
		Serial.println(WiFi.dnsIP());
	} else {
		Serial.print("WiFi not connected, status: ");
		Serial.println(status);
		assert(false);
	}
};

void FSServer::handleClient() {
	server->handleClient();
};

void list(){
  Serial.println();
  String uri = server->uri();
  URLCode url;
  url.urlcode = uri;
  url.urldecode();
  uri = url.strcode;

  if (uri.startsWith("/")) {
  	uri = uri.substring(1, uri.length());
  }
  if (uri.endsWith("/")) {
  	uri = uri.substring(0, uri.length()-1);
  }
  String filename = root + uri;
  File fileDir = SPIFFS.open(filename);

  Serial.print("list filedir: ");
  Serial.println(filename);

  if (!fileDir) {
  	return;
  }
  if (fileDir.isDirectory()) {
  	int rootLength = filename.length();
  	Serial.println(filename.length());
  	if (filename != "/") {
  		rootLength = rootLength + 1;
	  }

    File file = fileDir.openNextFile();
    String resp = "";
    String CachFiles = ",";
    while (file) {
    	String p = file.path();
    	String fileN = file.name();

    	Serial.println(rootLength);

    	String splitedHeadPath = p.substring(rootLength, p.length());

    	Serial.print("path: ");
    	Serial.print(p);
    	Serial.print(" fileName: ");
    	Serial.print(fileN);
    	Serial.print(" pathIndex: ");
    	Serial.println(splitedHeadPath);

    	if (splitedHeadPath == fileN) {
      	resp += renderA(fileN + " (" + String(file.size() / 1024) + "KB)", p) + "</br>";
    	} else {
    		String dir = splitedHeadPath.substring(0, splitedHeadPath.indexOf("/"));

    		Serial.print("dir: ");
    		Serial.println(dir);

    		if (CachFiles.indexOf("," + dir + ",") < 0) {
    			resp += renderA(dir, filename + dir) + "</br>";
    			CachFiles += dir + ",";
    		}
    	}
      file = fileDir.openNextFile();
    }
    fileDir.close();

    server->sendHeader("Content-Type", "text/html"); 
    server->sendHeader("Pragma", "no-cache"); 
    server->sendHeader("Expires", "-1");
    resp = renderHTML(resp);
    server->setContentLength(resp.length()); 
    server->send(200, "text/html", ""); 
    server->sendContent(resp);
    return;
  }
  server->sendHeader("Content-Type", "text/text");
  server->sendHeader("Content-Disposition", String("attachment; filename=")+String(fileDir.name()));
  server->sendHeader("Connection", "close");
  server->streamFile(fileDir, "application/octet-stream");
  fileDir.close();
}

void FSServer::runSPIFFS(int port, const char* r) {
	Serial.print(F("Initializing SPIFFS ...")); 
	if (!SPIFFS.begin(true)) {
		Serial.println(F("SPIFFS failed ..."));
		assert(false);
	} else {
		Serial.println(F("SPIFFS initialised... file access enabled..."));
	}
	root = String(r);

	if (!root.startsWith("/")) {
		root = "/" + root;
	}
	if (!root.endsWith("/")) {
		root += "/";
	}
	server = new ESP32WebServer(port);

	server->onNotFound(list);
	server->begin();
	Serial.println("HTTP server started");
	Serial.print("listening: ");
	Serial.print(localIP);
	Serial.print(":");
	Serial.print(port);
};