#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DiyLed.h>

const String LIGHT_NAME = "Light";
const int NUM_PIXELS = 120;
bool state = true;
byte brightness = 100;
byte r = 255, g = 50, b = 255;
String mode = "Color";
const String MODES[] = {"Color", "Rainbow"};
const int MODESSIZE = 2;

const String SSID = "SSID";
const String PASSWORD = "**************";

DiyLed* client;

void valueCallback(prop, value) {
	// do your processing/handling in here
	if (prop == "power")
		Serial.println("Power property changed to: " + str(value));
	else if (prop == "brightness")
		Serial.println("Brightness property changed to: " + str(value));
	else if (prop == "color")
		Serial.println("Color property changed to: [" + str(value[0]) + ", " + str(value[1]) + ", " + str(value[2]) + "]");
	else if (prop == "mode")
		Serial.println("Mode property changed to: " + str(value));
}

void stateGetCallback() {
	client->currentData = client->assembleJson(LIGHT_NAME, NUM_PIXELS, state, brightness, mode, MODES, MODESSIZE, r, g, b);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client = new DiyLed(stateGetCallback, valueCallback);
  client->begin();
}

void loop() {
  client->loop();
  delay(1000);
}
