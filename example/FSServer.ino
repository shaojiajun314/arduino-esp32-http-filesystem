#include "FSServer.h"


const char ssid[] = "helloworld";
const char password[] = "123456";

FSServer fsserver;

void event(const String& path, EVENT_TYPE type) {
	Serial.print("path:");
	Serial.print(path);
	Serial.print(" type: ");
	Serial.println(type);
};

void setup(){
	Serial.begin(9600);
	delay(500);
	fsserver.begin(ssid, password);
	fsserver.setCallback(&event);
	fsserver.runSPIFFS(80, "/");
}

void loop(){
	fsserver.handleClient();
}
