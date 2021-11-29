#pragma once
#ifndef TSENSOR_HPP
#define TSENSOR_HPP

#define SENSOR_BUS_PIN 25
#define MAX_SENSORS_ON_BUS 5

#include <DallasTemperature.h>
#include <OneWire.h>

class TSensor {
   private:
	// private variables
	OneWire oneWire;
	DallasTemperature sensors;
	float values[MAX_SENSORS_ON_BUS];

	// private functions

	float readCelsius(uint8_t sensorIdx);

   public:
	// public constructors
	TSensor();

	// public functions

	/**
    * @brief Request a sensor value.
    * 
    * @param sensorIdx the index of the sensor.
    * 
    * @return float - the reading in degrees Celsius.
	* @throws int (-1) - if the index is out of bounce.
    * @throws int (-127) - if the sensor value could not be read.
    */
	float getCelsius(uint8_t sensorIdx = 0);

	/**
    * @brief Request a sensor value.
    * 
    * @param sensorIdx the index of the sensor.
    * 
    * @return float - the reading in degrees Fahrenheit.
	* @throws int (-1) - if the index is out of bounce.
    * @throws int (-127) - if the sensor value could not be read.
    */
	float getFahrenheit(uint8_t sensorIdx = 0);

	/**
    * @brief Request an update of all sensor values.
    */
	void updateAll();
};

#endif	// TSENSOR_HPP
