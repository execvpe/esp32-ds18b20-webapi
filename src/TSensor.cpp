#include "TSensor.hpp"

#define POWER_ON_RST_VALUE (85.0F)
#define INVALID_READ (-127.0F)

static void memset_flt(float *mem, float value, size_t size) {
	for (size_t i = 0; i < size; i++) {
		mem[i] = value;
	}
}

// public constructors

TSensor::TSensor() : oneWire(SENSOR_BUS_PIN), sensors(&oneWire) {
	//sensors.setResolution(12);
	//sensors.setWaitForConversion(true);
	sensors.begin();
}

float TSensor::readCelsius(uint8_t sensorIdx) {
	static float epsilon = std::numeric_limits<float>::epsilon();
	float value;

	for (size_t i = 0; i < 2; i++) {
		value = sensors.getTempCByIndex(sensorIdx);
		Serial.printf("Sensor %i - Value: %.2f\r\n", sensorIdx, value);

		if (abs(value - INVALID_READ) <= epsilon)
			throw -127;
		if (abs(value - POWER_ON_RST_VALUE) > epsilon)
			break;
		Serial.printf("Sensor %i has RST_VALUE (%.2f C). Reading again...\r\n", sensorIdx, value);
		sensors.requestTemperaturesByIndex(sensorIdx);
	}

	return value;
}

// public functions

float TSensor::getCelsius(uint8_t sensorIdx) {
	if (sensorIdx >= MAX_SENSORS_ON_BUS)
		throw -1;

	float value = values[sensorIdx];

	if (isnan(value))
		throw -127;

	return value;
}

float TSensor::getFahrenheit(uint8_t sensorIdx) {
	return sensors.toFahrenheit(this->getCelsius(sensorIdx));
}

void TSensor::updateAll() {
	const float nan = std::numeric_limits<float>::quiet_NaN();
	sensors.requestTemperatures();
	memset_flt(values, nan, MAX_SENSORS_ON_BUS);

	for (size_t i = 0; i < MAX_SENSORS_ON_BUS; i++) {
		try {
			values[i] = this->readCelsius(i);
		} catch (int e) {
			break;
		}
	}
}