/*
  OTAManager.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#if !defined(OTA_MANAGER_H)
#define OTA_MANAGER_H

#include <UtilCommon.h>
#include <UtilMessage.h>

#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

//Server Index Page
//------------------------------------------------------------------------------------
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'><input type='file' name='update'><BR><input type='submit' value='Update'></form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {}"
 "});"
 "});"
 "</script>";

//------------------------------------------------------------------------------------

class OTAManager {
public:
  WebServer *otaServer;
  UtilMessageCallback otaCallback;
  unsigned long updateInterval = 5000;
  
  OTAManager(){};
  void begin(UtilMessageCallback callback);
  void begin(UtilMessageCallback callback, String updatePath);
  void loop(void);

private:
  bool otaActive = false;
  unsigned long update = 0;

  void beginOTA(UtilMessageCallback callback, String updatePath = "");
  void localUpdate();
  void remoteUpdate(String updatePath);
  String getHost();
};

static OTAManager *OTAMan;

String OTAManager::getHost(){
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X%02X", baseMac[4], baseMac[5]);
  return "ESP32-" + String(baseMacChr);
}

//------------------------------------------------------------------------------------
void OTAManager::begin(UtilMessageCallback callback){
  beginOTA(callback);
}

//------------------------------------------------------------------------------------
void OTAManager::begin(UtilMessageCallback callback, String updatePath){
  beginOTA(callback, updatePath);
}

//------------------------------------------------------------------------------------
void OTAManager::beginOTA(UtilMessageCallback callback, String updatePath) {
  otaCallback = callback;

  if( updatePath.isEmpty() ){
    Serial.println("Starting local OTA update.");
    localUpdate();
  } else {
    Serial.println("Starting remote OTA update.");
    remoteUpdate(updatePath);
  }
}

//------------------------------------------------------------------------------------
void OTAManager::localUpdate(){
  // Make the access point
  String host = getHost();
  Serial.println("Host:" + String(host));
  
  otaServer = new WebServer(80);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(host.c_str(),"",6,0,5);
  Serial.print("OTA Server address:\t");
  Serial.println(WiFi.softAPIP());

  otaServer->on("/", HTTP_GET, [&]() {
    otaServer->sendHeader("Connection", "close");
    otaServer->send(200, "text/html", serverIndex);
  });

  // Setting up the server update route
  otaServer->on("/update", HTTP_POST, [&]() {
    otaServer->sendHeader("Connection", "close");
    otaServer->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    otaCallback({NFO_KEY, "Success"});

    delay(5000);
    otaCallback({ESP_RESTART});
  }, [&]() {
    HTTPUpload& upload = otaServer->upload();
    if (upload.status == UPLOAD_FILE_START) 
    {
      otaCallback({NFO_KEY, upload.filename.c_str()});
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) //start with max available size
      { 
        Update.printError(Serial);
      }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if(update < millis()){
        update = millis() + updateInterval;
        otaCallback({NFO_KEY, String(upload.totalSize/upload.currentSize).c_str()});
      }
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } 
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        otaCallback({NFO_KEY, String(upload.totalSize).c_str()});
      } 
      else {
        Update.printError(Serial);
      }
    }
  });

  otaServer->begin();
  otaActive = true;
  otaCallback({NFO_KEY, WiFi.softAPIP().toString().c_str()});
  Serial.println("OTA Server Running");
}

//------------------------------------------------------------------------------------
void OTAManager::remoteUpdate(String updatePath){
  // TODO : Setup how and when the SSID and PW are introduced
  WiFi.mode(WIFI_STA);
  WiFi.begin("SSID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP address: "+ String(WiFi.localIP()));
  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, updatePath);
  // Or:
  //t_httpUpdate_return ret = httpUpdate.update(client, "server", 80, "file.bin");

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }

  otaActive = true;
}

//------------------------------------------------------------------------------------
void OTAManager::loop(void) {
  if(otaActive){
    otaServer->handleClient();
  }
}

#endif