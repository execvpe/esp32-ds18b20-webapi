#pragma once
#ifndef TSENSOR_HPP
#define TSENSOR_HPP

#include <DallasTemperature.h>
#include <OneWire.h>

class TSensor {
	private:
	// private variables
	OneWire oneWire;
	DallasTemperature sensors;

	public:
	// public constructors
	TSensor(uint8_t busPin);

	// public functions

	/**
	 * @brief Request a sensor value.
	 *
	 * @param sensorIdx the index of the sensor.
	 *
	 * @return float - the reading in degrees Celsius.
	 * @throws int (-127) - if the sensor value could not be read.
	 */
	float readCelsius(uint8_t sensorIdx = 0);

	/**
	 * @brief Request a sensor value.
	 *
	 * @param sensorIdx the index of the sensor.
	 *
	 * @return float - the reading in degrees Fahrenheit.
	 * @throws int (-127) - if the sensor value could not be read.
	 */
	float readFahrenheit(uint8_t sensorIdx = 0);
};

#endif // TSENSOR_HPP
