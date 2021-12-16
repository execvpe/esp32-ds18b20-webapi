#include "WiFiHandler.hpp"

#include <HardwareSerial.h>
#include <WiFi.h>

#include "decrypt.hpp"
#include "encData.hpp"

// #define STATIC_IP

/////////////////////////////////////////////////////////////////////

namespace {	 // "static"
#ifdef STATIC_IP
	IPAddress localIP(172, 28, 116, 0);
	IPAddress gateway(172, 28, 0, 1);
	IPAddress subnet(255, 255, 0, 0);
	IPAddress primaryDNS(172, 28, 0, 1);
#endif
};	// namespace

// public constructors

WiFiHandler::WiFiHandler() {
#ifdef STATIC_IP
	if (!WiFi.config(localIP, gateway, subnet, primaryDNS, primaryDNS)) {
		Serial.println("STA Failed to configure");
	}
#endif
}

WiFiHandler::WiFiHandler(const char *hostname) {
	WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
	WiFi.setHostname(hostname);
}

// public functions

void WiFiHandler::begin() {
	char ssid[33];
	char password[65];
	decryptShuffled(ENCRYPTED_FIELD, 0, ssid);
	decryptShuffled(ENCRYPTED_FIELD, 1, password);

	Serial.printf("Connecting to SSID \"%s\"\n", ssid);

	WiFi.begin(ssid, password);
	memset(password, 0x00, 65);

	while (WiFi.status() != WL_CONNECTED) {
		delay(1500);
		Serial.printf("Connecting to SSID \"%s\"\n", ssid);
	}

	Serial.print("Successful. Current IP address: ");
	Serial.println(WiFi.localIP());
	Serial.print("Hostname: ");
	Serial.println(WiFi.getHostname());
}
