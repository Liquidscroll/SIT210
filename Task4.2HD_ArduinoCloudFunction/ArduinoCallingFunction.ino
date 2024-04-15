#include "Firebase_Arduino_WiFiNINA.h"

#include <WiFiNINA.h>

#include "secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char dataPath[] = "/colour.json";
char fireBaseUrl[] = DATA_URL;

int redLedPin = 2;
int blueLedPin = 3;
int greenLedPin = 4;

int wifiStatus = WL_IDLE_STATUS;
WiFiClient client;

String lastColour = "";
int maxReconnect = 10;
int numReconnect = 0;

void setup() {
  Serial.begin(9600);
  while(!Serial) { };
  while(wifiStatus != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }
  Serial.println("You're connected to the network");

  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  reconnect();

  Serial.println("Setup Complete");
}

void loop() {
  String line = "";
  bool readingData = false;

  while(client.available())
  {

    char c = client.read();
    Serial.write(c);
    if(c == '\n')
    { 
      if(line.startsWith("data:"))
      {
        readingData = true;
        line = line.substring(6);
      }
      else
      {
        line = "";
      }
      
      if(readingData)
      {
        int endOfData = line.lastIndexOf('\"');
        int startOfData = line.lastIndexOf('\"', endOfData - 1);
        
        String colour = line.substring(startOfData + 1, endOfData);
        
        controlLEDs(colour);

        readingData = false;
        line = "";
      }
    }
    else
    {
      line += c;
    }
  }

  while(!client.connected() && numReconnect < maxReconnect)
  {
    numReconnect++;
    reconnect();
  }
  if(numReconnect >= maxReconnect) { while(1) { ; } }
}


void controlLEDs(String colour) {
  // Turn all LEDs off initially.
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(blueLedPin, LOW);

  if(colour == "none" || colour == "null") { return; }

  // Turn the specified LED on.
  if (colour == "red") {
    digitalWrite(redLedPin, HIGH);
  } else if (colour == "green") {
    digitalWrite(greenLedPin, HIGH);
  } else if (colour == "blue") {
    digitalWrite(blueLedPin, HIGH);
  }
}

void reconnect()
{
  Serial.println("Attempting to connect to database...");
  if(!client.connectSSL(fireBaseUrl, 443))
  {
    Serial.println("Connection Failed.");
    return;
  }
  Serial.println("Connection Successful.");

  client.println("GET " + String(dataPath) + " HTTP/1.1");
  client.println("Host: " + String(fireBaseUrl));
  client.println("Accept: text/event-stream");
  client.println("Cache-Control: no-cache");
  client.println("Connection: keep-alive");
  client.println();
}
