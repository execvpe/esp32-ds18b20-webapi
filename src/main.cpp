#include <WiFi.h>

#include "SimpleServer.hpp"
#include "StringMacros.hpp"
#include "TSensor.hpp"
#include "decrypt.hpp"
#include "encData.hpp"

#define CUSTOM_HOSTNAME "VentControl-ESP32-116-0"

namespace {
	SimpleServer server;
	TSensor tsensor;

	TaskHandle_t cpu0_handle;
}  // namespace

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
#ifdef CUSTOM_HOSTNAME
		WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
		WiFi.setHostname(CUSTOM_HOSTNAME);
#endif
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

		memset(password, 0x00, 65);

		Serial.print("Successful. Current IP address: ");
		Serial.println(WiFi.localIP());
		Serial.print("Hostname: ");
		Serial.println(WiFi.getHostname());
	}
};	// namespace WifiHandler

static void loop0(void *) {
	Serial.print("loop0() core: ");
	Serial.println(xPortGetCoreID());

	while (1) {
		tsensor.updateAll();
		delay(5000);
	}
}

void setup() {
	Serial.begin(115200);
	WifiHandler::begin();
	server.begin();
	Serial.print("setup() core: ");
	Serial.println(xPortGetCoreID());

	xTaskCreatePinnedToCore(&loop0, "loop0", 10000, NULL, 0, &cpu0_handle, 0);
}

static int32_t checkHttp(const char *path) {
	if (path[0] == '/')
		path++;

	/////////////////////////////////////////////

	if (S_MATCH_FIRST(path, "TEMP/SENSOR_VALUE/", 18)) {
		const uint8_t max = std::numeric_limits<uint8_t>::max();
		const char *idxPos = path + 18;

		char *endPtr;
		long sensorIdx = strtol(idxPos, &endPtr, 10);

		if (idxPos == endPtr || sensorIdx < 0 || sensorIdx > max)
			return 0;

		if (strlen(endPtr) < 2)	 // at least "/C" or "/F" should be left
			return 0;

		if (endPtr[0] != '/')
			return 0;

		switch (endPtr[1]) {
			case 'C':
				return 1000 + (int32_t) sensorIdx;
			case 'F':
				return 2000 + (int32_t) sensorIdx;
			default:
				return 0;
		}
	}

	if (S_EQUALS(path, "index.html"))
		return 99901;

	return 0;
}

static void sendHttp(WiFiClient &client, const char *path, int code) {
	if (code >= 1000 && code <= 1255) {
		try {
			client.printf("%.2f", tsensor.getCelsius(code - 1000));
		} catch (int e) {
			client.printf("INVALID READ: %i", e);
		}
		return;
	}
	if (code >= 2000 && code <= 2255) {
		try {
			client.printf("%.2f", tsensor.getFahrenheit(code - 2000));
		} catch (int e) {
			client.print("INVALID READ: -127");
		}
		return;
	}

	switch (code) {
		case 99901:
			client.printf(
				"<html><head>"
				"<title>%s</title>"
				"</head><body>",
				WiFi.getHostname());

			for (size_t i = 0; true; i++) {
				try {
					client.printf(
						"<b>Current Temperature (Sensor %i):</b>   %.2f C, %.2f F<br>",
						i, tsensor.getCelsius(i), tsensor.getFahrenheit(i));
				} catch (int e) {
					if (i == 0)
						client.print("<b>NO SENSORS AVAILABLE</b>");
					break;
				}
			}
			client.print("</body></html>");
			break;
		default:
			return;
	}
}

void loop() {
	WiFiClient client = server.accept();

	if (server.isAvailable(client))
		server.handleConnection(client, &checkHttp, &sendHttp);
	client.stop();
}
