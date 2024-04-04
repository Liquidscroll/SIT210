#include <BH1750.h>
#include <Wire.h>
#include <WiFiNINA.h>

#include "secrets.h"

BH1750 lightMeter;

unsigned long sunlightEntryTime = 0;
unsigned long sunlightExitTime = 0;
bool readyToTrigger = false;

float lux = 0;
bool inSunlight = false;

// Interval will determine the time that we measure the device is in sunlight
// This is done to account for light level flucations that may give a flase-positive.

//const long interval = 300000; //5 minutes
const long interval = 10000; //10 seconds



char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char hookUrl[] = "hook.eu2.make.com";
const String hookKey = "dphbdxklhplbcbxbmy3v3g1j1ed54k7r";

int status = WL_IDLE_STATUS;
WiFiClient client;

void triggerWebhook()
{
  Serial.println("\nStarting connection to server...");
  if (client.connectSSL(hookUrl, 443)) { // Use port 443 for HTTPS
    Serial.println("Connected to server");
    // Construct the JSON data string, which will look like:
    // { "inSunlight" : {inSunlight Value} }
    String jsonData = "{\"inSunlight\":\"" + String(inSunlight ? "true" : "false") + "\"}";

    // Send the HTTP POST request
    client.println("POST /" + hookKey + " HTTP/1.1");
    client.println("Host: " + String(hookUrl));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println("Connection: close");
    client.println(); // End of headers
    client.println(jsonData); // Send the JSON payload

    Serial.println("Request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("Headers received");
        break;
      }
    }
    // Read the body of the response, this will indicate if our payload has been accepted.
    String line = client.readStringUntil('\n');
    Serial.println("Response:");
    Serial.println(line);
    Serial.println();
  } else {
    // If we couldn't make a connection
    Serial.println("Connection failed");
  }

}

void setup() {
  Serial.begin(9600);
  while(!Serial) {}; // Wait until Serial is connected.
  while(status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }

  Serial.println("You're connected to the network");
  
  Wire.begin(); // Initialise I2C
  lightMeter.begin();

  Serial.println("Setup Complete.");
}

void loop() {
  lux = lightMeter.readLightLevel();
  
  //Serial.print("Light Level: ");
  //Serial.println(lux);

  // Here we check if the light level has crossed our sunlight threshold.
  // We also store the start time of our entry or exit from sunlight.
  if(!inSunlight && lux >= 1000)
  {
    inSunlight = true;
    sunlightEntryTime = millis();
  }
  else if(inSunlight && lux < 1000)
  {
    inSunlight = false;
    sunlightExitTime = millis();
  }
  
  // First we check if we are in sunlight, or we have been in sunlight in the past 'interval' milliseconds.
  // We also check if, accounting for the interval, we have been in sunlight for the required time to trigger 
  // the state change. This ensures a bugger period before deciding its not in sunlight, allowing for temporary 
  // shadows without immediately toggling our state. This should account for things like cloud cover.
  if((inSunlight || (!inSunlight && millis() - sunlightExitTime < interval))
      && (millis() - sunlightEntryTime >= interval))
  {
    readyToTrigger = true;
  } 
  // Here we check if we're not currently in sunlight AND we've exited the sun over 'interval' milliseconds ago.
  // This resets the trigger condition if enough time has passed without sunlight, as this indicates a more 
  // permanent change in lighting. (e.g. the sun has set)
  else if(!inSunlight && millis() - sunlightExitTime >= interval)
  {
    readyToTrigger = false;
  }
  
  // If conditiions are met, then we are ready to trigger the webhook.
  if(readyToTrigger)
  {
    Serial.println("\nTriggering webhook...\n");
    triggerWebhook();
    readyToTrigger = false;
    sunlightEntryTime = millis();
  }
  
  delay(1000);
}
