#if !defined(SALESFORCE_MANAGER_H)
#define SALESFORCE_MANAGER_H

/*
  SFManager.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

//#include <ESPUtils.h>
#include <WiFiManager.h>

enum UtilSFRARequestState { sf_idle, sf_flow, sf_platform };
enum UtilSFRATokenState { sf_empty, sf_request, sf_refresh, sf_valid };

typedef void (*SFManagerCallback)(String data, bool success);

//typedef void (*SFManagerTokenCallback)(UtilMessage message);

//------------------------------------------------------------------------------------
class SFManager{
public:
  unsigned long retryTime;
  unsigned long refreshTime;
  UtilSFRARequestState  requestState = sf_idle;
  UtilSFRATokenState    tokenState = sf_empty;

  String token = "";
  String instance = "";

  int  hostPort = 443;
  String authHost = "login.salesforce.com";

  void requestToken();
  void refreshToken();
  void flowRequest(SFManagerCallback callback);
  void platformRequest(SFManagerCallback callback);
  void loop();
  
  //SFManager(){requestToken();};

//protected:

private:
  SFManagerCallback tokenCallback;
  SFManagerCallback refreshCallback;
  SFManagerCallback flowCallback;
  SFManagerCallback platformCallback;
  
  bool verifyConnection();
  bool verifyToken();
  void setNeedsRetry();
  void setNeedsRefresh();
  
  void flowRequest();
  void platformRequest();
};

SFManager SFMan;

//------------------------------------------------------------------------------------
void SFManager::setNeedsRetry(){
  retryTime = SFRA_RETRY_DELAY + millis();
}

//------------------------------------------------------------------------------------
void SFManager::setNeedsRefresh(){
  refreshTime = SFRA_REFRESH_DELAY + millis();
}

//------------------------------------------------------------------------------------
bool SFManager::verifyConnection(){
  #ifndef USE_WIFI
  Serial.println("ERROR: ESPUtils - WIFIManager undefined, please add #define USE_WIFI and/or #define USE_SFRA prior to including ESPUtils.h")
  return false;
  #endif

  if( WiFiManager::state == connected ){
    Serial.println("WiFi : Connected");
    return true;
  }
  
  Serial.println("WiFi : Not Ready.");
  if( WiFiManager::state == idle ){ 
    WiFiManager::beginStation();
  }

  setNeedsRetry();
  return false;
}

//------------------------------------------------------------------------------------
bool SFManager::verifyToken(){
  if( tokenState == sf_valid){
    Serial.println("SFManager : Valid token.");
    return true;
  }
    
  if(tokenState == sf_empty)
  {
    Serial.println("SFManager : Requesting token");
    //requestToken();
  }

  if(tokenState == sf_request || tokenState == sf_refresh){
    Serial.println("SFManager : Waiting for token");
  }

  requestToken();
  return false;
}

//------------------------------------------------------------------------------------
void SFManager::requestToken(){
  tokenState = sf_request;
  
  if(!verifyConnection()){
    return;
  }

  String requestBody = "grant_type=password&client_id=" + String(SFRA_CLIENT_ID) + 
    "&client_secret=" + String(SFRA_CLIENT_SECRET) + 
    "&username=" + String(SFRA_USER) + 
    "&password=" + String(SFRA_PASS);
  Serial.println(requestBody);

  WiFiClientSecure client;
  Serial.printf("Connecting to %s... ", authHost.c_str());
  if (!client.connect(authHost.c_str(), hostPort)) {
    Serial.println("Failed.");
    setNeedsRetry();
    client.stop();
    return;
  } 
  Serial.println("Success.\n");
  
  client.println("POST /services/oauth2/token HTTP/1.1");
  client.println("Host: "+ authHost);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Content-Length: "+ String(requestBody.length(), DEC));
  client.println();
  client.println(requestBody);

  Serial.print("Authentication result... ");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Valid.");
      break;
    }
  }
  Serial.println("");

  Serial.println("Reading token.");
  String line = client.readString();
  if (line.indexOf("\"access_token\":\"") != -1) {
    setNeedsRefresh();
    tokenState = sf_valid;
    token = line.substring(line.indexOf("\"access_token\":\"") + 16, line.indexOf("\",\"instance_url\""));
    instance = line.substring(line.indexOf("\"instance_url\":\"") + 16 + 8, line.indexOf("\",\"id\""));
    Serial.println("Auth Token: " + token);
    Serial.println("Org Instance: " + instance);
  } else {
    Serial.println("Failed to parse token.");
  }

  client.stop();  
}

//------------------------------------------------------------------------------------
void SFManager::refreshToken(){
  token = "";
  Serial.println("Refresh");
  //revokeToken();
  requestToken();
  /*
  state = refreshing;

  if(!verifyConnection()){
    return;
  }

  if(token.isEmpty()){
    requestToken();
    state = refreshing;
    return;
  }

  WiFiClientSecure client;
  String requestBody = "grant_type=refresh_token&client_id=" + String(SFRA_CLIENT_ID) + 
    "&client_secret=" + String(SFRA_CLIENT_SECRET) + 
    "&refresh_token=" + token;

  Serial.printf("Connecting to %s... ", SFRA_HOST);
  
  if (!client.connect(SFRA_HOST, SFRA_PORT)) {
    Serial.println("Failed.");
    setNeedsRetry();
    return;
  } else {
    Serial.println("Success.");
  }
 
  client.println("POST /services/oauth2/token HTTP/1.1");
  client.println("Host: " + String(SFRA_HOST));
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(requestBody.length());
  client.println();
  client.println(requestBody);

  Serial.print("Refresh request sent, waiting for result... ");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Done.");
      break;
    }
  }
  
  Serial.print("Reading token... ");
  String line = client.readString();
  Serial.println(line);
  if (line.indexOf("\"access_token\":\"") != -1) {
    Serial.println("Success.");
    setNeedsRefresh();
    token = line.substring(line.indexOf("\"access_token\":\"") + 16, line.indexOf("\",\"instance_url\""));
    Serial.println(token);
    //instance = line.substring(line.indexOf("\"instance_url\":\"") + 16 + 8, line.indexOf("\",\"id\""));
  } else {
    Serial.println("Failed.");
    setNeedsRetry();
  }

  client.stop();  
  */
}

//------------------------------------------------------------------------------------
void SFManager::flowRequest(SFManagerCallback callback){
  if(platformCallback != nullptr){
    Serial.println("SFManager : Pending Flow Request");
  }

  flowCallback = callback;
  flowRequest();
}

//------------------------------------------------------------------------------------
void SFManager::flowRequest(){
  requestState = sf_flow;

  if(!verifyToken()){
    setNeedsRetry();
    return;
  }
  
  String flow = "CloudBoard_IoT_Connector";
  String requestBody = "{\"inputs\":[{\"spaceid\":\"77262\"}]}";

  Serial.println(requestBody);

  WiFiClientSecure client;
  Serial.printf("Connecting to %s... ", instance.c_str());
  if (!client.connect(instance.c_str(), hostPort)) {
    Serial.println("Failed.");
    client.stop();
    setNeedsRetry();
    return;
  } 
  Serial.println("Success.");

  client.println("POST /services/data/"+ String(SFRA_API) +"/actions/custom/flow/"+ flow +" HTTP/1.1");
  client.println("Host: "+ instance);
  client.println("Content-Type: application/json");
  client.println("Content-Length: "+ String(requestBody.length(), DEC));
  client.println("Authorization: Bearer "+ token);
  client.println();
  client.println(requestBody);

  Serial.print("Flow result... ");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.print(line);
    if (line == "\r") {
      Serial.print("Done.");
      break;
    }
  }
  Serial.println("");

  Serial.print("Reading result... ");
  String line = client.readString();
  Serial.println("");
  Serial.println(line);

  if (line.indexOf("\"success\":true") != -1) {
    Serial.println("Success.");
    flowCallback("Data",true);
  } else {
    Serial.println("Failed.");
    flowCallback("",false);
  }

  //{"message":"Unexpected character ('j' (code 106)): was expecting a colon to separate field name and value at [line:1, column:18]","errorCode":"JSON_PARSER_ERROR"}
  //{"id":"e09xx0000000001AAA","success":true,"errors":[{"statusCode":"OPERATION_ENQUEUED","message":"48eedfc5-f9eb-4fbb-8cba-2d49f9f98022","fields":[]}]}

  // if (line.indexOf("\"access_token\":\"") != -1) {
  //   Serial.println("Success.");
  //   setNeedsRefresh();
  //   token = line.substring(line.indexOf("\"access_token\":\"") + 16, line.indexOf("\",\"instance_url\""));
  //   Serial.println("Auth Token: " + token);
  //   instance = line.substring(line.indexOf("\"instance_url\":\"") + 16 + 8, line.indexOf("\",\"id\""));
  //   Serial.println("Org Instance: " + instance);
  // } else {
  //   Serial.println("Failed.");
  //   setNeedsRetry();
  // }

  client.stop(); 
  requestState = sf_idle;
  flowCallback = nullptr;
}


//------------------------------------------------------------------------------------
void SFManager::platformRequest(SFManagerCallback callback){
  if(platformCallback != nullptr){
    Serial.println("SFManager : Pending Platform Request");
  }

  platformCallback = callback;
  platformRequest();
}

//------------------------------------------------------------------------------------
void SFManager::platformRequest(){
  requestState = sf_platform;

  if(!verifyToken()){
    setNeedsRetry();
    return;
  }
  
  String platformEvent = "TestPlatformEvent__e";
  String eventField = "jandolina@salesforce.com";
  String eventPayload = "Hello, Are you comfortable with recieveing emails from Platform Events? Yours Truly, The Microcontroller";
  String requestBody = "{\"TestField__c\":\""+ eventField +"\",\"TestPayload__c\":\""+eventPayload+"\"}";

  Serial.println(requestBody);

  WiFiClientSecure client;
  Serial.printf("Connecting to %s... ", instance.c_str());
  if (!client.connect(instance.c_str(), hostPort)) {
    Serial.println("Failed.");
    client.stop();
    setNeedsRetry();
    return;
  } 
  Serial.println("Success.");

  client.println("POST /services/data/"+ String(SFRA_API) +"/sobjects/"+ platformEvent +" HTTP/1.1");
  client.println("Host: "+ instance);
  client.println("Content-Type: application/json");
  client.println("Content-Length: "+ String(requestBody.length(), DEC));
  client.println("Authorization: Bearer "+ token);
  client.println();
  client.println(requestBody);

  Serial.print("Platform Event result... ");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.print(line);
    if (line == "\r") {
      Serial.print("Done.");
      break;
    }
  }
  Serial.println("");

  Serial.print("Reading result... ");
  String line = client.readString();
  Serial.println("");
  Serial.println(line);

  if (line.indexOf("\"success\":true") != -1) {
    Serial.println("Success.");
    platformCallback("Data",true);
  } else {
    Serial.println("Failed.");
    platformCallback("",false);
  }

  client.stop(); 
  requestState = sf_idle;
  platformCallback = nullptr;
}

//------------------------------------------------------------------------------------
void SFManager::loop(){  
  if (refreshTime != 0 && refreshTime < millis()){
    refreshTime = 0;
    refreshToken();
    return;
  }

  if (retryTime != 0 && retryTime < millis()){
    retryTime = 0;
    switch(requestState) {
      case sf_flow:
        Serial.println("SFManager : Flow Request");
        flowRequest();
        break;
      case sf_platform:
        Serial.println("SFManager : Platform Request");
        platformRequest();
        break;
      default:
      break;
    }
  }
}
#endif