/*********************************************************************
 This is an example using Device Parameters and the ESPUtils Library

 By default the preferences object is named "ESPUTILS"
 To change that look at the UTIL_PREF_KEY in UtilCommon.h

*********************************************************************/

#include <Arduino.h>
#include <ESPUtils.h>

#define PARAM_ONE   "paramOne"
#define PARAM_TWO   "paramTwo"
#define PARAM_THREE "paramThree"

void showProperties(){
  Serial.println("\nDevice Parameters");
  Serial.println("Param One (int value) - " + String(ESPUtils::getParameterI(PARAM_ONE)));
  Serial.println("Param Two (string value) - " + String(ESPUtils::getParameterS(PARAM_TWO)));

  // Note - Default Values (for string) are not working currently 
  // Serial.println("Param Three (string value) - " + String(ESPUtils::getParameterS(PARAM_THREE, "Empty")));
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESPUtils Parameter Demo");
  
  // Note - Uncomment this line to clear all cached properties.
  //ESPUtils::clearPreferences();

  // Show the properties on boot
  showProperties();
  
  // Initialize properties or update their values

  // Check to see if there is data in PARAM_ONE, if not set its initial value
  if(ESPUtils::getParameterI(PARAM_ONE) == INT_MIN)
  {
    ESPUtils::setParameter(PARAM_ONE, 1);
  }
  else {
    int p1 = ESPUtils::getParameterI(PARAM_ONE);
    p1 += 1;
    ESPUtils::setParameter(PARAM_ONE, p1); 
  }

  // Check to see if there is data in PARAM_TWO, if not set its initial value
  if(ESPUtils::getParameterS(PARAM_TWO) == "")
  {
    ESPUtils::setParameter(PARAM_TWO, "ESPUtils");
  }  

  // Show the updated properties
  showProperties();

  // Restart the microcontroller
  Serial.println("Rebooting to demonstrate cached properties.");
  
  delay(1000);
  Serial.print("3... ");
  
  delay(1000);
  Serial.print("2... ");
  
  delay(1000);
  Serial.println("1");
  
  delay(1000);
  ESPUtils::restart();
}

void loop() {
}