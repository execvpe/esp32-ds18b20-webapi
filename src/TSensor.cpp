#include "TSensor.hpp"

TSensor::TSensor() {
	oneWire = OneWire(SENSOR_BUS_PIN);
	sensors = DallasTemperature(&oneWire);
	sensors.begin();
}

float TSensor::getCelsius(uint8_t sensorIdx) {
	sensors.requestTemperaturesByIndex(sensorIdx);
	return sensors.getTempCByIndex(sensorIdx);
}
