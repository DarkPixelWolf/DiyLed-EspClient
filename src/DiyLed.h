#ifndef DiyLed_h
#define DiyLed_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class DiyLed {

	typedef void(*CallbackDataFunction) ();
	typedef void(*CallbackValue) (String, String);

private:
	ESP8266WebServer* server = nullptr;
	WiFiUDP udp;
	IPAddress ipMulti = IPAddress(239, 255, 255, 250);
	int portMulti = 1900;
	bool udpConnected = false;
	char packetBuffer[255];
	String escapedMac = "";

	CallbackDataFunction dataCallback;
	CallbackValue valueCallback;

	void respondToSearch(bool multicast) {
		IPAddress localIP = WiFi.localIP();
		char s[16];
		sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

		String response =
			"HTTP/1.1 200 OK\r\n"
			"EXT:\r\n"
			"CACHE-CONTROL: max-age=100\r\n"
			"LOCATION: http://" + String(s) + ":80/properties\r\n"
			"SERVER: LiyLed/1.1, UPnP/1.0, DiyLedLight/1.1\r\n"
			"hue-bridgeid: " + escapedMac + "\r\n"
			"ST: urn:diyleddevice:light\r\n"
			"USN: uuid:" + escapedMac + "::urn:diyleddevice\r\n"
			"\r\n";

		Serial.println("Responding search");

		if (multicast) {
			Serial.println("SEND RESPONSE");
			udp.beginPacketMulticast(ipMulti, portMulti, WiFi.localIP());
			udp.write(response.c_str());
			udp.endPacket();
		}
		else {
			Serial.println("SEND RESPONSE");
			Serial.println(udp.remoteIP());
			Serial.println(udp.remotePort());
			udp.beginPacket(udp.remoteIP(), portMulti);
			udp.write(response.c_str());
			udp.endPacket();
			Serial.println("-> Server received?");
		}
	}

	void serveNotFound()
	{
		Serial.println("Not Found");
		handleApi(server->uri(), server->arg(0));
	}

	void serveProperties()
	{
		Serial.println("Serving properties");
		dataCallback();

		server->send(200, "application/json", currentData.c_str());
	}

	void startHttpServer() {
		if (server == nullptr) {
			Serial.println("HTTP Server object == nullptr");
			server = new ESP8266WebServer(80);
			Serial.println("HTTP Server object created");
			server->onNotFound([=]() {serveNotFound(); });
		}
		server->on("/properties", HTTP_GET, [=]() {serveProperties(); });
		server->begin();
		Serial.println("HTTP Server started");
	}

public:
	DiyLed() {}
	DiyLed(CallbackDataFunction _dataCallback, CallbackValue _valueCallback) {
		dataCallback = _dataCallback;
		valueCallback = _valueCallback;
	}

	String currentData = "";

	String assembleJson(String _name, int _leds, bool _state, byte _brightness, String _mode, const String _modes[], const int _modessize, byte _r, byte _g, byte _b) {
		IPAddress localIP = WiFi.localIP();
		char s[16];
		sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

		StaticJsonBuffer<500> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root["id"] = "createRequestPacket";
		JsonObject& data = root.createNestedObject("data");
		JsonArray& color = data.createNestedArray("color");
		data["request"] = "light";
		data["id"] = escapedMac;
		data["name"] = _name;
		data["ip"] = String(s);
		data["ledCount"] = _leds;
		data["power"] = String(_state);
		data["brightness"] = _brightness;
		Serial.println("Before MODES");
		Serial.println(_modessize);
		JsonArray& modes = data.createNestedArray("modes");
		for (int i = 0; i < _modessize; i++) {
		  Serial.println("In MODES");
		  Serial.println(_modes[i]);
		  modes.add(String(_modes[i]));
		}
		Serial.printf("After MODES");
		data["mode"] = _mode;
		color.add(_r);
		color.add(_g);
		color.add(_b);
			
		String json;
		root.printTo(json);

		return json;
	}

	bool begin() {
		escapedMac = WiFi.macAddress();
		escapedMac.replace(":", "");
		escapedMac.toLowerCase();
		Serial.println("Escaped mac set");

		udpConnected = udp.beginMulticast(WiFi.localIP(), ipMulti, portMulti);
		Serial.println("UDP started: " + String(udpConnected));

		if (udpConnected) {
			Serial.println("Starting HTTP");
			startHttpServer();
			respondToSearch(true);
			Serial.println("Startup complete");
			return true;
		}
		Serial.println("Startup failed");
		return false;
	}

	void loop() {
		if (server == nullptr) return;
		server->handleClient();

		if (!udpConnected) return;
		int packetSize = udp.parsePacket();
		if (!packetSize) return;

		int len = udp.read(packetBuffer, 254);
		if (len > 0) {
			packetBuffer[len] = 0;
		}
		udp.flush();

		Serial.println("UDP!");

		String request = packetBuffer;
		//Serial.println(request);
		if (request.indexOf("M-SEARCH") >= 0) {
			if (request.indexOf("urn:diyleddevice:light") > 0) {
				respondToSearch(false);
			}
		}
	}

	void handleApi(String req, String body) {
		Serial.println("Handling api call");
		Serial.println(req);
		Serial.println(body);
		String response = "";
		
		StaticJsonBuffer<500> jsonBuffer;
		JsonObject& data = jsonBuffer.parseObject(body);
		
		if (data.success()) {
			if (req.indexOf("/updateValue") > 0) {
				String s = String(data["data"]["value"].asString());
				if (String(data["data"]["key"].asString()) == "color") {
					data["data"]["value"].printTo(s);
				}
				valueCallback(data["data"]["key"].asString(), s);
				response = "{\"id\": \"successPacket\", \"data\" : {\"message\": \"Wert geaendert.\", \"id\" : " + String(data["data"]["id"].asString()) + "}}";
			}
			else if (req.indexOf("/applyScene") > 0) {
				valueCallback("brightness", data["data"]["brightness"].asString());
				valueCallback("mode", data["data"]["mode"].asString());
				String s;
				data["data"]["color"].printTo(s);
				valueCallback("color", s);
				valueCallback("power", data["data"]["power"].asString());
				response = "{\"id\": \"successPacket\", \"data\" : {\"message\": \"Werte geaendert.\", \"id\" : " + String(data["data"]["id"].asString()) + "}}";
			}
			else {
				response = "{\"id\": \"errorPacket,\", \"data\" : {\"message\": \"Ungueltige Anfrage.\", \"id\" : " + String(data["data"]["id"].asString()) + "}}";
			}
		}
		else {
			response = "{\"id\": \"errorPacket,\", \"data\" : {\"message\": \"Ungueltige Daten.\", \"id\" : \"UNKNOWN\"}}";
		}
		
		Serial.println("Responding: " + response);
		server->send(200, "application/json", response);
	}
};
#endif