#include <FreeRTOS.h>

#include "Actors.hpp"
#include "SimpleServer.hpp"
#include "StringMacros.hpp"
#include "TSensor.hpp"
#include "WiFiHandler.hpp"

#define BUZZER_PIN      14
#define TEMP_SENSOR_PIN 25

#define CUSTOM_HOSTNAME "TempMonitor-ESP32-115-0"

#define KiB(X) (1024 * X)

#define GET_BYTE(VARIABLE, POSITION) ((VARIABLE >> (8 * POSITION)) & 0xFF)
#define SET_BYTE(VARIABLE, DESTINATION_TYPE, POSITION, VALUE)               \
	VARIABLE &= ~(static_cast<DESTINATION_TYPE>(0xFF) << (8 * (POSITION))); \
	VARIABLE |= (static_cast<DESTINATION_TYPE>(VALUE) << (8 * (POSITION)));

namespace { // "static"
	Actors actors;

	SimpleServer server;
	TSensor tsensor(TEMP_SENSOR_PIN);
	WiFiHandler wifiHandler(CUSTOM_HOSTNAME);

	TaskHandle_t cpu0_handle;
} // namespace

static int32_t checkHttp(const char *const path) {
	static_assert(CHAR_BIT <= 8, "The return value works with 8-bit characters only!");
	static constexpr uint8_t u8max = std::numeric_limits<uint8_t>::max();

	char *fullRequest;
	char pathDup[strlen(path) + 1];
	{
		int endPos = snprintf(pathDup, strlen(path) + 1, "%s", path);

		if (pathDup[endPos - 1] == '/')
			pathDup[endPos - 1] = '\0';

		fullRequest = pathDup;

		if (fullRequest[0] == '/')
			fullRequest++;
	}
	/////////////////////////////////////////////////////////////////////////////

	// Request structure:
	//
	// "SENSOR/TEMPERATURE/<n>/VALUE/<unit>"

	if (STRING_STARTS_WITH(fullRequest, "SENSOR/TEMPERATURE/")) {
		STRING_PRUNE_SUBSTRING(fullRequest, "SENSOR/TEMPERATURE/");

		char *requestType;
		long sensorIdx = strtol(fullRequest, &requestType, 10);

		if (requestType == fullRequest || sensorIdx < 0 || sensorIdx > u8max)
			return 0;

		fullRequest = requestType;

		if (STRING_STARTS_WITH(fullRequest, "/VALUE/")) {
			STRING_PRUNE_SUBSTRING(fullRequest, "/VALUE/");

			if (strlen(fullRequest) > 1)
				return 0;

			int32_t returnCode = 0;
			switch (*fullRequest) {
				case 'C':                                  // Celsius
					SET_BYTE(returnCode, int32_t, 3, 'V'); // Value
					SET_BYTE(returnCode, int32_t, 1, 'C'); // Celsius
					SET_BYTE(returnCode, int32_t, 0, sensorIdx);
					return returnCode;
				case 'F':                                  // Fahrenheit
					SET_BYTE(returnCode, int32_t, 3, 'V'); // Value
					SET_BYTE(returnCode, int32_t, 1, 'F'); // Fahrenheit
					SET_BYTE(returnCode, int32_t, 0, sensorIdx);
					return returnCode;
			}
		}

		return 0;
	}

	// Request structure:
	//
	// "ACTOR/BUZZER/<n>/OFF"
	// "ACTOR/BUZZER/<n>/ON[/<value>]" (unit: milliseconds)

	if (STRING_STARTS_WITH(fullRequest, "ACTOR/BUZZER/")) {
		STRING_PRUNE_SUBSTRING(fullRequest, "ACTOR/BUZZER/");

		char *newState;
		long buzzerIdx = strtol(fullRequest, &newState, 10);

		if (newState == fullRequest || buzzerIdx < 0 || buzzerIdx > u8max)
			return 0;

		fullRequest = newState;

		if (STRING_EQUALS(fullRequest, "/OFF")) {
			int32_t returnCode = 0;
			SET_BYTE(returnCode, int32_t, 3, 'B'); // Buzzer
			SET_BYTE(returnCode, int32_t, 1, 0);   // Off
			SET_BYTE(returnCode, int32_t, 0, buzzerIdx);

			actors.set(buzzerIdx, ACTOR_INACTIVE);

			return returnCode;
		}

		if (STRING_STARTS_WITH(fullRequest, "/ON")) {
			STRING_PRUNE_SUBSTRING(fullRequest, "/ON");

			if (*fullRequest == '\0') {
				int32_t returnCode = 0;
				SET_BYTE(returnCode, int32_t, 3, 'B'); // Buzzer
				SET_BYTE(returnCode, int32_t, 1, 1);   // On
				SET_BYTE(returnCode, int32_t, 0, buzzerIdx);

				actors.set(buzzerIdx, ACTOR_ACTIVE);

				return returnCode;
			}

			char *endPtr;
			long duration = strtol(++fullRequest, &endPtr, 10);

			if (endPtr == fullRequest || duration <= 0 || *endPtr != '\0')
				return 0;

			int32_t returnCode = 0;
			SET_BYTE(returnCode, int32_t, 3, 'B'); // Buzzer
			SET_BYTE(returnCode, int32_t, 1, 'S'); // Signal
			SET_BYTE(returnCode, int32_t, 0, buzzerIdx);

			actors.signal(buzzerIdx, ACTOR_ACTIVE, duration);

			return returnCode;
		}

		return 0;
	}

	if (STRING_EQUALS(fullRequest, "index.html")) {
		int32_t returnCode = 0;
		SET_BYTE(returnCode, int32_t, 3, 'I'); // Index
		return returnCode;
	}

	return 0;
}

static void sendHttp(WiFiClient &client, const char *path, int32_t code) {
	try {
		switch (GET_BYTE(code, 3)) {
			case 'B': // Buzzer
				const char *state;
				switch (GET_BYTE(code, 1)) {
					case 0:
						state = "OFF";
						break;
					case 1:
						state = "ON";
						break;
					default:
						state = "BEEP/SIGNAL";
				}
				client.printf("OK. Buzzer %i: %s", GET_BYTE(code, 0), state);
				return;

			case 'V': // Value
				switch (GET_BYTE(code, 1)) {
					case 'C': // Celsius
						client.printf("%.2f", tsensor.readCelsius(GET_BYTE(code, 0)));
						break;
					case 'F': // Fahrenheit
						client.printf("%.2f", tsensor.readFahrenheit(GET_BYTE(code, 0)));
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

	client.printf("<html><head>"
				  "<title>%s</title>"
				  "</head><body>"
				  "<i>System Time (millis): %lu</i><br><br>",
				  CUSTOM_HOSTNAME, millis());

	for (size_t i = 0; true; i++) {
		try {
			client.printf("<b>Current Temperature (Sensor %i):</b>   %.2f C<br>", i, tsensor.readCelsius(i));
		} catch (int e) {
			if (i == 0)
				client.print("<b>NO SENSORS AVAILABLE</b>");
			break;
		}
	}
	client.print("</body></html>");
}

// Running on Core 0 (Protocol CPU) along with WiFi, etc.
static void setup0(void *) {
	while (1) {
		wifiHandler.checkActiveConnection();
		delay(10000);
	}
}

void setup() {
	Serial.begin(115200);
	actors.add(BUZZER_PIN);
	wifiHandler.begin();
	server.begin();

	xTaskCreatePinnedToCore(&setup0, "setup0", KiB(32), NULL, 1, &cpu0_handle, 0);
}

// Running on Core 1 (Application CPU)
void loop() {
	WiFiClient client = server.accept();

	if (server.isAvailable(client))
		server.handleConnection(client, &checkHttp, &sendHttp);
	client.stop();
}
