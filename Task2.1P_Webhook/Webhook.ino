#include <WiFiNINA.h>

#include "DHT.h"
#include "secrets.h"
#include "ThingSpeak.h"

#define DHTPIN 2

#define DHTTYPE DHT11



DHT dht(DHTPIN, DHTTYPE);

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

unsigned long chNum = SECRET_CH_ID;
const char *APIKey = SECRET_WRITE_APIKEY;


int status = WL_IDLE_STATUS;
WiFiClient client;

unsigned long lastSensorRead = 0;
unsigned long lastDataWrite = 0;
const long readInterval = 2000;
const long writeInterval = 60000;

float hum, tempC, tempF;
void setup()
{
  Serial.begin(9600);

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }

  Serial.println("You're connected to the network");

  ThingSpeak.begin(client);
  dht.begin();
}

void loop()
{
  unsigned long currTime = millis();

  // We want to read the sensor on a 2 second interval,
  // so we compare the current time to when we last read it
  // and compare with our desired interval.
  if(currTime - lastSensorRead >= readInterval)
  {
    lastSensorRead = currTime;

    hum = dht.readHumidity();
    tempC = dht.readTemperature();
    tempF = dht.readTemperature(true);
  }

  // As above, we want to write data every 60 seconds
  // so we use check if the interval time has been reached
  // and we do it in this way to prevent any blocking from
  // happening when we use delay().
  if(currTime - lastDataWrite >= writeInterval)
  {
    lastDataWrite = currTime;

    ThingSpeak.setField(1, tempC);
    ThingSpeak.setField(2, tempF);
    ThingSpeak.setField(3, hum);

    int returnStatus = ThingSpeak.writeFields(chNum, APIKey);
    // Check return status to confirm channel has been updated correctly,
    // otherwise we print the error code to the Serial Monitor
    if(returnStatus == 200)
    {
      Serial.println("Channel Update Successful.");
    } 
    else
    {
      Serial.println("Problem updating channel. HTTP error code " + String(returnStatus));
    }
  }
}