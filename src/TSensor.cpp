#include "TSensor.hpp"

#define POWER_ON_RST_VALUE (85.0F)

// public constructors

TSensor::TSensor() {
	oneWire = OneWire(SENSOR_BUS_PIN);
	sensors = DallasTemperature(&oneWire);
	sensors.begin();
}

// public functions

float TSensor::getCelsius(uint8_t sensorIdx) {
	static float epsilon = std::numeric_limits<float>::epsilon();
	float value;

	for (size_t i = 0; i < 2; i++) {
		sensors.requestTemperaturesByIndex(sensorIdx);
		value = sensors.getTempCByIndex(sensorIdx);
		if (abs(value - POWER_ON_RST_VALUE) > epsilon)
			break;
	}

	return value;
}
