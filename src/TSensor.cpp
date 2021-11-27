#include "TSensor.hpp"

// public constructors

TSensor::TSensor() {
	oneWire = OneWire(SENSOR_BUS_PIN);
	sensors = DallasTemperature(&oneWire);
	sensors.begin();
}

// public functions

float TSensor::getCelsius(uint8_t sensorIdx) {
	sensors.requestTemperaturesByIndex(sensorIdx);
	return sensors.getTempCByIndex(sensorIdx);
}
