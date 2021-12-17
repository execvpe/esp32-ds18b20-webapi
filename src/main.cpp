#include <FreeRTOS.h>

#include "SimpleServer.hpp"
#include "StringMacros.hpp"
#include "TSensor.hpp"
#include "WiFiHandler.hpp"

#define CUSTOM_HOSTNAME "VentControl-ESP32-116-0"
#define UPDATE_VALUES_EVERY_SEC 10

#define KiB(X) (1024 * X)
#define GET_BYTE(V, N) ((V >> (8 * N)) & 0xFF)
#define SET_BYTE(V, T, N, B)       \
	V &= ~((T) 0xFF << (8 * (N))); \
	V |= ((T) (B) << (8 * (N)));

namespace {	 // "static"
	WiFiHandler wifiHandler(CUSTOM_HOSTNAME);
	SimpleServer server;
	TSensor tsensor;

	TaskHandle_t cpu0_handle;
}  // namespace

static void setup0(void *) {
	Serial.print("setup0() core: ");
	Serial.println(xPortGetCoreID());

	while (1) {
		tsensor.updateAll();
		vTaskDelay(((TickType_t) UPDATE_VALUES_EVERY_SEC * 1000) / portTICK_RATE_MS);
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
	static_assert(CHAR_BIT <= 8, "The return value works with 8-bit characters only!");
	static constexpr uint8_t u8max = std::numeric_limits<uint8_t>::max();

	if (path[0] == '/')
		path++;

	/////////////////////////////////////////////////////////////////////////////

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

		int32_t ret = 0;

		switch (endPtr[1]) {
			case 'C':							 // Celsius
				SET_BYTE(ret, int32_t, 3, 'S');	 // Sensor
				SET_BYTE(ret, int32_t, 1, 'C');	 // Celsius
				SET_BYTE(ret, int32_t, 0, sensorIdx);
				return ret;
			case 'F':							 // Fahrenheit
				SET_BYTE(ret, int32_t, 3, 'S');	 // Sensor
				SET_BYTE(ret, int32_t, 1, 'F');	 // Fahrenheit
				SET_BYTE(ret, int32_t, 0, sensorIdx);
				return ret;
		}
		return 0;
	}

	if (STRING_STARTS_WITH(path, "TEMP/MILLIS_ELAPSED/")) {
		STRING_PRUNE_SUBSTRING(path, "TEMP/MILLIS_ELAPSED/");

		char *endPtr;
		long sensorIdx = strtol(path, &endPtr, 10);

		if (path == endPtr || sensorIdx < 0 || sensorIdx > u8max)
			return 0;

		int32_t ret = 0;
		SET_BYTE(ret, int32_t, 3, 'M');	 // Millis
		SET_BYTE(ret, int32_t, 0, sensorIdx);

		return ret;
	}

	if (STRING_EQUALS(path, "index.html")) {
		int32_t ret = 0;
		SET_BYTE(ret, int32_t, 3, 'I');	 // Index
		return ret;
	}

	return 0;
}

static void sendHttp(WiFiClient &client, const char *path, int32_t code) {
	try {
		switch (GET_BYTE(code, 3)) {
			case 'M':  // Millis
				client.printf("%lu", tsensor.elapsedSince(GET_BYTE(code, 0)));
				return;

			case 'S':  // Sensor
				switch (GET_BYTE(code, 1)) {
					case 'C':  // Celsius
						client.printf("%.2f", tsensor.getCelsius(GET_BYTE(code, 0)));
						break;
					case 'F':  // Fahrenheit
						client.printf("%.2f", tsensor.getFahrenheit(GET_BYTE(code, 0)));
						break;
				}
				return;
			default:
				break;
		}
	} catch (int e) {
		client.printf("INVALID READ: %i", e);
	}

	if (GET_BYTE(code, 3) != 'I')
		return;

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
}

void loop() {
	WiFiClient client = server.accept();

	if (server.isAvailable(client))
		server.handleConnection(client, &checkHttp, &sendHttp);
	client.stop();
}
