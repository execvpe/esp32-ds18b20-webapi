#include <FreeRTOS.h>

#include "SimpleServer.hpp"
#include "StringMacros.hpp"
#include "TSensor.hpp"
#include "WiFiHandler.hpp"

#define CUSTOM_HOSTNAME "VentControl-ESP32-116-0"
#define UPDATE_VALUES_EVERY_SEC 15

#define VTASK_DELAY(MS) (vTaskDelay(((TickType_t) (MS)) / portTICK_RATE_MS))

#define KiB(X) (1024 * X)
#define GET_BYTE(VARIABLE, POSITION) ((VARIABLE >> (8 * POSITION)) & 0xFF)
#define SET_BYTE(VARIABLE, DESTINATION_TYPE, POSITION, VALUE)               \
	VARIABLE &= ~(static_cast<DESTINATION_TYPE>(0xFF) << (8 * (POSITION))); \
	VARIABLE |= (static_cast<DESTINATION_TYPE>(VALUE) << (8 * (POSITION)));

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
		VTASK_DELAY(UPDATE_VALUES_EVERY_SEC * 1000);
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

static int32_t checkHttp(const char *fullRequest) {
	static_assert(CHAR_BIT <= 8, "The return value works with 8-bit characters only!");
	static constexpr uint8_t u8max = std::numeric_limits<uint8_t>::max();

	if (fullRequest[0] == '/')
		fullRequest++;

	/////////////////////////////////////////////////////////////////////////////

	// Request structure:
	//
	// "SENSOR/TEMPERATURE/<n>/VALUE/<unit>"
	// "SENSOR/TEMPERATURE/<n>/ELAPSED_TIME/<unit>" (for now, unit must be milliseconds: "MS")
	// "SENSOR/TEMPERATURE/<n>/_ALL/"

	if (STRING_STARTS_WITH(fullRequest, "SENSOR/TEMPERATURE/")) {
		STRING_PRUNE_SUBSTRING(fullRequest, "SENSOR/TEMPERATURE/");

		char *requestType;
		long sensorIdx = strtol(fullRequest, &requestType, 10);

		if (requestType == fullRequest || sensorIdx < 0 || sensorIdx > u8max)
			return 0;

		if (STRING_STARTS_WITH(requestType, "/VALUE/")) {
			STRING_PRUNE_SUBSTRING(requestType, "/VALUE/");

			if (strlen(requestType) > 1)
				return 0;

			int32_t returnCode = 0;
			switch (*requestType) {
				case 'C':									// Celsius
					SET_BYTE(returnCode, int32_t, 3, 'V');	// Value
					SET_BYTE(returnCode, int32_t, 1, 'C');	// Celsius
					SET_BYTE(returnCode, int32_t, 0, sensorIdx);
					return returnCode;
				case 'F':									// Fahrenheit
					SET_BYTE(returnCode, int32_t, 3, 'V');	// Value
					SET_BYTE(returnCode, int32_t, 1, 'F');	// Fahrenheit
					SET_BYTE(returnCode, int32_t, 0, sensorIdx);
					return returnCode;
			}

			return 0;
		}

		if (STRING_EQUALS(requestType, "/ELAPSED_TIME/MS")) {
			int32_t returnCode = 0;
			SET_BYTE(returnCode, int32_t, 3, 'E');	// Elapsed time
			SET_BYTE(returnCode, int32_t, 0, sensorIdx);

			return returnCode;
		}

		if (STRING_EQUALS(requestType, "/_ALL")) {
			int32_t returnCode = 0;
			SET_BYTE(returnCode, int32_t, 3, 'A');	// All
			SET_BYTE(returnCode, int32_t, 0, sensorIdx);

			return returnCode;
		}

		return 0;
	}

	if (STRING_EQUALS(fullRequest, "index.html")) {
		int32_t returnCode = 0;
		SET_BYTE(returnCode, int32_t, 3, 'I');	// Index
		return returnCode;
	}

	return 0;
}

static void sendHttp(WiFiClient &client, const char *path, int32_t code) {
	try {
		switch (GET_BYTE(code, 3)) {
			case 'A':  // All
				client.printf("%.2f C\n", tsensor.getCelsius(GET_BYTE(code, 0)));
				client.printf("%.2f F\n", tsensor.getFahrenheit(GET_BYTE(code, 0)));
				client.printf("%lu MS", tsensor.elapsedSince(GET_BYTE(code, 0)));
				return;

			case 'E':  // Elapsed time
				client.printf("%lu", tsensor.elapsedSince(GET_BYTE(code, 0)));
				return;

			case 'V':  // Value
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
