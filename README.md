# DiyLed-EspClient
This is the clientside of the DiyLed system for ESP8266 devices.

* [Server]()
* [Client for Python]()
* [DiyLed App]()

#### What is the DiyLed system?
The DiyLed system is my version of systems/enviroments like Philips Hue (sort of). This system was created to

* allow the use of any device that is not supported by Amazon Alexa and Philips Hue
* give the user a overall higher customizability regarding control, light effects etc
* be a kind of challenge/project for myself to be honest

#### Functions
* fully customizable server and client (for python3 and esp)
* full Alexa integration
* compatible DiyLed App
* no static ip setup needed (is recommended though)

#### Installation
*This library currently only works with ESP8266 devices, other devices may be added in the future.*

Arduino IDE:
The installation is very simple, just download the repository as a zip file and install it through the library manager of the Arduino IDE, then you can include it into your script:
```cpp
#include <DiyLed.h>
```

#### Usage
After importing the library you have to create some functions for the client to work:

ValueCallback, this function is called when the server wants to change a property, the types of `value` are specified below the example:
```cpp
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
```
`prop` Name | `value` Type | Values | Description
--- | --- | --- | ---
power | string | toggle, true, false | The state of the light, on/off
brightness | int | 0 - 255 | The brightness of the light
color | int array[3] | 3 values 0 - 255 | RGB color values as array
mode | string | defined by MODES array | Mode of the light, MODES array is provided when creating a DiyLedClient object



StateGetCallback, this function is used by the DiyLedClient to get the current state of the device, as this is completely up to you, the client uses a callback function to get the state from your script, argument types and meaning are specified below the example:
```cpp
void stateGetCallback() {
	dlc->currentData = dlc->assembleJson(light_name, num_leds, state, brightness, mode, modes, modessize, r, g, b);
}
```
Argument | Type | Values | Description
--- | --- | --- | ---
light_name | string | * | The name of your light
num_leds | int | * | The number of leds your light has
state | boolean | True, False | The state of your light, e.g. on=True/off=False
brightness | int | 0 - 255 | The brightness of your light
mode | string | * | The mode the light is currently in
modes | string[] | * | An array containing all available modes
modessize | int | * | This has to be the exact size of the modes array
r | int | 0 - 255 | R color value
g | int | 0 - 255 | G color value
b | int | 0 - 255 | B color value


After creating these functions you can create a DiyLedClient object and initialize it like so (in the `setup()` function):
```cpp
DiyLed* client;

void setup() {
  // Connect to WiFi first!
  // ... 
  client = new DiyLed(stateGetCallback, valueCallback);
  client->begin();
  // ...
}
```
If you want to see a log of events you can add `DEBUG=True` as an additional argument.

Great! Now there is only one thing left. Make sure the clients `loop()` function is called in the Arduino `loop()` function:
```cpp
void loop() {
  // ...
  client->loop();
  // ...
  delay(1000); // you don't have to use a delay here, it just reduces load
}
```

That is it! The repository contains a `ClientExample.ino` file for reference.
