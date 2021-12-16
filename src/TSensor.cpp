#include "TSensor.hpp"

#define POWER_ON_RST_VALUE (85.0F)
#define INVALID_READ (-127.0F)

#define FLOAT_EQUALS(X, Y) (abs(X - Y) < std::numeric_limits<float>::epsilon())

static void setHead(void *compare, void *value);

struct Reading {
	float temperature;
	unsigned long timestamp;
	size_t index;
	Reading *next;

	Reading() {
		Serial.printf("Constructor %f\n", temperature);

		setHead(nullptr, this);
	}

	~Reading() {
		Serial.printf("Destructor %f\n", temperature);

		setHead(this, nullptr);

		// "The value of the operand of delete may be a null pointer value."
		// Delete recursively
		delete next;
	}
};

static Reading *head = nullptr;

static void setHead(void *compare, void *value) {
	if (head == compare)
		head = static_cast<Reading *>(value);
}

// public constructors

TSensor::TSensor() : oneWire(SENSOR_BUS_PIN), sensors(&oneWire) {
	//sensors.setResolution(12);
	//sensors.setWaitForConversion(true);
	sensors.begin();
}

float TSensor::readCelsius(uint8_t sensorIdx) {
	float value;

	for (size_t i = 0; i < 2; i++) {
		value = sensors.getTempCByIndex(sensorIdx);
		Serial.printf("Sensor %i - Value: %.2f C\n", sensorIdx, value);

		if (FLOAT_EQUALS(value, INVALID_READ))
			throw -127;
		if (!FLOAT_EQUALS(value, POWER_ON_RST_VALUE))
			break;
		Serial.printf("Sensor %i has RST_VALUE (%.2f C). Reading again...\n", sensorIdx, value);
		sensors.requestTemperaturesByIndex(sensorIdx);
	}

	return value;
}

// public functions

float TSensor::getCelsius(uint8_t sensorIdx) {
	Reading *current = head;
	for (size_t idx = 0; current != nullptr; idx++, current = current->next) {
		if (idx == sensorIdx) {
			return current->temperature;
		}
	}
	throw -127;
}

float TSensor::getFahrenheit(uint8_t sensorIdx) {
	return sensors.toFahrenheit(this->getCelsius(sensorIdx));
}

void TSensor::updateAll() {
	sensors.requestTemperatures();

	Reading *drag = nullptr;
	Reading *current = head;
	for (size_t idx = 0;; idx++) {
		float temp;
		try {
			temp = this->readCelsius(idx);
		} catch (int e) {
			if (current && (abs(millis() - current->timestamp) > (INVALIDATE_AFTER_SEC * 1000))) {
				Serial.println("No longer valid!");
				delete current;
				if (drag != nullptr) {
					drag->next = nullptr;
				}
			}
			return;
		}

		if (current == nullptr) {
			current = new Reading();
			current->next = nullptr;
		}

		if (drag != nullptr) {
			drag->next = current;
		}

		current->temperature = temp;
		current->timestamp = millis();
		current->index = idx;

		drag = current;
		current = current->next;
	}
}
