/*
 * main.cpp
 *
 *  Created on: Apr 15, 2022
 *  Created By: donskytech
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"

#define DHTPIN 26 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

static int humidVal = 0;
static int tempVal = 0;
static int tempFVal = 0;

// CHANGE THIS TO ADD YOUR WIFI USERNAME/PASSWORD
const char * WIFI_SSID = "BabyBoy";
const char * WIFI_PASS = "0833126198.miness";
// const char * WIFI_SSID = "digitalskills";
// const char * WIFI_PASS = "1234567890";

//Initialize the JSON data we send to our websocket server
const int capacity = JSON_OBJECT_SIZE(3);
StaticJsonDocument<capacity> doc;
  

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

#define USE_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    USE_SERIAL.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
    USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
    break;
  case WStype_TEXT:
    USE_SERIAL.printf("[WSc] get text: %s\n", payload);
    break;
  case WStype_BIN:
    USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
    break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
  case WStype_PING:
  case WStype_PONG:
    break;
  }
}

void setup()
{
  //Set the baud rate
  USE_SERIAL.begin(115200);
  USE_SERIAL.printf("Begin websocket client program....\n");

  for (uint8_t t = 4; t > 0; t--)
  {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

  // WiFi.disconnect();
  USE_SERIAL.printf("Connecting");
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    USE_SERIAL.printf(".");
    delay(100);
  }
  USE_SERIAL.printf("\nConnected!");

  // server address, port and URL
  webSocket.begin("192.168.43.217", 8080, "/sendSensorData");
  // webSocket.begin("10.6.4.78", 8080, "/sendSensorData");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  // webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  // // Set the resolution values
  // analogReadResolution(RESOLUTION);
  dht.begin();
}

void readLDRValue()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperatureInC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float temperatureInF = dht.readTemperature(true);

  // Read and print the sensor pin value
  //int tempSensorVal = analogRead(ANALOG_READ_PIN);

  USE_SERIAL.println(humidity);
  USE_SERIAL.println(temperatureInC);
  USE_SERIAL.println(temperatureInF);

  // Check if value read is different then send a websocket message to the server
  if (humidity != humidVal)
  {
    humidVal = humidity;

    //send JSON message in this format {"value": 100}
    doc["Nitrogen"] = humidity;
    doc["Phrosphrous"] = temperatureInC;
    doc["Potassium"] = temperatureInF;

    // Declare a buffer to hold the result
    char output[100];

    serializeJson(doc, output);

    // send message to server when Connected
    webSocket.sendTXT(output);
    // webSocket.sendTXT(output2);
    // webSocket.sendTXT(output3);
  }

  // sleep for some time before next read
  delay(100);
}

// This will go into loop
void loop()
{
  readLDRValue();
  webSocket.loop();
}