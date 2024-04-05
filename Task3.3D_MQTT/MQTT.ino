//#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <NewPing.h>

#include "secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const int triggerPin = 4;
const int echoPin = 3;
const int ledPin = 2;
const int MAX_DISTANCE = 20;

NewPing sonar(triggerPin, echoPin, MAX_DISTANCE);

bool motionDetected = false;
const unsigned long pingInterval = 50;
const int patThreshold = 5;
unsigned long lastPingTime = 0;
unsigned long currTime = 0;
int distance = 0;
int minDist;
int maxDist;

char broker[] = "broker.emqx.io";
const int port = 1883;

int wifiStatus = WL_IDLE_STATUS;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i=0;i<length;i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(String(topic) == "SIT210/wave") 
  { 
    flashLed(3, 250); 
  }
  if(String(topic) == "SIT210/pat") 
  { 
    flashLed(6, 100); 
  }
}



void reconnect()
{
  // Taken from other code
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // Don't need username or password, nor do we need a Last Will and Testament so we set these to null.
    // We do want to specify that we want a clean session as we don't want to respond to messages that are sent
    // while the device is not active.
    if (client.connect("arduinoClient-jjt-SIT210", NULL, NULL, NULL, NULL, NULL, NULL, true)) {
      Serial.println("connected");

      bool subSuccess = client.subscribe("SIT210/wave");
      if(!subSuccess) { Serial.println("Error, could not subscribe to SIT210/wave"); }
      
      subSuccess = client.subscribe("SIT210/pat");
      if(!subSuccess) { Serial.println("Error, could not subscribe to SIT210/pat"); }

      delay(1000);

    } else {
      Serial.print("Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }  
}

void indicateGesture()
{
  int range = maxDist - minDist;
  bool pubSuccess;

  if(range > patThreshold)
  {
    Serial.println("Pat Detected");
    pubSuccess = client.publish("SIT210/pat", "Jonathan Tynan");
    if(!pubSuccess) { Serial.println("Error, could not publish to SIT210/pat"); }
  }
  else
  {
    
    Serial.println("Wave Detected");
    pubSuccess = client.publish("SIT210/wave", "Jonathan Tynan");
    if(!pubSuccess) { Serial.println("Error, could not publish to SIT210/wave"); }
  }

}

void flashLed(int times, int del)
{
  for(int i = 0; i < times; i++)
  {
    digitalWrite(ledPin, HIGH);
    delay(del);
    digitalWrite(ledPin, LOW);
    delay(del);
  }
}

void setup()
{
  Serial.begin(115200);

  while(!Serial) {}
  

  while(wifiStatus != WL_CONNECTED)
  {
   Serial.print("Attempting to connect to WPA SSID: ");
   Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }
  Serial.println("You're connected to the network");

  pinMode(ledPin, OUTPUT);

  client.setServer(broker, port);
  client.setCallback(callback);
  
  Serial.println("Setup Complete");

}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  currTime = millis();
  if(currTime - lastPingTime > pingInterval)
  {
    distance = sonar.ping_cm();
    lastPingTime = millis();

    if(!motionDetected && distance > 0)
    {
      motionDetected = true;
      minDist = MAX_DISTANCE + 1;
      maxDist = 0;
    }
    else if(motionDetected && distance == 0)
    {
      motionDetected = false;
      indicateGesture();
    }

    if(motionDetected)
    {
      if(distance != 0 && distance < minDist) { minDist = distance; }
      if(distance != 0 && distance > maxDist) { maxDist = distance; }
    }
  }
}