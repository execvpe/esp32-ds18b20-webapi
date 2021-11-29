#pragma once
#ifndef TSENSOR_HPP
#define TSENSOR_HPP

#define SENSOR_BUS_PIN 26

#include <DallasTemperature.h>
#include <OneWire.h>

class TSensor {
   private:
	// private variables
	OneWire oneWire;
	DallasTemperature sensors;

   public:
	// public constructors
	TSensor();

	// public functions
	float getCelsius(uint8_t sensorIdx = 0);
	float getFahrenheit(uint8_t sensorIdx = 0);
};

#endif	// TSENSOR_HPP
