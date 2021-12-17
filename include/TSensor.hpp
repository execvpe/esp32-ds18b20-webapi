#pragma once
#ifndef TSENSOR_HPP
#define TSENSOR_HPP

#define SENSOR_BUS_PIN 25
#define INVALIDATE_AFTER_SEC 45

#include <DallasTemperature.h>
#include <OneWire.h>

class TSensor {
   private:
	// private variables
	OneWire oneWire;
	DallasTemperature sensors;

	// private functions

	float readCelsius(uint8_t sensorIdx);

   public:
	// public constructors
	TSensor();

	// public functions

	/**
    * @brief Request how much time has passed since the last valid reading.
    * 
    * @param sensorIdx the index of the sensor.
    * 
    * @return unsigned long - the amount in milliseconds.
    * @throws int (-127) - if the sensor value could not be read.
    */
	unsigned long elapsedSince(uint8_t sensorIdx = 0);

	/**
    * @brief Request a sensor value.
    * 
    * @param sensorIdx the index of the sensor.
    * 
    * @return float - the reading in degrees Celsius.
    * @throws int (-127) - if the sensor value could not be read.
    */
	float getCelsius(uint8_t sensorIdx = 0);

	/**
    * @brief Request a sensor value.
    * 
    * @param sensorIdx the index of the sensor.
    * 
    * @return float - the reading in degrees Fahrenheit.
    * @throws int (-127) - if the sensor value could not be read.
    */
	float getFahrenheit(uint8_t sensorIdx = 0);

	/**
    * @brief Request an update of all sensor values.
    */
	void updateAll();
};

#endif	// TSENSOR_HPP
