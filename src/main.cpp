#include <FreeRTOS.h>

#include "SimpleServer.hpp"
#include "StringMacros.hpp"
#include "TSensor.hpp"
#include "WiFiHandler.hpp"

#define CUSTOM_HOSTNAME "VentControl-ESP32-116-0"

#define KiB(X) (1024 * X)

namespace {	 // "static"
	WiFiHandler wifiHandler(CUSTOM_HOSTNAME);
	SimpleServer server;
	TSensor tsensor;

	TaskHandle_t cpu0_handle;
}  // namespace

static void setup0(void *) {
	static constexpr TickType_t delay = 10000;

	Serial.print("setup0() core: ");
	Serial.println(xPortGetCoreID());

	while (1) {
		tsensor.updateAll();
		vTaskDelay(delay / portTICK_RATE_MS);
	}
}

void setup() {
	Serial.begin(115200);
	wifiHandler.begin();
	server.begin();
	Serial.print("setup() core: ");
	Serial.println(xPortGetCoreID());

	xTaskCreatePinnedToCore(&setup0, "setup0", KiB(32), NULL, 0, &cpu0_handle, 0);
}

static int32_t checkHttp(const char *path) {
	static constexpr uint8_t u8max = std::numeric_limits<uint8_t>::max();

	if (path[0] == '/')
		path++;

	/////////////////////////////////////////////

	if (STRING_STARTS_WITH(path, "TEMP/SENSOR_VALUE/")) {
		STRING_PRUNE_SUBSTRING(path, "TEMP/SENSOR_VALUE/");

		char *endPtr;
		long sensorIdx = strtol(path, &endPtr, 10);

		if (path == endPtr || sensorIdx < 0 || sensorIdx > u8max)
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

	if (STRING_EQUALS(path, "index.html"))
		return 99901;

	return 0;
}

static void sendHttp(WiFiClient &client, const char *path, int code) {
	static constexpr uint8_t u8max = std::numeric_limits<uint8_t>::max();

	if (code >= 1000 && code <= 1000 + u8max) {
		try {
			client.printf("%.2f", tsensor.getCelsius(code - 1000));
		} catch (int e) {
			client.printf("INVALID READ: %i", e);
		}
		return;
	}
	if (code >= 2000 && code <= 2000 + u8max) {
		try {
			client.printf("%.2f", tsensor.getFahrenheit(code - 2000));
		} catch (int e) {
			client.printf("INVALID READ: %i", e);
		}
		return;
	}

	switch (code) {
		case 99901:
			client.printf(
				"<html><head>"
				"<title>%s</title>"
				"</head><body>",
				CUSTOM_HOSTNAME);

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
