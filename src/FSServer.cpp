#include "FSServer.h"
#include <URLCode.h>


String renderHTML(String body, String path) {
	return "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>file system</title></head><body><h1>Directory listing for " + path + "</h1><hr><ul>" + body + "</ul><hr>  <h3 style='margin-top: 50px'>Select File to Upload</h3><form action='/upload?path=" + path + "' method='post' enctype='multipart/form-data'><input class='buttons' style='width:40%' type='file' name='upload' id='upload' value='' required><br><br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br></form>  </body></html>";
};

String renderA(String data, String url, bool isFile=true) {
	return "<li><a href=\"" + url + "\">" + data + "</a>" + (!isFile?"":("<form style='display: inline; margin-left: 20px;' action='/delete' method='post'><input type='hidden' name='filename' value='" + url + "'><button class='buttons' style='width:10%' type='submit'>delete</button></form>")) + "</li>";
};

ESP32WebServer* server = nullptr;
String root = "";

FSServer::~FSServer() {
	if (server != nullptr) {
		delete server;
	}
};

void FSServer::begin(const char* ssid, const char* password) {
	WiFi.mode(WIFI_STA);
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

void handleFileList(){
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

  if (!fileDir) {
  	return;
  }
  if (fileDir.isDirectory()) {
  	int rootLength = filename.length();
  	// Serial.println(filename.length());
  	if (filename != "/") {
  		rootLength = rootLength + 1;
	  }

    File file = fileDir.openNextFile();
    String resp = "";
    String CachFiles = ",";
    while (file) {
    	String p = file.path();
    	String fileN = file.name();
    	String splitedHeadPath = p.substring(rootLength, p.length());
    	if (splitedHeadPath == fileN) {
    		int sizeKB = file.size() / 1024;
      		resp += renderA(fileN + " (" + (sizeKB>0?(String(sizeKB) + "KB)"):(String(file.size()) + "B)")), p) + "</br>";
    	} else {
    		String dir = splitedHeadPath.substring(0, splitedHeadPath.indexOf("/"));
    		if (CachFiles.indexOf("," + dir + ",") < 0) {
    			resp += renderA(dir, filename + dir, false) + "</br>";
    			CachFiles += dir + ",";
    		}
    	}
      file = fileDir.openNextFile();
    }
    fileDir.close();

    server->sendHeader("Content-Type", "text/html"); 
    server->sendHeader("Pragma", "no-cache"); 
    server->sendHeader("Expires", "-1");
    resp = renderHTML(resp, filename);
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
};

String getRedirectLocation(String filename) {
	String path = "";
	int i = filename.indexOf("/");
	while (i >= 0) {
		path += filename.substring(0, i+1);
		filename = filename.substring(i+1, filename.length());
		i = filename.indexOf("/");
	}
	if (path == "") {
		path = "/";
	}
	return path;
};

String getFileABSPath(String filename, String prePath="") {
	if(filename.startsWith("/")) {
		filename = filename.substring(1, filename.length());
	}
	if (!prePath.endsWith("/")) {
		prePath = prePath + "/";
	}
	filename = prePath + filename;
	if(!filename.startsWith("/")) {
		filename = "/" + filename;
	}
	return filename;
};

File UploadFile; 
void handleFileUpload() {
	HTTPUpload& uploadfile = server->upload();
	String filename = uploadfile.filename;
	String path = server->arg("path");
	filename = getFileABSPath(filename, path);
	Serial.print("Upload File Name: ");
	Serial.println(filename);
	if(uploadfile.status == UPLOAD_FILE_START) {
		SPIFFS.remove(filename);
		UploadFile = SPIFFS.open(filename, FILE_WRITE);
	} else if (uploadfile.status == UPLOAD_FILE_WRITE) {
		if (uploadfile.currentSize == 0) {
			SPIFFS.remove(filename);
			Serial.println("size 0");
			server->sendHeader("Location", getRedirectLocation(filename)); 
			server->send(302);
			return;
		}
		if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
	} else if (uploadfile.status == UPLOAD_FILE_END) {
		if(UploadFile){                                    
			UploadFile.close();
			
			Serial.print("Upload Size: ");
			Serial.println(uploadfile.totalSize);
			
			server->sendHeader("Location", getRedirectLocation(filename)); 
			server->send(302);
		} else {
			Serial.println("error todo");
			server->send(400);
		}
	}
}

void handleFileDelete() {
	String filename = server->arg("filename");
	// filename = getFileABSPath(filename);

	Serial.print("filename: ");
	Serial.println(filename);

	SPIFFS.remove(filename);
	server->sendHeader("Location", getRedirectLocation(filename)); 
	server->send(302);
};

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

	if(SPIFFS.exists("/")) {
		SPIFFS.remove("/"); // clear
	}

	server = new ESP32WebServer(port);

	server->on("/upload",  HTTP_POST, [](){ server->send(200);}, handleFileUpload);
	server->on("/delete",  HTTP_POST, handleFileDelete);
	server->onNotFound(handleFileList);
	server->begin();
	Serial.println("HTTP server started");
	Serial.print("listening: ");
	Serial.print(localIP);
	Serial.print(":");
	Serial.print(port);
};