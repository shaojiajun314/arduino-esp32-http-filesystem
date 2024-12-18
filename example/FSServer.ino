#include "FSServer.h"


const char ssid[] = "nbxxx";
const char password[] = "sbk123456";

FSServer fsserver;

void setup(){
	Serial.begin(9600);
	delay(100);
	fsserver.begin(ssid, password);
	fsserver.runSPIFFS(80, "/");
}

void loop(){
	fsserver.handleClient();
}
