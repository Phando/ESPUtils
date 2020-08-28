#if !defined(SALESFORCE_MANAGER_H)
#define SALESFORCE_MANAGER_H

/*
  SFManager.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#include <WiFiManager.h>
#define UTILS_SF_CAPACITY 20
#define UTILS_SF_READ_TIME 2000

enum UtilSFRATokenState { 
  sf_token_empty, 
  sf_token_request, 
  sf_token_refresh, 
  sf_token_valid
};

enum UtilSFRARequestType { 
  sf_type_flow, 
  sf_type_event
};

typedef void (*UtilSFRACallback)(bool success, String payload);

struct UtilSFRARequest {
  UtilSFRARequestType type;
  String targetName; 
  String requestBody;
  UtilSFRACallback callback;
} ;

//------------------------------------------------------------------------------------
class SFManager{
public:
  unsigned long retryTime = millis();
  unsigned long refreshTime = 0;
  UtilSFRATokenState    tokenState = sf_token_empty;

  int pushIndex = 0;
  int popIndex = 0;
  UtilSFRARequest requestList[UTILS_SF_CAPACITY];

  String token = "";
  String instance = "";

  int  hostPort = 443;
  String authHost = "login.salesforce.com";

  bool pendingRequests();
  void requestToken();
  void refreshToken();
  void eventRequest(String eventName, String requestBody, UtilSFRACallback callback);
  void flowRequest(String flowName, String requestBody, UtilSFRACallback callback);
  void loop();

//protected:

private:
  WiFiClientSecure getAuthClient();
  WiFiClientSecure getInstanceClient();
  int nextIndex(int index);
  bool verifyConnection();
  bool verifyToken();
  void setNeedsRetry();
  void setNeedsRefresh();
  
  void executeRequest();
  bool executeEventRequest(String eventName, String requestBody, UtilSFRACallback callback);
  bool executeFlowRequest(String flowName, String requestBody, UtilSFRACallback callback);
  void scheduleRequest(UtilSFRARequestType type, String flowName, String requestBody, UtilSFRACallback callback);
};

SFManager SFMan;

//------------------------------------------------------------------------------------
bool SFManager::pendingRequests(){
  return nextIndex(popIndex) == pushIndex;
}

//------------------------------------------------------------------------------------
int SFManager::nextIndex(int index){
  return ++index == UTILS_SF_CAPACITY ? 0 : index;
}

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

  //setNeedsRetry();
  return false;
}

//------------------------------------------------------------------------------------
bool SFManager::verifyToken(){
  Serial.println("Verify Token");

  if( tokenState == sf_token_valid){
    Serial.println("SFManager : Valid token.");
    return true;
  }
    
  if(tokenState == sf_token_empty)
  {
    Serial.println("SFManager : Requesting token");
  }

  if(tokenState == sf_token_request || tokenState == sf_token_refresh){
    Serial.println("SFManager : Waiting for token");
  }

  requestToken();
  return false;
}

//------------------------------------------------------------------------------------
WiFiClientSecure SFManager::getAuthClient(){
  WiFiClientSecure client;
  Serial.println("GetAuthClient");
  
  if(!verifyConnection()){
     return client;
  }

  Serial.printf("Connecting to %s... ", authHost.c_str());
  
  if (!client.connect(authHost.c_str(), hostPort)) {
    Serial.println("Failed.");
    client.stop();
    return client;
  } 

  Serial.printf("Success.");
  return client;
}

//------------------------------------------------------------------------------------
WiFiClientSecure SFManager::getInstanceClient(){
  WiFiClientSecure client;
  Serial.println("GetInstanceClient");

  if(!verifyConnection()){
     return client;
  }

  if(!verifyToken()){
    return client;
  }
  
  Serial.printf("Connecting to %s... ", instance.c_str());
  
  if (!client.connect(instance.c_str(), hostPort)) {
    Serial.println("Failed.");
    client.stop();
    return client;
  } 

  Serial.printf("Success.");
  return client;
}

//------------------------------------------------------------------------------------
void SFManager::requestToken(){
  Serial.println("Request Token");
  tokenState = sf_token_request;
  
  WiFiClientSecure client = getAuthClient();
  if( !client.connected() ){
    return;
  }

  String requestBody = "grant_type=password&client_id=" + String(SFRA_CLIENT_ID) + 
    "&client_secret=" + String(SFRA_CLIENT_SECRET) + 
    "&username=" + String(SFRA_USER) + 
    "&password=" + String(SFRA_PASS);
  Serial.println(requestBody);

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
    tokenState = sf_token_valid;
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
  */
}
 
//------------------------------------------------------------------------------------
void SFManager::eventRequest(String eventName, String requestBody, UtilSFRACallback callback){
  scheduleRequest(sf_type_event, eventName, requestBody, callback);
}

//------------------------------------------------------------------------------------
void SFManager::flowRequest(String flowName, String requestBody, UtilSFRACallback callback){
  scheduleRequest(sf_type_flow, flowName, requestBody, callback);
}

//------------------------------------------------------------------------------------
void SFManager::scheduleRequest(UtilSFRARequestType type, String targetName, String requestBody, UtilSFRACallback callback){
  requestList[pushIndex] = {type, targetName, requestBody, callback};
  pushIndex = nextIndex(pushIndex);

  //pushIndex = ++pushIndex == UTILS_SF_CAPACITY ? 0 : pushIndex;
  Serial.println("Push: "+ String(pushIndex) +" - "+ String(popIndex));
  executeRequest();
}

//------------------------------------------------------------------------------------
void SFManager::executeRequest(){  
  UtilSFRARequest request = requestList[popIndex];
  bool success = false;

  if(request.type == sf_type_event){
    success = executeEventRequest(request.targetName, request.requestBody, request.callback);
  } 
  else if(request.type == sf_type_flow){
    success = executeFlowRequest(request.targetName, request.requestBody, request.callback);
  }

  if( success ){
    //popIndex = ++popIndex == UTILS_SF_CAPACITY ? 0 : popIndex;
    popIndex = nextIndex(popIndex);
    Serial.println("Pop: "+ String(pushIndex) +" - "+ String(popIndex));
  }
}

//------------------------------------------------------------------------------------
bool SFManager::executeEventRequest(String eventName, String requestBody, UtilSFRACallback callback){

  WiFiClientSecure client = getInstanceClient();
  if( !client.connected() ){
    return false;
  }

  client.println("POST /services/data/"+ String(SFRA_API) +"/sobjects/"+ eventName +" HTTP/1.1");
  client.println("Host: "+ instance);
  client.println("Content-Type: application/json");
  client.println("Content-Length: "+ String(requestBody.length(), DEC));
  client.println("Authorization: Bearer "+ token);
  client.println();
  client.println(requestBody);

  Serial.print("Platform Event result... ");
  unsigned long readTime = millis() + UTILS_SF_READ_TIME;

  while (client.connected() && readTime < millis()) {
    String line = client.readStringUntil('\n');
    Serial.print(line);
    if (line == "\r") {
      Serial.println("Done.");
      break;
    }
  }
  
  String line = client.readString();
  client.stop(); 

  //Serial.println(line);
  
  Serial.print("\nParsing result... ");
  if (line.indexOf("\"success\":true") != -1) {
    Serial.println("Success.");
    callback(true,line);
    return true;
  } 
  
  Serial.println("Failed.");
  callback(false,"");
  return false;
}

//------------------------------------------------------------------------------------
bool SFManager::executeFlowRequest(String flowName, String requestBody, UtilSFRACallback callback){

  WiFiClientSecure client = getInstanceClient();
  if( !client.connected() ){
    return false;
  }

  client.println("POST /services/data/"+ String(SFRA_API) +"/actions/custom/flow/"+ flowName +" HTTP/1.1");
  client.println("Host: "+ instance);
  client.println("Content-Type: application/json");
  client.println("Content-Length: "+ String(requestBody.length(), DEC));
  client.println("Authorization: Bearer "+ token);
  client.println();
  client.println(requestBody);

  Serial.print("Flow result... ");
  unsigned long readTime = millis() + UTILS_SF_READ_TIME;

  while (client.connected() && readTime < millis()) {
    String line = client.readStringUntil('\n');
    Serial.print(line);
    if (line == "\r") {
      Serial.println("Done.");
      break;
    }
  }
  
  String line = client.readString();
  client.stop();

  Serial.println("\nParsing result.");
  if (line.indexOf("isSuccess\":true") != -1) {
    Serial.println("Success.");
    callback(true,line);
    return true;
  } 
  
  Serial.println("Failed: " + line);
  callback(false,"");
  return false;
  
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

  // client.stop(); 
  // requestState = sf_state_idle;
  //flowCallback = nullptr;
}

//------------------------------------------------------------------------------------
void SFManager::loop(){  
  // if (refreshTime != 0 && refreshTime < millis()){
  //   refreshTime = 0;
  //   refreshToken();
  //   return;
  // }

  // Attempt delayed requests at a resonable interval. 
  if (retryTime != 0 && retryTime < millis()){
    
    // If there are outstanding requests, attempt to execute one.
    if( pendingRequests() ){
      executeRequest();
    }

    // Set the delay for the next execution cycle.
    setNeedsRetry();
  }

  // if (retryTime != 0 && retryTime < millis()){
  //   retryTime = 0;
  //   switch(requestState) {
  //     case sf_state_flow:
  //       Serial.println("SFManager : Flow Request");
  //       flowRequest();
  //       break;
  //     case sf_state_platform:
  //       Serial.println("SFManager : Platform Request");
  //       platformRequest();
  //       break;
  //     default:
  //     break;
  //   }
  // }
}
#endif