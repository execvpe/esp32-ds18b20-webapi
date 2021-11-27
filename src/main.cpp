#include <WiFi.h>

#include "SimpleServer.hpp"
#include "StringMacros.hpp"
#include "TSensor.hpp"
#include "decrypt.hpp"
#include "encData.hpp"

#define STATIC_IP  // enable if using static ip instead of DHCP

SimpleServer server;
TSensor tsensor;

namespace WifiHandler {
	namespace {	 // "private"
#ifdef STATIC_IP
		IPAddress localIP(172, 28, 116, 0);
		IPAddress gateway(172, 28, 0, 1);
		IPAddress subnet(255, 255, 0, 0);
		IPAddress primaryDNS(172, 28, 0, 1);
#endif
	};	// namespace

	void begin() {
#ifdef STATIC_IP
		if (!WiFi.config(localIP, gateway, subnet, primaryDNS, primaryDNS)) {
			Serial.println("STA Failed to configure");
		}
#endif

		char ssid[33];
		char password[65];
		decryptShuffled(ENCRYPTED_FIELD, 0, ssid);
		decryptShuffled(ENCRYPTED_FIELD, 1, password);

		Serial.printf("Connecting to SSID \"%s\"\n", ssid);

		WiFi.begin(ssid, password);
		while (WiFi.status() != WL_CONNECTED) {
			delay(1000);
			Serial.printf("Connecting to SSID \"%s\"\n", ssid);
		}
		Serial.print("Successful. Current IP address: ");
		Serial.println(WiFi.localIP());
	}
};	// namespace WifiHandler

void setup() {
	Serial.begin(115200);
	WifiHandler::begin();
	server.begin();
}

static int check(const char *path) {
	if (path[0] == '/')
		path++;

	/////////////////////////////////////////////

	if (S_EQUALS(path, "TEMP_C"))
		return 1001;

	if (S_EQUALS(path, "STATUS_PAGE"))
		return 90001;

	return 0;
}

static void send(WiFiClient &client, const char *path, int code) {
	switch (code) {
		case 1001:
			client.printf("%.2f", tsensor.getCelsius());
			break;
		case 90001:
			client.printf(
				"<html><head>"
				"<title>Ventilation Temperature Control</title>"
				"</head><body>"
				"<b>Current Temperature:</b> %.2f C"
				"</body></html>",
				tsensor.getCelsius());
		default:
			return;
	}
}

void loop() {
	WiFiClient client = server.accept();

	if (server.isAvailable(client))
		server.handleConnection(client, &check, &send);
	client.stop();
}
