#include "TSensor.hpp"

#define INVALIDATE_AFTER_SEC 45

#define POWER_ON_RST_VALUE (85.0F)
#define INVALID_READ       (-127.0F)

#define FLOAT_EQUALS(X, Y) (abs(X - Y) < std::numeric_limits<float>::epsilon())

#define VTASK_DELAY(MS) (vTaskDelay(((TickType_t) (MS)) / portTICK_RATE_MS))

struct Reading {
	float temperature;
	unsigned long timestamp;
};

// private functions

float TSensor::readCelsius(uint8_t sensorIdx) {
	float value;

	for (size_t i = 0; i < 2; i++) {
		value = sensors.getTempCByIndex(sensorIdx);

		if (FLOAT_EQUALS(value, INVALID_READ)) {
			Serial.printf("[%i] Sensor %i: INVALID_READ (%.2f C). Reading again...\n", i, sensorIdx, value);
			sensors.requestTemperatures();
			VTASK_DELAY(800);
			continue;
		}
		if (!FLOAT_EQUALS(value, POWER_ON_RST_VALUE))
			break;

		Serial.printf("[%i] Sensor %i: POWER_ON_RST_VALUE (%.2f C). Reading again...\n", i, sensorIdx, value);
		VTASK_DELAY(100);
		sensors.requestTemperatures();
		VTASK_DELAY(1200);
	}

	if (FLOAT_EQUALS(value, INVALID_READ))
		throw -127;

	Serial.printf("Sensor %i - Value: %.2f C\n", sensorIdx, value);

	return value;
}

// public constructors

TSensor::TSensor(uint8_t busPin) : oneWire(busPin), sensors(&oneWire) {
	// sensors.setResolution(12);
	// sensors.setWaitForConversion(true);
	sensors.begin();
}

// public functions

float TSensor::readFahrenheit(uint8_t sensorIdx) {
	return sensors.toFahrenheit(this->readCelsius(sensorIdx));
}
